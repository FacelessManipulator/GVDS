#pragma once

#include <udt.h>
#include <boost/asio.hpp>
#include <memory>
#include <rpc/msgpack.hpp>
#include <vector>
#include <unordered_map>
#include <future>
#include <atomic>
#include "io_proxy/rpc_types.h"
#include "msg/udt_writer.h"

namespace hvs {

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
  
  std::unique_ptr<ioproxy_rpc_buffer> wait_op(int id);

 protected:
  std::shared_ptr<UDTWriter> writer;

 private:
  std::unordered_map<int, std::future<std::unique_ptr<ioproxy_rpc_buffer>>> futures;
  std::unordered_map<int, std::promise<std::unique_ptr<ioproxy_rpc_buffer>>> ready_promises;
  UDTClient *parent;
  clmdep_msgpack::unpacker unpacker;
  UDTSOCKET socket_;
  bool m_stop = false;
  std::atomic_uint64_t seq_n;
  friend class UDTClient;
};
}  // namespace hvs
