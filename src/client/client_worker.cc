/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:36:13
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:36:13
 */
#include "client/client_worker.h"
#include "client/fuse_mod.h"
#include "client/msg_mod.h"
#include "client/queue.h"

using namespace gvds;

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
  if (worker.cur_buf->path == "") {
    // fake buf, do nothing
    if (worker.cur_buf->callback) worker.cur_buf->callback();
    callback();
    return;
  }
  if (!node->fuse->use_udt) {
    ///> call **async** op processing function
    int channel_id = node->queue->get_spare_channel() + 1;
    auto oper = node->rpc->get_operator(worker.cur_buf->dest, channel_id);
    OpRequest request;
    request.set_type(OpType::write);
    request.set_filepath(worker.cur_buf->path);
    request.mutable_io_param()->set_size(worker.cur_buf->buf.size);
    request.mutable_io_param()->set_offset(worker.cur_buf->offset);
    request.set_data(worker.cur_buf->buf.ptr, worker.cur_buf->buf.size);
    oper->SubmitAsync(request, callback);
  } else {
    auto udtc = node->rpc->udt_channel(worker.cur_buf->dest, false);
    // if (!udtc.get()) return -ETIMEDOUT;
    ioproxy_rpc_buffer buf(*(worker.cur_buf.get()));
    int id = udtc->write(buf);
    if (id == -ECONNRESET) {
      udtc = node->rpc->udt_channel(worker.cur_buf->dest, true);
      // if (!udtc.get()) return -ETIMEDOUT;
      id = udtc->write(buf);
    }
    // if(id < 0)
    //   return -ECONNREFUSED;
    // not waiting, result immediately
    udtc->registe_handler(id, callback);
  }
  //  node->rpc->async_call(worker.cur_buf->dest, "iop_write", callback,
  //  worker.cur_buf);
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
  size_t bufsize = worker.cur_buf->buf.size;
  worker.cur_buf->destroy();
  node->queue->done_one(0, bufsize);
  return transit<Waiting>();
}

void ClientWorker::initiate_impl() {
  sc::state_machine<ClientWorker, Waiting, Client_allocator>::initiate();
}