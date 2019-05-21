#include "msg/udt_server.h"
#include "context.h"

using namespace hvs;
using namespace std;

bool UDTServer::start() {
  auto _config = HvsContext::get_context()->_config;
  auto _pt = _config->get<int>("ioproxy.data_port");
  auto _bs = _config->get<int>("ioproxy.data_buffer");
  auto _mc = _config->get<int>("ioproxy.data_conn");
  port = _pt.value_or(9095);
  buff_size = _bs.value_or(10240000);  // default 10MB
  max_conn =
      _mc.value_or(50);  // default max 1000 pending connection per server

  addrinfo hints;
  addrinfo* res;

  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  // hints.ai_socktype = SOCK_DGRAM;

  string service = to_string(port);

  if (0 != getaddrinfo(NULL, service.c_str(), &hints, &res)) {
    dout(-1) << "ERROR: illegal port number for udt or port is busy." << dendl;
    return 0;
  }

  serv_fd = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  // UDT Options
  //   UDT::setsockopt(serv, 0, UDT_CC, new CCCFactory<CUDPBlast>,
  //                   sizeof(CCCFactory<CUDPBlast>));
  //   UDT::setsockopt(serv, 0, UDT_MSS, new int(9000), sizeof(int));
  UDT::setsockopt(serv_fd, 0, UDT_RCVBUF, &buff_size, sizeof(int));
  UDT::setsockopt(serv_fd, 0, UDP_RCVBUF, &buff_size, sizeof(int));

  if (UDT::ERROR == UDT::bind(serv_fd, res->ai_addr, res->ai_addrlen)) {
    dout(-1) << "ERROR: cannot bind udt port: "
             << UDT::getlasterror().getErrorMessage() << dendl;
    return 0;
  }

  freeaddrinfo(res);

  dout(5) << "DEBUG: udt server is ready at port: " << service << dendl;

  if (UDT::ERROR == UDT::listen(serv_fd, max_conn)) {
    dout(-1) << "ERROR: udt listen failed: "
             << UDT::getlasterror().getErrorMessage() << dendl;
    return 0;
  }

  sockaddr_storage clientaddr;
  int addrlen = sizeof(clientaddr);

  epoll_fd = UDT::epoll_create();
  {
    int add_usock_ret = UDT::epoll_add_usock(epoll_fd, serv_fd, &read_event);
    if (add_usock_ret < 0)
      dout(-1) << "ERROR: UDT::epoll_add_usock error: " << add_usock_ret
               << dendl;
  }
  dout(5) << "DEBUG: udt server enter loops." << dendl;
  m_stop = false;
  create("udt server");
  return true;
}

void UDTServer::stop() { m_stop = false; }

void* UDTServer::entry() {
  while (!m_stop) {
    std::set<UDTSOCKET> readfds;
    std::set<UDTSOCKET> writefds;
    // check per 1s
    dout(-1) << "udt server epoll waiting" << dendl;
    int state =
        UDT::epoll_wait(epoll_fd, &readfds, &writefds, -1, NULL, NULL);
    dout(-1) << "udt server epoll wait return" << dendl;
    if (state > 0) {
      // read
      handleReadFds(readfds, serv_fd);
    } else if (state == 0) {
      // timeout
      // std::cout << "." << std::flush;
    } else {
      if ((CUDTException::EINVPARAM == UDT::getlasterror().getErrorCode()) ||
          (CUDTException::ECONNLOST == UDT::getlasterror().getErrorCode())) {
        m_stop = true;
        // UDT::epoll_remove_usock(eid, cur_sock);
          // error happend
          std::cout << "UDT epoll_wait: " << UDT::getlasterror().getErrorCode()
                    << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
      } else {
          // maybe timeout
          // std::cout << "." << std::flush;
      }
    }
  }
  dout(5) << "DEBUG: stopping UDT epoll." << dendl;
  int release_state = UDT::epoll_release(epoll_fd);
  if (release_state != 0)
    dout(-1) << "ERROR: UDT epoll_release: " << release_state << dendl;

  int close_state = UDT::close(serv_fd);
  if (close_state != 0)
    dout(-1) << "ERROR: UDT close failed:" << UDT::getlasterror().getErrorCode()
             << ' ' << UDT::getlasterror().getErrorMessage() << dendl;

  dout(5) << "DEBUG: udt stopped" << dendl;
  return 0;
}

void UDTServer::handleReadFds(const std::set<UDTSOCKET>& readfds,
                              const UDTSOCKET& listen_sock_) {
  dout(-1) << "epoll event on server" << dendl;
  for (const UDTSOCKET cur_sock : readfds) {
    // new connection
    if (cur_sock == listen_sock_) {
      sockaddr addr;
      int addr_len;
      UDTSOCKET new_sock = UDT::accept(listen_sock_, &addr, &addr_len);
      if (new_sock == UDT::INVALID_SOCK) {
        // error accept
        dout(5) << "WARNNING: UDT accept failed:"
                << UDT::getlasterror().getErrorCode() << ' '
                << UDT::getlasterror().getErrorMessage() << dendl;
        continue;
      } else {
        // session information
        char clienthost[NI_MAXHOST];
        char clientport[NI_MAXSERV];
        getnameinfo((sockaddr*)&addr, addr_len, clienthost, sizeof(clienthost),
                    clientport, sizeof(clientport),
                    NI_NUMERICHOST | NI_NUMERICSERV);
        dout(15) << "INFO: udt accept a new connection from " << clienthost
                 << ":" << clientport << dendl;
        int add_usock_ret =
            UDT::epoll_add_usock(epoll_fd, new_sock, &read_event);
        if (add_usock_ret < 0) {
          // error add sock to epoll
          dout(5) << "WARNING: UDT::epoll_add_usock new_sock add error: "
                  << add_usock_ret << dendl;
        } else {
          // create sessions
          // check if exists before
          auto session = make_shared<ServerSession>(this, new_sock);
          sessions[new_sock] = session;
        }
      }
    } else {
      // handle read request
      do {
        auto it = sessions.find(cur_sock);
        it->second->do_read();
      } while (false);
    }
  }
}

void* UDTServer::recvdata(const UDTSOCKET recver) {
  char* data;
  data = new char[buff_size];

  while (true) {
    int rsize = 0;
    int rs;
    while (rsize < buff_size) {
      int rcv_size;
      int var_size = sizeof(int);
      UDT::getsockopt(recver, 0, UDT_RCVDATA, &rcv_size, &var_size);
      if (UDT::ERROR ==
          (rs = UDT::recv(recver, data + rsize, buff_size - rsize, 0))) {
        cout << "recv:" << UDT::getlasterror().getErrorMessage() << endl;
        break;
      }
      rsize += rs;
    }
  }

  delete[] data;

  //   UDT::close(recver);
  return 0;
}

namespace hvs {
UDTServer* init_udtserver() {
  UDTServer* udt_server = new UDTServer();
  udt_server->start();
  return udt_server;
}
}  // namespace hvs