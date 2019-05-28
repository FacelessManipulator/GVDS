#pragma once
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "common/json.h"

namespace hvs {

enum StorageResState // 存储资源状态
{
    Initializing = 0,// 初始化中
    Normal,          // 正常使用
    OverLoad,        // 负载过载
    Logouting,       // 注销退出中
    Quited           // 已经退出
};

struct StorageResource : public hvs::JsonSerializer {

  std::string storage_src_id;    // 存储资源UUID
  std::string storage_src_name;  // 存储资源名称
  std::string host_center_id;    // 存储资源所在超算中心UUID
  std::string host_center_name;  // 存储资源所在超算中心名称
  int64_t total_capacity;        // 存储资源空间容量大小
  int64_t assign_capacity;       // 已经分配空间容量大小
  std::string mgs_address;       // 存储资源MGS地址
  StorageResState state;         // 存储资源状态
//  std::string ioproxy_uuid;      // 存储集群对应的IO代理的UUID

  explicit StorageResource()
  {
    
  }

  explicit StorageResource(const std::string& src_id,const std::string& src_name,
                           const std::string& center_id,const std::string& center_name,
                           const int64_t t_capacity, const int64_t a_capacity,
                           const std::string& m_address, const StorageResState res_state/*, const std::string iop_uuid*/) {
    storage_src_id = src_id;
    storage_src_name = src_name;
    host_center_id = center_id;
    host_center_name = center_name;
    total_capacity = t_capacity ;
    assign_capacity = a_capacity;
    mgs_address = m_address;
    state = res_state;
//    ioproxy_uuid =  iop_uuid;
  }

  virtual void serialize_impl() override {
    put("storage_src_id", storage_src_id);
    put("storage_src_name", storage_src_name);
    put("host_center_id", host_center_id);
    put("host_center_name", host_center_name);
    put("total_capacity", total_capacity);
    put("assign_capacity", assign_capacity);
    put("mgs_address", mgs_address);
    int _state = static_cast<int>(state);
    put("state", _state);
//    put("ioproxy_uuid", ioproxy_uuid);
  };

  virtual void deserialize_impl() override {
    get("storage_src_id", storage_src_id);
    get("storage_src_name", storage_src_name);
    get("host_center_id", host_center_id);
    get("host_center_name", host_center_name);
    get("total_capacity", total_capacity);
    get("assign_capacity", assign_capacity);
    get("mgs_address", mgs_address);
    int _state;
    get("state", _state);
    state = static_cast<StorageResState>(_state);
//    get("ioproxy_uuid", ioproxy_uuid);
  };
 
 static const std::string& prefix() {
      static std::string _prefix("STOR-");
      return _prefix;
  }

  std::string key() {
    return prefix() + storage_src_id;
  }

  std::string json_value() {
    return serialize();
  }
};

}  // namespace hvs