/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:38:00
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:38:00
 */
#pragma once
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "common/json.h"

namespace gvds {

struct NodeStatistics {
  unsigned long uptime;
};

struct IOProxyNode : public gvds::JsonSerializer {
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

struct Space : public gvds::JsonSerializer {
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
  std::vector<std::string> replica_spaces;

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
    put("replica_spaces", replica_spaces);
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
    get("replica_spaces", replica_spaces);
  };

 public:
  Space() = default;
  Space& operator=(const Space& oths) {
    spaceID = oths.spaceID;
    spaceName = oths.spaceName;
    spaceSize = oths.spaceSize;
    hostCenterID = oths.hostCenterID;
    storageSrcID = oths.storageSrcID;
    spacePath = oths.spacePath;
    hostCenterName = oths.hostCenterName;
    storageSrcName = oths.storageSrcName;
    status = oths.status;
    replica_spaces = oths.replica_spaces;
  }
};

struct SpaceRequest : public gvds::JsonSerializer {
 public:
  std::string spaceID;  //空间ID
  std::vector<std::string> spaceIDs;
  int64_t newSpaceSize;      //空间容量
  std::string newSpaceName;  //区域名
  void serialize_impl() {
    put("UUID", spaceID);
    put("UUIDs", spaceIDs);
    put("newSpaceSize", newSpaceSize);
    put("newSpaceName", newSpaceName);
  }

  void deserialize_impl() {
    get("UUID", spaceID);
    get("UUIDs", spaceIDs);
    get("newSpaceSize", newSpaceSize);
    get("newSpaceName", newSpaceName);
  }
};

struct Zone : public gvds::JsonSerializer {
  // enum AuthType {
  //   NO,
  //   X,
  //   W,
  //   WX,
  //   R,
  //   RX,
  //   RW,
  //   RWX,
  // };

 public:
  std::string zoneID;                 //区域ID,UUID
  std::string zoneName;               //区域名
  std::string ownerID;                //区域主人ID，UUID
  std::vector<std::string> memberID;  //区域成员ID，UUID
  std::vector<std::string> spaceID;   //区域映射空间ID，UUID
  std::vector<std::shared_ptr<Space>> spaceBicInfo;  //空间基本信息

  // AuthType ownerType;
  // AuthType groupType;
  // AuthType otherType;
  int ownerAuth;
  int groupAuth;
  int otherAuth;

  bool contains_spaceinfo;

 public:
  void serialize_impl() override {
    put("UUID", zoneID);
    put("name", zoneName);
    put("owner", ownerID);
    put("members", memberID);
    put("spaces", spaceID);
    if (contains_spaceinfo) {
      put("spaceinfo", spaceBicInfo);
      // int ownerAuth = ownerType;
      // put("ownerAuth", ownerAuth);
      // int groupAuth = groupType;
      // put("groupAuth", groupAuth);
      // int otherAuth = otherType;
      // put("otherAuth", otherAuth);
      put("ownerAuth", ownerAuth);
      put("groupAuth", groupAuth);
      put("otherAuth", otherAuth);
    }
  }

  void deserialize_impl() override {
    get("UUID", zoneID);
    get("name", zoneName);
    get("owner", ownerID);
    get("members", memberID);
    get("spaces", spaceID);
    if (contains_spaceinfo) {
      get("spaceinfo", spaceBicInfo);
      get("ownerAuth", ownerAuth);
      get("groupAuth", groupAuth);
      get("otherAuth", otherAuth);
      // int ownerAuth;
      // get("ownerAuth", ownerAuth);
      // ownerType = static_cast<AuthType>(ownerAuth);
      // int groupAuth;
      // get("groupAuth", groupAuth);
      // groupType = static_cast<AuthType>(groupAuth);
      // int otherAuth;
      // get("otherAuth", otherAuth);
      // otherType = static_cast<AuthType>(otherAuth);
    }
  }
  Zone& operator=(const Zone& oths) {
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

struct ZoneRequest : public gvds::JsonSerializer {
 public:
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

// apply_info  数据库中结构
class struct_apply_info : public gvds::JsonSerializer {
 public:
  std::string id;
  std::string data;

 public:
  void serialize_impl() {
    put("id", id);
    put("data", data);
  }
  void deserialize_impl() {
    get("id", id);
    get("data", data);
  }

 public:
  struct_apply_info() = default;
};

// apply_info 的string vector
class struct_apply_content : public gvds::JsonSerializer {
 public:
  std::vector<std::string> applycontent;

 public:
  void serialize_impl() { put("applycontent", applycontent); }
  void deserialize_impl() { get("applycontent", applycontent); }

 public:
  struct_apply_content() = default;
};

}  // namespace gvds