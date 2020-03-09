/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:38:38
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:38:38
 */
#include "io_proxy/io_worker.h"
#include "io_proxy/io_proxy.h"
#include "io_proxy/proxy_op.h"

using namespace hvs;

Waiting::Waiting(my_context ctx) : my_base(ctx) {
  IOProxy* node = (IOProxy*)HvsContext::get_context()->node;
  outermost_context_type& worker = outermost_context();
  worker.status = IOProxyWorker::WAITING;
  // drop old rpc mechanism
  // node->add_idle_worker(&worker);
  // invoked when worker turn to waiting state
  worker.cur_call.reset(new IOProxyWorker::CallData(&worker));
  worker._op_service->RequestSubmit(&worker.cur_call->_op_ctx,
                                    &worker.cur_call->request,
                                    &worker.cur_call->responder, worker._cq,
                                    worker._cq, worker.cur_call.get());
}

Waiting::~Waiting() {
  // invoked when worker turn from waiting state
}

sc::result Waiting::react(const OpQueued& op) {
  // react when op dispatched to this worker
  outermost_context_type& worker = outermost_context();
  // worker.cur_op = op.op;
  return transit<Processing>();
}

Processing::Processing(my_context ctx) : my_base(ctx) {
  IOProxy* node = (IOProxy*)HvsContext::get_context()->node;
  outermost_context_type& worker = outermost_context();
  worker.status = IOProxyWorker::PROCESSING;
  // invoked when worker turn to waiting state
  boost::intrusive_ptr<OpComplete> callback_event = new OpComplete();
  boost::function0<void> callback =
      boost::bind(&IOProxy_scheduler::queue_event, &(worker.my_scheduler()),
                  worker.my_handle(), callback_event);
  // worker.cur_op->op_submit = std::chrono::steady_clock::now();
  ///> call **async** op processing function
  // node->proxy_op.prepare_op(worker.cur_op);
  node->proxy_op.do_op(worker.cur_call->request, worker.cur_call->reply,
                       callback);
}

Processing::~Processing() {
  // invoked when worker turn from waiting state
  outermost_context_type& worker = outermost_context();
  // worker.cur_op.reset();
}

sc::result Processing::react(const OpComplete& op) {
  // react when op dispatched to this worker
  outermost_context_type& worker = outermost_context();
  // worker.cur_op->op_complete = std::chrono::steady_clock::now();
  // invoked when worker turn to waiting state
  // call complete callbacks
  // for (auto cb : worker.cur_op->complete_callbacks) cb();
  return transit<Finished>();
}

Finished::Finished(my_context ctx) : my_base(ctx) {
  IOProxy* node = (IOProxy*)HvsContext::get_context()->node;
  outermost_context_type& worker = outermost_context();
  worker.status = IOProxyWorker::FINISHED;
  worker.cur_call->responder.Finish(worker.cur_call->reply, Status::OK,
                                    worker.cur_call.get());
}

Finished::~Finished() {
  // invoked when worker turn from waiting state
  outermost_context_type& worker = outermost_context();
  // worker.cur_op.reset();
}

sc::result Finished::react(const OpFinished& op) {
  // react when op dispatched to this worker
  outermost_context_type& worker = outermost_context();
  // worker.cur_op->op_complete = std::chrono::steady_clock::now();
  // invoked when worker turn to waiting state
  // call complete callbacks
  // for (auto cb : worker.cur_op->complete_callbacks) cb();
  return transit<Waiting>();
}

void IOProxyWorker::initiate_impl() {
  sc::state_machine<IOProxyWorker, Waiting, IOProxy_allocator>::initiate();
}