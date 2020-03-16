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

#include <boost/pool/pool_alloc.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>

#include "common/buffer.h"
#include "context.h"
#include "gvds_struct.h"

namespace gvds {

namespace sc = ::boost::statechart;
namespace mpl = ::boost::mpl;
typedef std::allocator<void> Client_allocator;
typedef sc::fifo_scheduler<> Client_scheduler;
class ClientBufferQueue;
// event buf queued
struct BufferQueued : sc::event<BufferQueued> {
  std::shared_ptr<Buffer> buf;
  BufferQueued(std::shared_ptr<Buffer> _buf)
      : buf(_buf) {}
};

struct BufferComplete : sc::event<BufferComplete> {};
struct ClientWorker;
struct Waiting;
struct Processing;
}  // namespace gvds

namespace boost {
namespace statechart {
// The following class member specialization ensures that
// state_machine<>::initiate is not instantiated at a point where Waiting
// is not defined yet.
template <>
inline void asynchronous_state_machine<gvds::ClientWorker, gvds::Waiting,
                                       gvds::Client_scheduler,
                                       gvds::Client_allocator>::initiate_impl() {
}
}  // namespace statechart
}  // namespace boost

namespace gvds {
struct ClientWorker
    : sc::asynchronous_state_machine<ClientWorker, Waiting, Client_scheduler,
                                     Client_allocator> {
 public:
  ClientWorker(my_context ctx) : my_base(ctx) {}

  // local variables or functions
  uint8_t worker_id;
  uint8_t scher_id;
  std::shared_ptr<Buffer> cur_buf;
  ClientBufferQueue* queue;

  // This function is defined in Client.cc
  virtual void initiate_impl();
};

struct Waiting : sc::state<Waiting, ClientWorker> {
 public:
  typedef mpl::list<sc::custom_reaction<BufferQueued>> reactions;

  Waiting(my_context ctx);
  ~Waiting();

  sc::result react(const BufferQueued &);
};

struct Processing : sc::state<Processing, ClientWorker> {
 public:
  typedef mpl::list<sc::custom_reaction<BufferComplete>> reactions;

  Processing(my_context ctx);
  ~Processing();

  sc::result react(const BufferComplete &);
};
}  // namespace gvds
