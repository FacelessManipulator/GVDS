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
      : status(Stopped) {
    auto id = boost::uuids::random_generator()();
    uuid = boost::lexical_cast<std::string>(id);
  }

  explicit IOProxyNode(const char* _key) {
    key(_key);
  }

  IOProxyNode& operator=(const IOProxyNode& oths) {
    rpc_port = oths.rpc_port;
    data_port = oths.data_port;
    ip = oths.ip;
    name = oths.name;
    status = oths.status;
    uuid = oths.uuid;
  }
  
  virtual void serialize_impl() override {
    put("uuid", uuid);
    put("ip", ip);
    put("name", name);
    put("rpc_port", rpc_port);
    put("data_port", data_port);
    int s = status;
    put("status", s);
  };
  virtual void deserialize_impl() override {
    get("uuid", uuid);
    get("ip", ip);
    get("name", name);
    get("rpc_port", rpc_port);
    get("data_port", data_port);
    int s;
    get("status", s);
    status = static_cast<Status>(s);
  };
  void key(const char* key) {
    boost::uuids::uuid tag;
    std::stringstream ss(key);
    ss >> tag;
    uuid = boost::lexical_cast<std::string>(tag);
  }
  std::string json_value() { return serialize(); }
};

struct Space {
 public:
  std::string name;
  std::string uuid;
  int64_t size;
  Space(const std::string& _name) : name(_name) {}
  Space(const std::string& _name, std::string _uuid, int64_t _size) : name(_name), uuid(_uuid), size(_size) {}
};

struct Zone {
public:
    std::string name;
    boost::uuids::uuid tag;
    Zone(const std::string& _name) : name(_name) {}
};

}  // namespace hvs