#pragma once
#ifndef SPACE_H
#define SPACE_H 

//#include <uuid/uuid.h>
#include "common/JsonSerializer.h"
//#include "datastore/datastore.h"
//#include "context.h"
#include "common/json.h"
//#include "datastore/couchbase_helper.h"

class Space : public hvs::JsonSerializer {
 public:
    std::string spaceID;//空间ID,UUID
    std::string spaceName;//空间名
    int64_t spaceSize;//空间容量
    std::string hostCenterID;//存储资源所在超算中心UUID
    std::string storageSrcID;// 存储资源UUID
    std::string spacePath;// 空间所在存储集群的路径
    bool status;//空间可用状态

 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  Space() = default;
};

class SpaceInfo : public hvs::JsonSerializer//空间基本信息,考虑要不要map
{
 public:
    std::vector<std::string> spaceID;//空间ID
    std::map<std::string, std::string> spaceName;//空间名
    std::map<std::string, int64_t> spaceSize;//空间容量

 public:
  void serialize_impl() override;
  void deserialize_impl() override;
  
 public:
  SpaceInfo() = default;
};

class SpaceMetaData : public hvs::JsonSerializer//空间元数据信息
{
 public:
    std::string spaceID;//空间ID
    std::string spaceName;//空间名
    std::string storageSrcID;// 存储资源UUID
    std::string storageSrcName;// 存储资源名称
    std::string hostCenterID; //存储资源所在超算中心UUID
    std::string hostCenterName;// 存储资源所在超算中心名称
    std::string spacePath;// 空间所在存储集群的路径

 public:
  void serialize_impl() override; 
  void deserialize_impl() override;

 public:
  SpaceMetaData() = default;
};

class SpaceRenameReq : public hvs::JsonSerializer//区域重命名请求
{
 public:
   std::string spaceID;//区域ID    
   std::string newSpaceName;//区域名


 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  SpaceRenameReq() = default;
};

class SpaceSizeChangeReq : public hvs::JsonSerializer//空间缩放请求
{
public:
    std::string     spaceID;//空间ID
    std::string     spaceName;//空间名
    int64_t         newSpaceSize;//空间容量

public:
    void serialize_impl() override;
    void deserialize_impl() override;

public:
    SpaceSizeChangeReq() = default;
};
#endif/*SPACE_H*/
