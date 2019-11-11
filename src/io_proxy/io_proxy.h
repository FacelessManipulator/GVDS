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
#include "io_proxy/proxy_op.h"
#include "io_proxy/fd_mgr.h"
#include "msg/op.h"
#include "msg/node.h"
#include "msg/udt_server.h"
#include "io_proxy/io_mon.h"

namespace hvs {
class IOProxy : public Thread, public Node {
 public:
  IOProxy() : m_stop(false), Node(IO_PROXY_NODE), _rpc(nullptr), proxy_op(this), fdm(this), iom(this) {
    // TODO: should read from config file
    m_max_op = 1000;
    m_max_worker = 1024;
    pthread_mutexattr_t mu_attr;
    pthread_mutexattr_init(&mu_attr);
    pthread_mutexattr_settype(&mu_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_queue_mutex, &mu_attr);
    pthread_mutex_init(&m_dispatch_mutex, &mu_attr);
    pthread_cond_init(&m_cond_dispatcher, nullptr);
    pthread_cond_init(&m_cond_ioproxy, nullptr);
    pthread_mutexattr_destroy(&mu_attr);
  }
  bool start();
  void stop();
  std::string absolute_path(const std::string& path_rel) {
    std::string path_abs(data_path);
    path_abs.append(path_rel);
    return path_abs;
  }
  bool queue_op(std::shared_ptr<OP> op, bool block = true);
  bool queue_and_wait(std::shared_ptr<OP> op);
  bool queue_and_wait(const std::vector<std::shared_ptr<OP>>& ops);
  bool add_idle_worker(IOProxyWorker* woker);
  ~IOProxy() override {
    stop();
    pthread_mutex_destroy(&m_queue_mutex);
    pthread_mutex_destroy(&m_dispatch_mutex);
    pthread_cond_destroy(&m_cond_dispatcher);
    pthread_cond_destroy(&m_cond_ioproxy);
  };

 private:
  void* entry() override;
  void fresh_stat();
  void _dispatch();
  void _dispatch_unsafe(std::queue<std::shared_ptr<OP>>* t);
  IOProxyWorker* _get_idle_worker();

 private:
  std::vector<boost::thread*> worker_threads;
  std::vector<std::shared_ptr<IOProxy_scheduler>> schedulers;
//  boost::lockfree::spsc_queue<IOProxyWorker* ,
//                              boost::lockfree::capacity<1024>>
//      idle_list;

  std::queue<IOProxyWorker*> idle_list;
  std::mutex idle_list_mu;
  std::atomic<long> idle_worker_num;
  std::queue<std::shared_ptr<OP>> op_waiting_line;
  int m_max_op;  // the max number of op in ioproxy
  int m_max_worker;      // the max number of worker
  std::string manager_addr;

 private:
  // thread saft variables
  pthread_mutex_t m_queue_mutex;
  pthread_mutex_t m_dispatch_mutex;
  pthread_cond_t m_cond_ioproxy;     // wait on when processing op exceeds max limits
  pthread_cond_t m_cond_dispatcher;  // wait on when op waiting line is empty

  pthread_t m_queue_mutex_holder;
  pthread_t m_dispatcher_mutex_holder;
  std::string data_path;
  bool m_stop;

  public:
  RpcServer* _rpc;
  UDTServer* _udt;
  ProxyOP proxy_op;
  FdManager fdm;
  IOMonitor iom;
  virtual void rpc_bind(RpcServer* server) override;
  friend class IOProxyWorker;
};
extern hvs::IOProxy* init_ioproxy();
extern void destroy_ioproxy(hvs::IOProxy* iop);
}  // namespace hvs