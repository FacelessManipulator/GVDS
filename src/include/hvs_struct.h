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
  std::string cid;
  static const std::string& prefix() {
    static std::string _prefix("IOPN-");
    return _prefix;
  }
  explicit IOProxyNode() : status(Stopped) {
    auto id = boost::uuids::random_generator()();
    uuid = boost::lexical_cast<std::string>(id);
  }

  explicit IOProxyNode(const char* _key) { key(_key); }

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
    put("cid", cid);
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
    get("cid", cid);
  };
  void key(const char* key) {
    boost::uuids::uuid tag;
    std::stringstream ss(key);
    ss >> tag;
    uuid = boost::lexical_cast<std::string>(tag);
  }
  std::string json_value() { return serialize(); }
};

struct Space : public hvs::JsonSerializer {
 public:
  std::string spaceID;       //空间ID,UUID
  std::string spaceName;     //空间名
  int64_t spaceSize;         //空间容量
  std::string hostCenterID;  //存储资源所在超算中心UUID
  std::string storageSrcID;  // 存储资源UUID
  std::string spacePath;     // 空间所在存储集群的路径
  std::string hostCenterName;
  std::string storageSrcName;
  bool status;  //空间可用状态

 public:
  void serialize_impl() override {
    put("UUID", spaceID);
    put("name", spaceName);
    put("capcity", spaceSize);
    put("SC_UUID", hostCenterID);
    put("Storage_UUID", storageSrcID);
    put("root_location", spacePath);
    put("hostCenterName", hostCenterName);
    put("storageSrcName", storageSrcName);
    put("Status", status);
  };

  void deserialize_impl() override {
    get("UUID", spaceID);
    get("name", spaceName);
    get("capcity", spaceSize);
    get("SC_UUID", hostCenterID);
    get("Storage_UUID", storageSrcID);
    get("root_location", spacePath);
    get("hostCenterName", hostCenterName);
    get("storageSrcName", storageSrcName);
    get("Status", status);
  };

 public:
  Space() = default;
    Space& operator = (const Space& oths) {
        spaceID = oths.spaceID;
        spaceName = oths.spaceName;
        spaceSize = oths.spaceSize;
        hostCenterID = oths.hostCenterID;
        storageSrcID = oths.storageSrcID;
        spacePath = oths.spacePath;
        hostCenterName = oths.hostCenterName;
        storageSrcName = oths.storageSrcName;
        status = oths.status;
    }
};

struct SpaceRequest : public hvs::JsonSerializer {
  enum SpaceRequestType {
    rename,
    sizeChange,
  };

 public:
  SpaceRequestType type;
  std::string spaceID;       //空间ID
  int64_t newSpaceSize;      //空间容量
  std::string newSpaceName;  //区域名
  void serialize_impl() {
    put("UUID", spaceID);
    int t = type;
    put("type", t);
    put("newSpaceSize", newSpaceSize);
    put("newSpaceName", newSpaceName);
  }

  void deserialize_impl() {
    get("UUID", spaceID);
    int t;
    get("type", t);
    type = static_cast<SpaceRequestType>(t);
    get("newSpaceSize", newSpaceSize);
    get("newSpaceName", newSpaceName);
  }
};

struct Zone : public hvs::JsonSerializer {
 public:
  std::string zoneID;                 //区域ID,UUID
  std::string zoneName;               //区域名
  std::string ownerID;                //区域主人ID，UUID
  std::vector<std::string> memberID;  //区域成员ID，UUID
  std::vector<std::string> spaceID;   //区域映射空间ID，UUID
  std::vector<std::shared_ptr<Space>> spaceBicInfo;    //空间基本信息
  bool contains_spaceinfo;

 public:
  void serialize_impl() override {
    put("UUID", zoneID);
    put("name", zoneName);
    put("owner", ownerID);
    put("members", memberID);
    put("spaces", spaceID);
    if (contains_spaceinfo) put("spaceinfo", spaceBicInfo);
  }

  void deserialize_impl() override {
    get("UUID", zoneID);
    get("name", zoneName);
    get("owner", ownerID);
    get("members", memberID);
    get("spaces", spaceID);
    if (contains_spaceinfo) get("spaceinfo", spaceBicInfo);
  }
  Zone& operator = (const Zone& oths) {
      zoneID = oths.zoneID;
      zoneName = oths.zoneName;
      ownerID = oths.ownerID;
      memberID = oths.memberID;
      spaceID = oths.spaceID;
      spaceBicInfo = oths.spaceBicInfo;
      contains_spaceinfo = oths.contains_spaceinfo;
  }

 public:
  Zone() : contains_spaceinfo(true){};
};

struct ZoneRequest : public hvs::JsonSerializer {
  enum ZoneRequestType {
    rename,
    locateInfo,
    share,
    registe,
    cancel,
    mapadd,
    mapdeduct
  };

 public:
  ZoneRequestType type;
  std::string zoneID;  //空间ID
  std::string clientID;
  std::vector<std::string> memberID;  //区域成员ID，UUID
  std::vector<std::string> spaceID;   //区域成员ID，UUID
  std::string ownerID;                //区域主人账户ID
  std::string zoneName;               //区域名
  std::string newZoneName;            //区域名
  std::string spaceName;
  int64_t spaceSize;
  std::string spacePathInfo;
  void serialize_impl() {
    put("UUID", zoneID);
    int t = type;
    put("type", t);
    put("clientID", clientID);
    put("memberID", memberID);
    put("spaceID", spaceID);
    put("ownerID", ownerID);
    put("ZoneName", zoneName);
    put("newZoneName", newZoneName);
    put("spaceSize", spaceSize);
    put("spaceName", spaceName);
    put("spacePathInfo", spacePathInfo);
  }

  void deserialize_impl() {
    get("UUID", zoneID);
    int t;
    get("type", t);
    type = static_cast<ZoneRequestType>(t);
    get("clientID", clientID);
    get("memberID", memberID);
    get("spaceID", spaceID);
    get("ownerID", ownerID);
    get("ZoneName", zoneName);
    get("newZoneName", newZoneName);
    get("spaceSize", spaceSize);
    get("spaceName", spaceName);
    get("spacePathInfo", spacePathInfo);
  }
};

}  // namespace hvs