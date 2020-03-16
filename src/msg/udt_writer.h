#pragma once

#include <udt.h>
#include <boost/asio.hpp>
#include <memory>
#include <rpc/msgpack.hpp>
#include <vector>
#include "common/Thread.h"
#include <queue>

namespace gvds {

class UDTWriter : public Thread {
 public:
  UDTWriter(UDTSOCKET socket): socket_(socket), m_stop(true), should_close(false) {
    pthread_mutexattr_t mu_attr;
    pthread_mutexattr_init(&mu_attr);
    pthread_mutexattr_settype(&mu_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_queue_mutex, &mu_attr);
    pthread_mutex_init(&m_writer_mutex, &mu_attr);
    pthread_cond_init(&m_cond_writer, nullptr);
    pthread_cond_init(&m_cond_session, nullptr);
    pthread_mutexattr_destroy(&mu_attr);
    m_pending_write = 0;
  };
  ~UDTWriter() {
    close();
    pthread_mutex_destroy(&m_queue_mutex);
    pthread_mutex_destroy(&m_writer_mutex);
    pthread_cond_destroy(&m_cond_writer);
    pthread_cond_destroy(&m_cond_session);
  }
  void start();
  void close();
  void write(clmdep_msgpack::sbuffer &&data);
  bool error_stat() {return should_close;}

 protected:
  void *entry() override;
  void do_write();
  bool _write_unsafe(std::queue<clmdep_msgpack::sbuffer>* q);

 private:
  UDTSOCKET socket_;
  bool m_stop;
  bool should_close;
  std::atomic_long m_pending_write;

 private:
  std::queue<clmdep_msgpack::sbuffer> m_new;
  pthread_mutex_t m_queue_mutex;
  pthread_mutex_t m_writer_mutex;
  pthread_cond_t m_cond_writer;
  pthread_cond_t m_cond_session;
  unsigned long m_max_write;
};
}  // namespace gvds
