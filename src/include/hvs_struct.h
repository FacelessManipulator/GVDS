#pragma once
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "common/json.h"

namespace hvs {

struct NodeStatistics {
  unsigned long uptime;
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
  std::string uuid;
  static const std::string& prefix() {
    static std::string _prefix("IOPN-");
    return _prefix;
  }
  explicit IOProxyNode()
      : tag(), status(Stopped) {
    auto id = boost::uuids::random_generator()();
    key(boost::lexical_cast<std::string>(tag).c_str());
  }

  explicit IOProxyNode(const char* _key) {
    key(_key);
  }
  virtual void serialize_impl() override {
    put("uuid", uuid);
    put("ip", ip);
    put("name", name);
    put("rpc_port", rpc_port);
    put("data_port", data_port);
  };
  virtual void deserialize_impl() override {
    get("uuid", uuid);
    get("ip", ip);
    get("name", name);
    get("rpc_port", rpc_port);
    get("data_port", data_port);
  };
  void key(const char* key) {
    std::stringstream ss(key);
    ss >> tag;
    uuid = prefix() + boost::lexical_cast<std::string>(tag);
  }
  std::string json_value() { return serialize(); }
};

struct Space {
 public:
  std::string name;
  boost::uuids::uuid tag;
  Space(const std::string& _name) : name(_name) {}
};

}  // namespace hvs