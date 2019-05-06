#pragma once
#ifndef ZONE_H
#define ZONE_H 

//#include <uuid/uuid.h>

#include <algorithm>
//#include "space/Space.h"
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "space/Space.h"




class Zone : public hvs::JsonSerializer {
 public:
    std::string zoneID;//区域ID,UUID
    std::string zoneName;//区域名
    std::string ownerID;//区域主人ID，UUID
    std::vector<std::string> memberID;//区域成员ID，UUID
    std::vector<std::string> spaceID;//区域映射空间ID，UUID


 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  Zone() = default;
};

class ZoneInfo : public hvs::JsonSerializer//区域信息
{
 public:
    std::string zoneID;//区域ID    
    std::string zoneName;//区域名
    std::string ownerID;//区域主人账户ID  
    std::vector<std::string> memberID;//区域成员账户ID
    SpaceInfo spaceBicInfo;//空间基本信息

 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  ZoneInfo() = default;
};

class ZoneRenameReq : public hvs::JsonSerializer//区域重命名请求
{
 public:
   std::string zoneID;//区域ID    
   std::string ownerID;//区域主人账户ID  
   std::string newZoneName;//区域名


 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  ZoneRenameReq() = default;
};

class GetZoneLocateInfoReq : public hvs::JsonSerializer//区域定位信息返回
{
 public:
   std::string clientID;
   std::string zoneID;
   std::vector<std::string> spaceID;


 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  GetZoneLocateInfoReq() = default;
};

class GetZoneLocateInfoRes : public hvs::JsonSerializer//区域定位信息返回
{
 public:
   std::vector<std::string> zoneLocateInfoResult;


 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  GetZoneLocateInfoRes() = default;
};

class GetZoneInfoRes : public hvs::JsonSerializer//区域信息返回
{
 public:
   std::vector<std::string> zoneInfoResult;


 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  GetZoneInfoRes() = default;
};

class ZoneShareReq : public hvs::JsonSerializer //区域共享请求
{
 public:
    std::string zoneID;//区域ID,UUID
    std::string ownerID;//区域主人ID，UUID
    std::vector<std::string> memberID;//区域成员ID，UUID

 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  ZoneShareReq() = default;
};

class ZoneRegisterReq : public hvs::JsonSerializer {
 public:
    std::string zoneName;//区域名
    std::string ownerID;//区域主人ID，UUID
    std::vector<std::string> memberID;//区域成员ID，UUID
    std::string spaceName;
    int64_t spaceSize;
    std::string spacePathInfo;
    std::string globalManageNodeInfo;

 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  ZoneRegisterReq() = default;
};

class ZoneCancelReq : public hvs::JsonSerializer //区域共享请求
{
 public:
    std::string zoneID;//区域ID,UUID
    std::string ownerID;//区域主人ID，UUID

 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  ZoneCancelReq() = default;
};

#endif/*ZONE_H*/
