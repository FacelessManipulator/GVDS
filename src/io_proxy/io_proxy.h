#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include "context.h"

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/thread/thread.hpp>
#include "common/Thread.h"
#include "io_proxy/io_worker.h"
#include "msg/op.h"
#include "msg/node.h"

namespace hvs {

class IOProxy : public Thread, public Node {
 public:
  IOProxy() : m_stop(false), Node(IO_PROXY_NODE) {
    // TODO: should read from config file
    m_max_op = 1000;
    m_max_worker = 1024;
  }
  void start();
  void stop();
  bool queue_op(std::shared_ptr<OP> op, bool block = true);
  bool queue_and_wait(std::shared_ptr<OP> op);
  bool add_idle_worker(IOProxyWorker* woker);
  ~IOProxy() override {stop();};

 private:
  void* entry() override;
  void _dispatch();
  void _dispatch_unsafe(std::queue<std::shared_ptr<OP>>* t);
  IOProxyWorker* _get_idle_worker();

 private:
  std::vector<boost::thread*> worker_threads;
  std::vector<std::shared_ptr<IOProxy_scheduler>> schedulers;
  boost::lockfree::spsc_queue<IOProxyWorker* ,
                              boost::lockfree::capacity<1024>>
      idle_list;
  std::queue<std::shared_ptr<OP>> op_waiting_line;
  int m_max_op;  // the max number of op in ioproxy
  int m_max_worker;      // the max number of worker

 private:
  // thread saft variables
  pthread_mutex_t m_queue_mutex;
  pthread_mutex_t m_dispatch_mutex;
  pthread_cond_t m_cond_ioproxy;     // wait on when processing op exceeds max limits
  pthread_cond_t m_cond_dispatcher;  // wait on when op waiting line is empty

  pthread_t m_queue_mutex_holder;
  pthread_t m_dispatcher_mutex_holder;
  bool m_stop;
};
extern hvs::IOProxy* init_ioproxy();
extern void destroy_ioproxy(hvs::IOProxy* iop);
}  // namespace hvs