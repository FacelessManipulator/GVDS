#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include "context.h"

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/thread/thread.hpp>
#include "common/Thread.h"
#include "client/client_worker.h"
#include "client/client.h"
#include "common/Thread.h"
#include "gvds_struct.h"
#include "common/buffer.h"

namespace gvds {

class ClientBufferQueue : public ClientModule, Thread {
 private:
  virtual void start() override;
  virtual void stop() override;

 public:
  ClientBufferQueue(const char* name, Client* cli) : ClientModule(name, cli), m_stop(false) {
    auto _config = HvsContext::get_context()->_config;
    isThread = true;
    m_max_buf = _config->get<int>("client.max_queue").value_or(10240000);
    m_max_worker = _config->get<int>("client.onlink").value_or(1024);
    multi_channel = _config->get<int>("client.multi_channel").value_or(1);
    channel_loads.resize(multi_channel, 0);
    pthread_mutexattr_t mu_attr;
    pthread_mutexattr_init(&mu_attr);
    pthread_mutexattr_settype(&mu_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_queue_mutex, &mu_attr);
    pthread_mutex_init(&m_dispatch_mutex, &mu_attr);
    pthread_cond_init(&m_cond_dispatcher, nullptr);
    pthread_cond_init(&m_cond_ioproxy, nullptr);
    pthread_mutexattr_destroy(&mu_attr);
  }
  friend class Client;
  
  public:
  bool queue_buffer(std::shared_ptr<Buffer> buf, bool block = true);
  std::future<bool> block_on_last(std::shared_ptr<IOProxyNode> iop);
  void done_one(int channel_id = 0, size_t bufsize = 0);
  int get_spare_channel();
  bool add_idle_worker(ClientWorker* woker);

  ~ClientBufferQueue() override {
    stop();
    pthread_mutex_destroy(&m_queue_mutex);
    pthread_mutex_destroy(&m_dispatch_mutex);
    pthread_cond_destroy(&m_cond_dispatcher);
    pthread_cond_destroy(&m_cond_ioproxy);
  };

 private:
  void* entry() override;
  void _dispatch();
  void _dispatch_unsafe(std::queue<std::shared_ptr<Buffer>>* t);
  ClientWorker* _get_idle_worker();

 private:
  std::vector<boost::thread*> worker_threads;
  std::vector<std::shared_ptr<Client_scheduler>> schedulers;
//  boost::lockfree::spsc_queue<ClientWorker* ,
//                              boost::lockfree::capacity<10000>>
//      idle_list;
  std::queue<ClientWorker*> idle_list;
  std::mutex idle_list_mu;
  std::queue<std::shared_ptr<Buffer>> buf_waiting_line;
  int m_max_buf;  // the max number of op in ioproxy
  int m_max_worker;      // the max number of worker
  int multi_channel;    // channel number
  std::atomic_long idle_worker_num;
  std::atomic_long buf_onlink;
  std::atomic_long buf_inqueue;
  std::vector<int> channel_loads;

 private:
  // thread saft variables
  pthread_mutex_t m_queue_mutex;
  pthread_mutex_t m_dispatch_mutex;
  pthread_cond_t m_cond_ioproxy;     // wait on when processing buf exceeds max limits
  pthread_cond_t m_cond_dispatcher;  // wait on when buf waiting line is empty

  pthread_t m_queue_mutex_holder;
  pthread_t m_dispatcher_mutex_holder;
  bool m_stop;

  public:
  friend class ClientWorker;
};
}  // namespace gvds