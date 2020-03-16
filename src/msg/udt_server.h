#pragma once

#include <netdb.h>
#include <udt.h>
#include <unistd.h>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <set>
#include <thread>
#include "common/Thread.h"
#include "rpc/msgpack.hpp"
#include "msg/server_session.h"

namespace gvds {

class UDTServer : public Thread {
 public:
  UDTServer() { UDT::startup(); }
  ~UDTServer() { UDT::cleanup(); }

  bool start();
  void stop();
  void* entry() override;

  void handleReadFds(const std::set<UDTSOCKET>& readfds,
                     const UDTSOCKET& listen_sock_);
  void* recvdata(const UDTSOCKET);

 private:
  unsigned short port;
  unsigned long buff_size;
  unsigned long max_conn;
  unsigned long bw;
  UDTSOCKET epoll_fd;
  UDTSOCKET serv_fd;
  std::unordered_map<UDTSOCKET, std::shared_ptr<ServerSession>> sessions;
  const int read_event = UDT_EPOLL_IN | UDT_EPOLL_ERR;
  bool m_stop;

  friend class ServerSession;
  friend class IOProxy;
};

UDTServer* init_udtserver();

}  // namespace gvds