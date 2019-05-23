#include "udt_client.h"
#include "context.h"
using namespace hvs;
using namespace std;

UDTClient::UDTClient() {
  auto _config = HvsContext::get_context()->_config;
  auto _pb = _config->get<int>("client.data_port_begin");
  auto _pe = _config->get<int>("client.data_port_end");
  auto _bs = _config->get<int>("client.data_buffer");
  port_cur = _pb.value_or(9096);
  port_left = _pe.value_or(9150) - port_cur;
  buff_size = _bs.value_or(10240000);
  UDT::startup();
}

std::shared_ptr<ClientSession> UDTClient::create_session(
    const std::string& ip, const unsigned short port) {
  struct addrinfo hints, *local, *peer;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  string local_port(to_string(port_cur));
  if (0 != getaddrinfo(NULL, local_port.c_str(), &hints, &local)) {
    dout(10) << "WARNING: [udt client] incorrect local address.\n" << dendl;
    return nullptr;
  }

  UDTSOCKET session_fd =
      UDT::socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
  UDT::setsockopt(session_fd, 0, UDT_RCVBUF, &buff_size, sizeof(int));
  UDT::setsockopt(session_fd, 0, UDP_RCVBUF, &buff_size, sizeof(int));
  string port_s = to_string(port);
  if (0 != getaddrinfo(ip.c_str(), port_s.c_str(), &hints, &peer)) {
    dout(5) << "ERROR: [udt-client] incorrect server address. " << ip << ":"
            << port << dendl;
    return nullptr;
  }

  // connect to the server, implict bind
  if (UDT::ERROR == UDT::connect(session_fd, peer->ai_addr, peer->ai_addrlen)) {
    dout(5) << "ERROR: [udt-client] connect: "
            << UDT::getlasterror().getErrorMessage() << dendl;
    return nullptr;
  }
  freeaddrinfo(peer);
  // connected
  auto se_ref = make_shared<ClientSession>(this, session_fd);
      sessions[session_fd] = se_ref;
      se_ref->start();
  return se_ref;
}

void UDTClient::close_session(std::shared_ptr<ClientSession>& session) {
    session->close();
    UDT::close(session->socket_);
    sessions.erase(session->socket_);
}