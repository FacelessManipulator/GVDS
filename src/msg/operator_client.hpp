

#pragma once
// grpc related
#include <grpcpp/grpcpp.h>
#include <future>
#include "common/Thread.h"
#include "gvds_context.h"
#include "op.grpc.pb.h"

namespace gvds {
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
  std::future<OpReply *> SubmitAsync(const OpRequest &op,
                                     const std::function<void()> &callback) {
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
    std::promise<OpReply *> prom;
  };

 private:
  std::unique_ptr<Operator::Stub> stub_;
  CompletionQueue cq_;
  bool m_stop;
};
}  // namespace gvds