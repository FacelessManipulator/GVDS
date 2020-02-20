/*
 * @Author: Hanjie,Zhou 
 * @Date: 2020-02-20 00:37:06 
 * @Last Modified by:   Hanjie,Zhou 
 * @Last Modified time: 2020-02-20 00:37:06 
 */
#pragma once

#include <pistache/client.h>
#include <cerrno>
#include <mutex>
#include <unordered_map>
#include "client.h"
#include "common/Thread.h"
#include "hvs_struct.h"
#include "msg/node.h"
#include "msg/udt_client.h"

// grpc related
#include <grpcpp/grpcpp.h>
#include "op.grpc.pb.h"

namespace hvs {
using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using gvds::Operator;
using gvds::OpReply;
using gvds::OpRequest;
using gvds::OpType;
class OperatorClient : Thread {
 public:
  OperatorClient(std::shared_ptr<Channel> channel)
      : stub_(Operator::NewStub(channel)), m_stop(true) {
    // auto start thread
    start();
  }
  ~OperatorClient() { stop(); }

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  Status Submit(const OpRequest &op, OpReply &reply) {
    ClientContext context;
    Status status = stub_->Submit(&context, op, &reply);
    if (!status.ok()) {
      dout(10) << status.error_code() << ": " << status.error_message()
               << dendl;
    }
    return status;
  }
  std::future<OpReply*> SubmitAsync(const OpRequest &op, const std::function<void()> &callback) {
    AsyncClientCall *call = new AsyncClientCall;
    call->response_reader = stub_->PrepareAsyncSubmit(&call->context, op, &cq_);
    call->callback = callback;
    // StartCall initiates the RPC call
    call->response_reader->StartCall();
    call->response_reader->Finish(&call->reply, &call->status, (void *)call);
    return std::move(call->prom.get_future());
  }
  void start() {
    m_stop = false;
    create("grpc-client-t");
  }
  void stop() {
    // without two-phase lock or critical area may cause problem
    // but it's rare so I ignore it.
    if (!m_stop) {
      m_stop = true;
      cq_.Shutdown();
      join();
    }
  }
  void *entry() {
    void *got_tag;
    bool ok = false;
    while (!m_stop && cq_.Next(&got_tag, &ok)) {
      AsyncClientCall *call = static_cast<AsyncClientCall *>(got_tag);
      // TODO: currently I trigger callback whether the connection success or
      // not
      if (call->status.ok()) {
        call->prom.set_value(&call->reply);
        call->callback();
      } else {
        dout(-1) << "GRPC called failed" << dendl;
        call->prom.set_value(nullptr);
        call->callback();
      }
      // Once we're complete, delete the call data
      delete call;
    }
  }
  struct AsyncClientCall {
    OpReply reply;
    // Context for the client. It could be used to deliver extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<OpReply>> response_reader;
    std::function<void()> callback;
    std::promise<OpReply*> prom;
  };

 private:
  std::unique_ptr<Operator::Stub> stub_;
  CompletionQueue cq_;
  bool m_stop;
};

class ClientRpc : public ClientModule {
 private:
  virtual void start() override;
  virtual void stop() override;

 private:
  UDTClient udt_client;
  std::mutex rpc_mutex;
  std::atomic_int multi_channel;
  std::unordered_map<std::string, std::shared_ptr<RpcClient>> rpc_clients;
  std::unordered_map<std::string, std::shared_ptr<OperatorClient>> operators;
  std::unordered_map<std::string, std::shared_ptr<ClientSession>> udt_clients;
  std::unordered_map<std::string, std::shared_ptr<Pistache::Http::Client>>
      rest_clients;
  std::unordered_map<std::string, Pistache::Http::CookieJar> rest_cookies;
  std::vector<int> channelLoads;
  pthread_mutex_t cLoadMutex;

 public:
  std::shared_ptr<RpcClient> rpc_channel(std::shared_ptr<IOProxyNode> node,
                                         bool reconnect = false,
                                         int channel_id = 0);
  std::shared_ptr<OperatorClient> get_operator(
      std::shared_ptr<IOProxyNode> node, int channel_id = 0);
  std::shared_ptr<ClientSession> udt_channel(std::shared_ptr<IOProxyNode> node,
                                             bool reconnect = false);
  std::shared_ptr<Pistache::Http::Client> rest_channel(std::string endpoint);
  // auto handle the data write operation response

 public:
  ClientRpc(const char *name, Client *cli) : ClientModule(name, cli) {
    auto _config = HvsContext::get_context()->_config;
    multi_channel = _config->get<int>("client.multi_channel").value_or(1);
    channelLoads.resize(multi_channel, 0);
    isThread = true;
  }

  int getMinLoadCNumber() {
    int minLoad = channelLoads[0], channelNumber = 0;
    pthread_mutex_lock(&cLoadMutex);
    for (int i = 0; i < multi_channel; i++) {
      int load = channelLoads[i];
      if (load < minLoad) {
        channelNumber = i;
        minLoad = load;
      }
    }

    if (minLoad > 1000) {
      for (int i = 0; i < multi_channel; i++) {
        channelLoads[i] = channelLoads[i] - minLoad;
      }
    }
    channelLoads[channelNumber]++;
    pthread_mutex_unlock(&cLoadMutex);
    return channelNumber;
    return 0;
  }

  template <typename... Args>
  std::shared_ptr<RPCLIB_MSGPACK::object_handle> call(
      std::shared_ptr<IOProxyNode> node, std::string const &func_name,
      Args... args);

  template <typename... Args>
  bool async_call(std::shared_ptr<IOProxyNode> node,
                  std::string const &func_name, std::function<void()> f,
                  Args... args);

  int write_data(std::shared_ptr<IOProxyNode> node, ioproxy_rpc_buffer &buf);
  int write_data_async(std::shared_ptr<IOProxyNode> node,
                       ioproxy_rpc_buffer &buf);
  std::shared_ptr<ioproxy_rpc_buffer> read_data(
      std::shared_ptr<IOProxyNode> node, ioproxy_rpc_buffer &buf);
  // WARNNING: the return result may be empty if request failed
  std::string post_request(const std::string &endpoint, const std::string &url,
                           const std::string &data = "");
  std::string get_request(const std::string &endpoint, const std::string &url);
  std::string delete_request(const std::string &endpoint,
                             const std::string &url);

  friend class Client;
};

template <typename... Args>
std::shared_ptr<RPCLIB_MSGPACK::object_handle> ClientRpc::call(
    std::shared_ptr<IOProxyNode> node, std::string const &func_name,
    Args... args) {
  // TODO: We assume RpcClient can concurently call
  auto rpcc = rpc_channel(node, false, getMinLoadCNumber());
  auto res = rpcc->call(func_name, args...);
  if (!res) {
    // timeout? try reconnect
    rpcc = rpc_channel(node, true, getMinLoadCNumber());
    res = rpcc->call(func_name, args...);
  }
  if (!res) {
    return nullptr;
  } else {
    return std::make_shared<RPCLIB_MSGPACK::object_handle>(std::move(*res));
  }
}

template <typename... Args>
bool ClientRpc::async_call(std::shared_ptr<IOProxyNode> node,
                           std::string const &func_name,
                           std::function<void()> f, Args... args) {
  // TODO: We assume RpcClient can concurently call
  auto rpcc = rpc_channel(node, false, getMinLoadCNumber());
  // the callback function could be used to trigger some event
  bool res = rpcc->async_call(func_name, f, args...);
  if (!res) {
    // timeout? try reconnect
    rpcc = rpc_channel(node, true, getMinLoadCNumber());
    res = rpcc->async_call(func_name, f, args...);
  }
  return res;
}
}  // namespace hvs