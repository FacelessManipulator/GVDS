#pragma once

#include <udt.h>
#include <boost/asio.hpp>
#include <memory>
#include <rpc/msgpack.hpp>
#include <vector>
#include <unordered_map>
#include <future>
#include <atomic>
#include <functional>
#include "common/buffer.h"
#include "io_proxy/rpc_types.h"
#include "msg/udt_writer.h"

namespace gvds {

class UDTClient;

class ClientSession : public Thread {
 public:
  ClientSession(UDTClient *srv, UDTSOCKET socket);
  void start();
  void close();
  void* entry() override;
  void do_read();
  // return the id to be waited
  int write(ioproxy_rpc_buffer& buffer);
  void registe_handler(int id, std::function<void()> f) {
    std::lock_guard<std::mutex> lock(session_lock);
    auto_handler.insert(id);
    handlers.insert(std::make_pair(id, f));
  }
  void auto_handle(int id) {
    std::lock_guard<std::mutex> lock(session_lock);
    auto_handler.insert(id);
  }
  
  std::shared_ptr<ioproxy_rpc_buffer> wait_op(int id);
  int block_on_op(int id=-1);

 protected:
  std::shared_ptr<UDTWriter> writer;

 private:
  std::unordered_map<int, std::shared_future<std::shared_ptr<ioproxy_rpc_buffer>>> futures;
  std::unordered_map<int, std::promise<std::shared_ptr<ioproxy_rpc_buffer>>> ready_promises;
  UDTClient *parent;
  clmdep_msgpack::unpacker unpacker;
  UDTSOCKET socket_;
  bool m_stop = false;
  std::atomic_uint64_t seq_n;
  std::mutex session_lock;
  std::set<int> auto_handler;
  std::map<int, std::function<void()>> handlers;
  friend class UDTClient;
};
}  // namespace gvds
