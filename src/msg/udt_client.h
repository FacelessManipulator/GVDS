#pragma once

#include <netdb.h>
#include <udt.h>
#include <unistd.h>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <thread>
#include <unordered_map>
#include "msg/client_session.h"
#include "rpc/msgpack.hpp"
#include <mutex>

namespace gvds {

class UDTClient {
 public:
  UDTClient();
  ~UDTClient() { UDT::cleanup(); }

  // not thread safe
  std::shared_ptr<ClientSession> create_session(const std::string& ip,
                                                const unsigned short port);
  void close_session(std::shared_ptr<ClientSession>& session);

 private:
 private:
  std::unordered_map<UDTSOCKET, std::shared_ptr<ClientSession>> sessions;
  // the first port unused in range [data_port_begin, data_port_end]
  int port_cur;
  int port_left;
  unsigned long buff_size;
  unsigned long bw; // max bandwidth for blast UDT

  friend class UDTSession;
  friend class ClientSession;
};

}  // namespace gvds