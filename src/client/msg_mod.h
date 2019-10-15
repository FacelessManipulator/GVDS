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

namespace hvs {
class ClientRpc : public ClientModule {
 private:
  virtual void start() override;
  virtual void stop() override;

 private:
  UDTClient udt_client;
  std::mutex rpc_mutex;
  std::atomic_int multi_channel;
  std::unordered_map<std::string, std::shared_ptr<RpcClient>> rpc_clients;
  std::unordered_map<std::string, std::shared_ptr<ClientSession>> udt_clients;
  std::unordered_map<std::string, std::shared_ptr<Pistache::Http::Client>> rest_clients;
  std::unordered_map<std::string, Pistache::Http::CookieJar> rest_cookies;

public:
  std::shared_ptr<RpcClient> rpc_channel(std::shared_ptr<IOProxyNode> node, bool reconnect = false, int channel_id = 0);
  std::shared_ptr<ClientSession> udt_channel(std::shared_ptr<IOProxyNode> node, bool reconnect = false);
  std::shared_ptr<Pistache::Http::Client> rest_channel(std::string endpoint);
  // auto handle the data write operation response

 public:
  ClientRpc(const char* name, Client* cli) : ClientModule(name, cli) {
    isThread = true;
  }

  template <typename... Args>
  std::shared_ptr<RPCLIB_MSGPACK::object_handle> call(
      std::shared_ptr<IOProxyNode> node, std::string const& func_name,
      Args... args);

  template <typename... Args>
  bool async_call(
      std::shared_ptr<IOProxyNode> node, std::string const& func_name, std::function<void()> f,
      Args... args);

  int write_data(std::shared_ptr<IOProxyNode> node, ioproxy_rpc_buffer& buf);
  int write_data_async(std::shared_ptr<IOProxyNode> node, ioproxy_rpc_buffer& buf);
  std::shared_ptr<ioproxy_rpc_buffer> read_data(
      std::shared_ptr<IOProxyNode> node, ioproxy_rpc_buffer& buf);
  // WARNNING: the return result may be empty if request failed
  std::string post_request(const std::string& endpoint, const std::string& url,
                           const std::string& data = "");
  std::string get_request(const std::string& endpoint, const std::string& url);
  std::string delete_request(const std::string& endpoint, const std::string& url);

  friend class Client;
};

template <typename... Args>
std::shared_ptr<RPCLIB_MSGPACK::object_handle> ClientRpc::call(
    std::shared_ptr<IOProxyNode> node, std::string const& func_name,
    Args... args) {
  // TODO: We assume RpcClient can concurently call
  auto rpcc = rpc_channel(node, false, multi_channel++%10);
  auto res = rpcc->call(func_name, args...);
  if (!res) {
    // timeout? try reconnect
    rpcc = rpc_channel(node, true);
    res = rpcc->call(func_name, args...);
  }
  if(!res) {
    return nullptr;
  } else {
    return std::make_shared<RPCLIB_MSGPACK::object_handle>(std::move(*res));
  }
}

template <typename... Args>
bool ClientRpc::async_call(
            std::shared_ptr<IOProxyNode> node, std::string const& func_name, std::function<void()> f,
            Args... args) {
      // TODO: We assume RpcClient can concurently call
      auto rpcc = rpc_channel(node);
      // the callback function could be used to trigger some event
      bool res = rpcc->async_call(func_name, f, args...);
      if (!res) {
        // timeout? try reconnect
        rpcc = rpc_channel(node, true);
        res = rpcc->async_call(func_name, f, args...);
      }
      return res;
    }
}  // namespace hvs