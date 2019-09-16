#include "client/client_worker.h"
#include "client/queue.h"
#include "client/msg_mod.h"

using namespace hvs;

Waiting::Waiting(my_context ctx) : my_base(ctx) {
  Client* node = (Client*)HvsContext::get_context()->node;
  outermost_context_type& worker = outermost_context();
  node->queue->add_idle_worker(&worker);
  // invoked when worker turn to waiting state
}

Waiting::~Waiting() {
  // invoked when worker turn from waiting state
}

sc::result Waiting::react(const BufferQueued& bufq) {
  // react when op dispatched to this worker
  outermost_context_type& worker = outermost_context();
  worker.cur_buf = bufq.buf;
  return transit<Processing>();
}

Processing::Processing(my_context ctx) : my_base(ctx) {
  Client* node = (Client*)HvsContext::get_context()->node;
  outermost_context_type& worker = outermost_context();
  // invoked when worker turn to waiting state
  boost::intrusive_ptr<BufferComplete> callback_event = new BufferComplete();
  boost::function0<void> callback =
      boost::bind(&Client_scheduler::queue_event, &(worker.my_scheduler()),
                  worker.my_handle(), callback_event);
  ///> call **async** op processing function
  int channel_id = node->queue->get_spare_channel() + 1;
  auto rpcc = node->rpc->rpc_channel(worker.cur_buf->dest, false, channel_id);
  // the callback function could be used to trigger some event
  rpcc->async_call( "iop_write", callback, worker.cur_buf);
//  node->rpc->async_call(worker.cur_buf->dest, "iop_write", callback, worker.cur_buf);
}


Processing::~Processing() {
  // invoked when worker turn from waiting state
  outermost_context_type& worker = outermost_context();
  worker.cur_buf.reset();
}

sc::result Processing::react(const BufferComplete& op) {
  // react when op dispatched to this worker
  Client* node = (Client*)HvsContext::get_context()->node;
  outermost_context_type& worker = outermost_context();
  // invoked when worker turn to waiting state
  worker.cur_buf->destroy();
  node->queue->done_one();
  return transit<Waiting>();
}

void ClientWorker::initiate_impl() {
  sc::state_machine<ClientWorker, Waiting, Client_allocator>::initiate();
}