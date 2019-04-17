#pragma once

#include <boost/asio/ip/address.hpp>
#include <string>
#include "common/Thread.h"

namespace hvs {
enum HVSNodeType {
  IO_PROXY_NODE,
  MANAGER_NODE,
  CLIENT_NODE,
};
class Node {
 public:
  std::string name;
  HVSNodeType type;
  boost::asio::ip::address addr;
  Node(const std::string& _name, HVSNodeType _type, const std::string& ip)
      : name(_name), type(_type) {
    addr = boost::asio::ip::make_address(ip);
  }
  Node(HVSNodeType _type) : type(_type){};
};
}  // namespace hvs