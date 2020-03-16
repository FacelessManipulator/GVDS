/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-18 15:33:31
 * @Last Modified by: Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:38:24
 */

#pragma once
#include <error.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <vector>

// grpc related
#include <grpcpp/grpcpp.h>
#include "op.grpc.pb.h"
#include "op.pb.h"

// gvds related
#include "common/Thread.h"
#include "context.h"

namespace gvds {
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using gvds::Operator;
using gvds::OpReply;
using gvds::OpRequest;
// Logic and data behind the server's behavior.
class OpServerImpl final : public Thread {
 public:
  OpServerImpl(unsigned int port) : m_stop(true), port(port) {}
  ~OpServerImpl() { stop(); }

  void start();
  void stop();
  // There is no shutdown handling in this code.
  void* entry() override;

  Operator::AsyncService* get_service() { return &service_; }
  ServerCompletionQueue* get_cq() { return cq_.get(); }
  unsigned int get_port() { return port; }

 private:
  std::unique_ptr<ServerCompletionQueue> cq_;
  Operator::AsyncService service_;
  std::unique_ptr<Server> server_;
  bool m_stop;
  unsigned int port;
};
}  // namespace gvds
