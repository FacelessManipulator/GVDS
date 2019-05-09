#pragma once
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "common/json.h"

namespace hvs {

struct NodeStatistics {
  
};

struct IOProxyNode : public hvs::JsonSerializer {
  enum Status {
    Running = 0,
    Stopped,
    Starting,
  };
  boost::uuids::uuid tag;
  int rpc_port;
  int data_port;
  std::string ip;
  std::string name;
  Status status;
  static const std::string& prefix() {
      static std::string _prefix("IOPN-");
      return _prefix;
  }
  explicit IOProxyNode()
      : tag(boost::uuids::random_generator()()),
        status(Stopped) {
    name = boost::lexical_cast<std::string>(tag);
  }

  explicit IOProxyNode(const char* uuid) {
    std::stringstream ss(uuid);
    ss >> tag;
  }
  virtual void serialize_impl() override {
    put("ip", ip);
    put("name", name);
    put("rpc_port", rpc_port);
    put("data_port", data_port);
  };
  virtual void deserialize_impl() override {
    get("ip", ip);
    get("name", name);
    get("rpc_port", rpc_port);
    get("data_port", data_port);
  };
  void key(const char* key) {
    std::stringstream ss(key);
    ss >> tag;
  }
  std::string key() {
    return prefix() + boost::lexical_cast<std::string>(tag);
  }
  std::string json_value() {
    return serialize();
  }
};

}  // namespace hvs