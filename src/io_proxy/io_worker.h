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

#include "context.h"
#include "msg/op.h"

namespace hvs {

namespace sc = ::boost::statechart;
namespace mpl = ::boost::mpl;
    typedef std::allocator< void > IOProxy_allocator;
    typedef sc::fifo_scheduler<> IOProxy_scheduler;
class IOProxy;
// event op queued
struct OpQueued : sc::event<OpQueued> {
  std::shared_ptr<OP> op;
  OpQueued(std::shared_ptr<OP> _op) : op(_op) {}
};

struct OpComplete : sc::event<OpComplete> {};
struct IOProxyWorker;
struct Waiting;
struct Processing;
}  // namespace hvs

namespace boost {
namespace statechart {
// The following class member specialization ensures that
// state_machine<>::initiate is not instantiated at a point where Waiting
// is not defined yet.
template <>
inline void asynchronous_state_machine<hvs::IOProxyWorker, hvs::Waiting,
                                       hvs::IOProxy_scheduler,
        hvs::IOProxy_allocator>::initiate_impl() {}
}  // namespace statechart
}  // namespace boost

namespace hvs {
struct IOProxyWorker
    : sc::asynchronous_state_machine<IOProxyWorker, Waiting, IOProxy_scheduler,
            IOProxy_allocator> {
 public:
  IOProxyWorker(my_context ctx) : my_base(ctx) {}

  // local variables or functions
  uint8_t worker_id;
  uint8_t scher_id;
  std::shared_ptr<OP> cur_op;

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
}  // namespace hvs
