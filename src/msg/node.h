#pragma once

#include <boost/asio/ip/address.hpp>
#include <string>

namespace gvds {
class RpcServer;
enum GVDSNodeType {
  IO_PROXY_NODE,
  MANAGER_NODE,
  CLIENT_NODE,
};
class Node {
 public:
  std::string name;
  std::string uuid;
  GVDSNodeType type;
  boost::asio::ip::address addr;
  std::string center_id;
  Node(const std::string& _name, GVDSNodeType _type, const std::string& ip)
      : name(_name), type(_type) {
    addr = boost::asio::ip::make_address(ip);
  }
  Node(GVDSNodeType _type) : type(_type){};
  virtual void rpc_bind(RpcServer* server) {}
};
}  // namespace gvds