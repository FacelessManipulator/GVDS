#pragma once

#include <udt.h>
#include <boost/asio.hpp>
#include <memory>
#include <rpc/msgpack.hpp>
#include <vector>
#include "common/Thread.h"
#include <queue>

namespace hvs {

class UDTWriter : public Thread {
 public:
  UDTWriter(UDTSOCKET socket): socket_(socket), m_stop(true) {};
  void start();
  void close();
  void write(clmdep_msgpack::sbuffer &&data);

 protected:
  void *entry() override;
  void do_write();
  void _write_unsafe(std::queue<clmdep_msgpack::sbuffer>* q);

 private:
  UDTSOCKET socket_;
  bool m_stop;

 private:
  std::queue<clmdep_msgpack::sbuffer> m_new;
  pthread_mutex_t m_queue_mutex;
  pthread_mutex_t m_writer_mutex;
  pthread_cond_t m_cond_writer;
  pthread_cond_t m_cond_session;
  unsigned long m_max_write;
};
}  // namespace hvs
