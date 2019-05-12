#pragma once

/***********************************************************
 * @file  StorageResBicInfo.h
 * @brief 存储资源基本信息类
 * @author 韦冰
 * @version 0.0.1
 * @date 2018/3/25
 * @email weibing@buaa.edu.cn
 * @license GNU General Public License (GPL)
 *
 * 修改历史：
 * ----------------------------------------------
 * 日期     | 版本号  |   作者   |      描述
 * ----------------------------------------------
 * 2018/3/25 | 0.0.1  | 韦冰   | 实现基础功能
 * ----------------------------------------------
 *
 *
 ***********************************************************/

#include <string>
#include "StorageState.h"
#include "common/JsonSerializer.h"

/// 存储资源基本信息类
/**
 * @author: 韦冰
 * @date: 2018/3/25
 *
 * 存储资源基本信息类。
 */
namespace hvs {
class StorageResBicInfo : public JsonSerializer {
 public:
  std::string storage_src_id;    // 存储资源UUID
  std::string storage_src_name;  // 存储资源名称
  std::string host_center_id;    //存储资源所在超算中心UUID
  std::string host_center_name;  // 存储资源所在超算中心名称
  int64_t total_capacity;        // 存储资源空间容量大小
  std::string mgs_address;       // 存储资源MGS地址
  StorageResState state;         // 存储资源状态

 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  StorageResBicInfo();
  StorageResBicInfo(std::string src_id, std::string src_name,
                    std::string center_id, std::string center_name,
                    int64_t capacity, std::string m_address,
                    StorageResState res_state);
  virtual ~StorageResBicInfo();
  const char* key_prefix = "STO:";
  std::string key() { return std::string(key_prefix) + storage_src_id; };
};
}  // namespace hvs