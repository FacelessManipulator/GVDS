#pragma once

#include <udt.h>
#include <boost/asio.hpp>
#include <memory>
#include <rpc/msgpack.hpp>
#include <vector>
#include "common/Thread.h"

namespace hvs {

class UDTServer;

class UDTSession : public Thread {
 public:
  UDTSession(UDTServer *srv, UDTSOCKET socket);
  void start();
  void close();
  void do_read();
  void do_write();
  void write(clmdep_msgpack::sbuffer &&data) {
    write_queue_.push_back(std::move(data));
    if (write_queue_.size() > 1) {
      return;  // there is an ongoing write chain so don't start another
    }
    do_write();
  }

 protected:
  void *entry() override;

 private:
  UDTServer *parent;
  clmdep_msgpack::unpacker unpacker;
  clmdep_msgpack::sbuffer output_buf;
  UDTSOCKET socket_;
  bool m_stop = false;
  std::mutex session_mu;
  std::condition_variable session_cond;

 private:
  std::deque<clmdep_msgpack::sbuffer> write_queue_;
};
}  // namespace hvs
