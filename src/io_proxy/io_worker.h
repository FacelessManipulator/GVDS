/*
 * @Author: Hanjie,Zhou 
 * @Date: 2020-02-20 00:38:41 
 * @Last Modified by:   Hanjie,Zhou 
 * @Last Modified time: 2020-02-20 00:38:41 
 */
#pragma once

#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/fifo_scheduler.hpp>

#include <boost/bind.hpp>
#include <boost/config.hpp>
#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/mpl/list.hpp>
#include <memory>  // std::allocator

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/pool/pool_alloc.hpp>

// grpc related
#include <grpcpp/grpcpp.h>
#include "op.grpc.pb.h"
#include "op.pb.h"

#include "gvds_context.h"
#include "msg/op.h"

namespace gvds {

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using gvds::OpRequest;
using gvds::OpReply;
using gvds::Operator;

namespace sc = ::boost::statechart;
namespace mpl = ::boost::mpl;
    typedef std::allocator< void > IOProxy_allocator;
    typedef sc::fifo_scheduler<> IOProxy_scheduler;
class IOProxy;
// event op queued
struct OpQueued : sc::event<OpQueued> {
  std::shared_ptr<OP> op;
  OpQueued(std::shared_ptr<OP> _op) : op(_op) {}
  OpQueued() {}
};

struct OpComplete : sc::event<OpComplete> {};
struct OpFinished: sc::event<OpFinished> {};
struct IOProxyWorker;
struct Waiting;
struct Processing;
}  // namespace gvds

namespace boost {
namespace statechart {
// The following class member specialization ensures that
// state_machine<>::initiate is not instantiated at a point where Waiting
// is not defined yet.
template <>
inline void asynchronous_state_machine<gvds::IOProxyWorker, gvds::Waiting,
                                       gvds::IOProxy_scheduler,
        gvds::IOProxy_allocator>::initiate_impl() {}
}  // namespace statechart
}  // namespace boost

namespace gvds {
struct IOProxyWorker
    : sc::asynchronous_state_machine<IOProxyWorker, Waiting, IOProxy_scheduler,
            IOProxy_allocator> {
 public:
  IOProxyWorker(my_context ctx, Operator::AsyncService* service, ServerCompletionQueue* cq) : my_base(ctx),
    _op_service(service), _cq(cq) {}

  // local variables or functions
  uint8_t worker_id;
  uint8_t scher_id;
  std::shared_ptr<OP> cur_op;

 public:
  // The means of communication with the gRPC runtime for an asynchronous
  // server.
  Operator::AsyncService* _op_service;
  // The producer-consumer queue where for asynchronous server notifications.
  ServerCompletionQueue* _cq;

  // The means to get back to the client.
  struct CallData {
      CallData(IOProxyWorker* _worker): responder(&_op_ctx), worker(_worker) {}
      // Context for the rpc, allowing to tweak aspects of it such as the use
      // of compression, authentication, as well as to send metadata back to the
      // client.
      ServerContext _op_ctx;

      // What we get from the client.
      OpRequest request;
      // What we send back to the client.
      OpReply reply;
      ServerAsyncResponseWriter<OpReply> responder;
      IOProxyWorker* worker;
  };

  std::unique_ptr<CallData> cur_call;
  enum Status {
      WAITING = 1,
      PROCESSING,
      FINISHED,
  };
  std::atomic_int status;

  // This function is defined in ioproxy.cc
  virtual void initiate_impl();
};

struct Waiting : sc::state<Waiting, IOProxyWorker> {
 public:
  typedef mpl::list<sc::custom_reaction<OpQueued>> reactions;

  Waiting(my_context ctx);
  ~Waiting();

  sc::result react(const OpQueued &);
};

struct Processing : sc::state<Processing, IOProxyWorker> {
 public:
  typedef mpl::list<sc::custom_reaction<OpComplete>> reactions;

  Processing(my_context ctx);
  ~Processing();

  sc::result react(const OpComplete &);
};

    struct Finished : sc::state<Finished, IOProxyWorker> {
    public:
        typedef mpl::list<sc::custom_reaction<OpFinished>> reactions;

        Finished(my_context ctx);
        ~Finished();

        sc::result react(const OpFinished &);
    };
}  // namespace gvds
