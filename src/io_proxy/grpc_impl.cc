/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-18 18:45:31
 * @Last Modified by: Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:38:27
 */

#include "io_proxy/grpc_impl.h"
#include "io_proxy/io_worker.h"

using namespace gvds;
using namespace std;

void OpServerImpl::start() {
  std::string server_address("0.0.0.0:");
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address + to_string(port),
                           grpc::InsecureServerCredentials());
  // Register "service_" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *asynchronous* service.
  builder.RegisterService(&service_);
  // Get hold of the completion queue used for the asynchronous communication
  // with the gRPC runtime.
  cq_ = builder.AddCompletionQueue();
  // Finally assemble the server.
  server_ = builder.BuildAndStart();
  dout(-1) << "RPC Server listening on " << server_address + to_string(port)
           << dendl;
  m_stop = false;
  create("IOProxy_RPC");
}

void OpServerImpl::stop() {
  if (m_stop == true) return;
  m_stop = true;
  server_->Shutdown();
  // Always shutdown the completion queue after the server.
  cq_->Shutdown();
}

void* OpServerImpl::entry() {
  // Proceed to the server's main loop.
  void* tag;  // uniquely identifies a request.
  bool ok;
  while (!m_stop) {
    // in server-side ok is always true
    cq_->Next(&tag, &ok);
    auto worker = static_cast<IOProxyWorker::CallData*>(tag)->worker;
    switch (worker->status) {
      case IOProxyWorker::Status::WAITING: {
        boost::intrusive_ptr<OpQueued> ev = new OpQueued();
        worker->my_scheduler().queue_event(worker->my_handle(), ev);
        break;
      }
      case IOProxyWorker::Status::FINISHED: {
        boost::intrusive_ptr<OpFinished> ev = new OpFinished();
        worker->my_scheduler().queue_event(worker->my_handle(), ev);
        break;
      }
      default:
        break;
    }
    // static_cast<CallData*>(tag)->Proceed();
  }
}