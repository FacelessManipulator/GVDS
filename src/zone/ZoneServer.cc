#include <iostream>
#include <vector>
#include <stdio.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"

#include "zone/ZoneServer.h"

bool isSubset(std::vector<std::string> v1, std::vector<std::string> v2)
{
  int i=0,j=0;
  int m=v1.size();
  int n=v2.size();
  if(m<n)
  {		
    return 0;
  }
  sort(v1.begin(),v1.end());
  sort(v2.begin(),v2.end());
  while(i<n&&j<m)
  {
    if(v1[j]<v2[i])
    {
      j++;
    }
    else if(v1[j]==v2[i])
    {
      j++;
      i++;
    }
    else if(v1[j]>v2[i])
    {
      return 0;
    }
  }
	if(i<n)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}


namespace hvs{
ZoneServer* ZoneServer::instance = nullptr;

//区域重命名
void ZoneServer::ZoneRenameRest(const Rest::Request& request, Http::ResponseWriter response){
    std::cout << "====== start ZoneServer function: ZoneRenameRest ======"<< std::endl;
    auto info = request.body();

    ZoneRenameReq req;
    req.deserialize(info);
    std::string zoneID = req.zoneID;
    std::string ownerID = req.ownerID;
    std::string newZoneName = req.newZoneName;

    int result_i = ZoneRename(zoneID, ownerID, newZoneName);
    std::string result;
    if (result_i == 0)
    result = "success";
    else result = "fail";

    response.send(Http::Code::Ok, result); //point
    std::cout << "====== end ZoneServer function: ZoneRenameRest ======"<< std::endl;
}
int ZoneServer::ZoneRename(std::string zoneID, std::string ownerID, std::string newZoneName)
{
    Zone tmp;
    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("zone_info"));
    zonePtr->init();
    std::string tmp_key = zoneID;
    auto [vp, err] = zonePtr->get(tmp_key);
    std::string tmp_value = *vp;//待插入报错
    tmp.deserialize(tmp_value);
    if(tmp.ownerID == ownerID)
    {
      
      tmp.zoneName = newZoneName;
      tmp_value = tmp.serialize();
      zonePtr->set(tmp_key, tmp_value);
      /*
      std::string path = "name";
      std::string value = newZoneName;
      zonePtr->set(tmp_key, path, value);*/
      return 0;
    }
    else return -1;
 }

//区域定位
void ZoneServer::GetZoneLocateInfoRest(const Rest::Request& request, Http::ResponseWriter response){
  std::cout << "====== start ZoneServer function: GetZoneLocateInfoRest ======"<< std::endl;
  auto info = request.body();

  GetZoneLocateInfoReq req;
  req.deserialize(info);
  std::string clientID = req.clientID;
  std::string zoneID = req.zoneID;
  std::vector<std::string> spaceID = req.spaceID;
  std::vector<std::string> result_zl;

  bool result_b = GetZoneLocateInfo(result_zl, clientID, zoneID, spaceID);
  std::string result;
  if (result_b == true){
    GetZoneLocateInfoRes res;
    res.zoneLocateInfoResult = result_zl;
    result = res.serialize();
  }
  else{
    result = "fail";
  }
  response.send(Http::Code::Ok, result); //point
  std::cout << "====== end ZoneServer function: GetZoneLocateInfoRest ======"<< std::endl;

}

bool ZoneServer::GetZoneLocateInfo(std::vector<std::string> &result, std::string clientID, std::string zoneID, std::vector<std::string> spaceID)
{
    Zone tmp;
    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("zone_info"));
    zonePtr->init();
    std::string tmp_key = zoneID;
    auto [vz, err] = zonePtr->get(tmp_key);
    std::string tmp_value = *vz;
    tmp.deserialize(tmp_value);
    if(clientID == tmp.ownerID||std::find(tmp.memberID.begin(), tmp.memberID.end(), clientID) != tmp.memberID.end())//clientID是zone的成员或主人
    {
        if(isSubset(tmp.spaceID, spaceID))
        {
            SpaceServer* tmp_server = hvs::SpaceServer::getInstance();
            tmp_server->GetSpacePosition(result, spaceID);
            return true;
        }
        else return false;
    }
    else return false;
}

//区域信息检索
void ZoneServer::GetZoneInfoRest(const Rest::Request& request, Http::ResponseWriter response){
  std::cout << "====== start ZoneServer function: GetZoneInfoRest ======"<< std::endl;
  auto info = request.body();

  std::string clientID = info;
  std::vector<std::string> result_z;

  bool result_b = GetZoneInfo(result_z, clientID);
  std::string result;
  if (result_b == true){
    GetZoneInfoRes res;
    res.zoneInfoResult = result_z;
    result = res.serialize();
  }
  else{
    result = "fail";
  }
  response.send(Http::Code::Ok, result); //point
  std::cout << "====== end ZoneServer function: GetZoneInfoRest ======"<< std::endl;

}
bool ZoneServer::GetZoneInfo(std::vector<std::string> &result_z, std::string clientID)
{
  //查找是owner的区域
  std::string query = "select * from `zone_info` where owner = \"ownerID\";";
  int pos = query.find("ownerID");
  //query.replace(pos, 7, clientID);
  query.erase(pos, 7);
  query.insert(pos, clientID);
  std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("zone_info"));
  zonePtr->init();
  auto [vp, err] = zonePtr->n1ql(query);

  //查找是member的区域
  std::string query2 = "select * from `zone_info` where \"clientID\" within members;";
  int pos2 = query2.find("clientID");
  query2.erase(pos2, 8);
  query2.insert(pos2, clientID);
  //query2.replace(pos2, 8, clientID);
  std::shared_ptr<hvs::CouchbaseDatastore> zonePtr2 = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("zone_info"));
  zonePtr2->init();
  auto [vp2, err2] = zonePtr2->n1ql(query2);

  if(vp->size()==0 && vp2->size()==0) return false;
  else
  {
    for (std::vector<std::string>::iterator it = vp->begin(); it != vp->end(); it++)  
    {
      Zone tmp;
      ZoneInfo tmp_zi;
      std::string n1ql_result = *it;
      std::string tmp_value = n1ql_result.substr(13, n1ql_result.length() - 14);
      tmp.deserialize(tmp_value);
      tmp_zi.zoneID = tmp.zoneID;
      tmp_zi.zoneName = tmp.zoneName;
      tmp_zi.ownerID = tmp.ownerID;
      tmp_zi.memberID = tmp.memberID;

      std::string result_s;
      SpaceServer* tmp_server = hvs::SpaceServer::getInstance();
      tmp_server->GetSpaceInfo(result_s, tmp.spaceID);
      tmp_zi.spaceBicInfo.deserialize(result_s);
      std::string zi_result = tmp_zi.serialize();
      result_z.emplace_back(zi_result);
    }



    for (std::vector<std::string>::iterator it2 = vp2->begin(); it2 != vp2->end(); it2++)
    {
      Zone tmp2;
      ZoneInfo tmp_zi2;
      std::string n1ql_result2 = *it2;
      std::string tmp_value2 = n1ql_result2.substr(13, n1ql_result2.length() - 14);
      tmp2.deserialize(tmp_value2);
      tmp_zi2.zoneID = tmp2.zoneID;
      tmp_zi2.zoneName = tmp2.zoneName;
      tmp_zi2.ownerID = tmp2.ownerID;
      tmp_zi2.memberID = tmp2.memberID;

      std::string result_s2;
      SpaceServer* tmp_server = hvs::SpaceServer::getInstance();
      tmp_server->GetSpaceInfo(result_s2, tmp2.spaceID);
      tmp_zi2.spaceBicInfo.deserialize(result_s2);
      std::string zi_result2 = tmp_zi2.serialize();
      result_z.emplace_back(zi_result2);
    }
    return true;
  }
}


//区域共享
void ZoneServer::ZoneShareRest(const Rest::Request& request, Http::ResponseWriter response){
  std::cout << "====== start ZoneServer function: ZoneShareRest ======"<< std::endl;
  auto info = request.body();
  ZoneShareReq req;
  req.deserialize(info);
  std::string zoneID = req.zoneID;
  std::string ownerID = req.ownerID;
  std::vector<std::string> memberID = req.memberID;

  int result_i = ZoneShare(zoneID, ownerID, memberID);
  std::string result;
  if (result_i == 0)
  result = "success";
  else result = "fail";

  response.send(Http::Code::Ok, result); //point
  std::cout << "====== end ZoneServer function: ZoneShareRest ======"<< std::endl;
}
int ZoneServer::ZoneShare(std::string zoneID, std::string ownerID, std::vector<std::string> memberID)
{
  Zone tmp;
  std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
  zonePtr->init();
  std::string tmp_key = zoneID;
  auto [vp, err] = zonePtr->get(tmp_key);
  std::string tmp_value = *vp;
  tmp.deserialize(tmp_value);
  if(tmp.ownerID == ownerID)
  {
    //插入调用lbq模块
    for(std::vector<std::string>::iterator it = memberID.begin(); it != memberID.end(); it++)
    {
      std::string tmp_mem = *it;
      if(std::find(tmp.memberID.begin(), tmp.memberID.end(), tmp_mem) != tmp.memberID.end()) continue;
      else
        tmp.memberID.emplace_back(tmp_mem);
    }
    tmp_value = tmp.serialize();
    zonePtr->set(tmp_key, tmp_value);
    return 0;
  }
  else return -1;
}


void ZoneServer::ZoneShareCancelRest(const Rest::Request& request, Http::ResponseWriter response){
  std::cout << "====== start ZoneServer function: ZoneShareCancelRest ======"<< std::endl;
  auto info = request.body();
  ZoneShareReq req;
  req.deserialize(info);
  std::string zoneID = req.zoneID;
  std::string ownerID = req.ownerID;
  std::vector<std::string> memberID = req.memberID;

  int result_i = ZoneShareCancel(zoneID, ownerID, memberID);
  std::string result;
  if (result_i == 0)
  result = "success";
  else result = "fail";

  response.send(Http::Code::Ok, result); //point
  std::cout << "====== end ZoneServer function: ZoneShareCancelRest ======"<< std::endl;
}
int ZoneServer::ZoneShareCancel(std::string zoneID, std::string ownerID, std::vector<std::string> memberID)
{
  Zone tmp;
  std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
  zonePtr->init();
  std::string tmp_key = zoneID;
  auto [vp, err] = zonePtr->get(tmp_key);
  std::string tmp_value = *vp;
  tmp.deserialize(tmp_value);
  if(tmp.ownerID == ownerID)
  {
    //插入调用lbq模块
    for(std::vector<std::string>::iterator it = memberID.begin(); it != memberID.end(); it++)
    {
      std::string tmp_mem = *it;
      std::vector<std::string>::iterator m = tmp.memberID.begin();
      while(m != tmp.memberID.end())
      {
        if(*m == tmp_mem)
        {
          m = tmp.memberID.erase(m);
        }
        else
        {
          ++m;
        }
      }
    }
    tmp_value = tmp.serialize();
    zonePtr->set(tmp_key, tmp_value);
    return 0;
  }
  else return -1;
}


void ZoneServer::ZoneRegisterRest(const Rest::Request& request, Http::ResponseWriter response){
  std::cout << "====== start ZoneServer function: ZoneRegisterRest ======"<< std::endl;
  auto info = request.body();
  ZoneRegisterReq req;
  req.deserialize(info);
  std::string zoneName = req.zoneName;
  std::string ownerID = req.ownerID;
  std::vector<std::string> memberID = req.memberID;
  std::string spaceName = req.spaceName;
  int64_t spaceSize = req.spaceSize;
  std::string spacePathInfo = req.spacePathInfo;//spacemetadata类，在客户端序列化为string。
  //std::string globalManageNodeInfo =req.globalManageNodeInfo;客户端判断

  int result_i = ZoneRegister(zoneName, ownerID, memberID, spaceName, spaceSize, spacePathInfo);
  std::string result;
  if (result_i == 0)
  result = "success";
  else result = "fail";

  response.send(Http::Code::Ok, result); //point
  std::cout << "====== end ZoneServer function: ZoneRegisterRest ======"<< std::endl;
}
int ZoneServer::ZoneRegister(std::string zoneName, std::string ownerID, std::vector<std::string> memberID,
    std::string spaceName, int64_t spaceSize, std::string spacePathInfo)
{
  Zone tmp;
  std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
  zonePtr->init();
  SpaceMetaData tmp_smd;
  tmp_smd.deserialize(spacePathInfo);
  
  // if(tmp_smd.hostCenterID.empty())
  // {
  // }
  // else
  // {
  // }在客户端判断

  //1、调用spacecreate接口（涉及到跨域创建空间的情况，则返还客户端，并再次发送）
  //跨域空间创建情况，考虑采用各超算管各自的创建，本超算不成功则返回客户端发送请求到下一顺位
  //2、调用权限模块
  SpaceServer* tmp_server = hvs::SpaceServer::getInstance();
  std::string res_sc = tmp_server->SpaceCreate(spaceName, ownerID, memberID, spaceSize, spacePathInfo);
  if (res_sc == "false")
  {
    return -1;
  }
  else
  {
    std::string spaceID = res_sc;
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    const std::string tmp_uuid = oost::uuids::to_string(a_uuid);
    tmp.zoneID = tmp_uuid;
    tmp.zoneName = zoneName;
    tmp.ownerID = ownerID;
    tmp.memberID = memberID;
    tmp.spaceID.emplace_back(spaceID);

    std::string tmp_key(tmp.zoneID);
    std::string tmp_value = tmp.serialize();
    zonePtr->set(tmp_key, tmp_value);
    return 0;
  }
}


void ZoneServer::ZoneCancelRest(const Rest::Request& request, Http::ResponseWriter response){
  std::cout << "====== start ZoneServer function: ZoneCancelRest ======"<< std::endl;
  auto info = request.body();
  ZoneCancelReq req;
  std::cout << info << std::endl;
  req.deserialize(info);
  std::string zoneID = req.zoneID;
  std::string ownerID = req.ownerID;

  int result_i = ZoneCancel(zoneID, ownerID);
  std::string result;
  if (result_i == 0)
  result = "success";
  else result = "fail";

  response.send(Http::Code::Ok, result); //point
  std::cout << "====== end ZoneServer function: ZoneCancelRest ======"<< std::endl;
}
int ZoneServer::ZoneCancel(std::string zoneID, std::string ownerID)
{
  Zone tmp;
  std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
  zonePtr->init();
  std::string tmp_key = zoneID;
  auto [vp, err] = zonePtr->get(tmp_key);
  std::string tmp_value = *vp;
  std::cout << tmp_value << std::endl;
  tmp.deserialize(tmp_value);
  std::cout << tmp.ownerID << std::endl;
  if(tmp.ownerID == ownerID)
  {
    //插入lbq权限删除
    SpaceServer* tmp_server = hvs::SpaceServer::getInstance();
    if(tmp_server->SpaceDelete(tmp.spaceID) == 0)
    {
      zonePtr->remove(tmp_key);
      return 0;
    }
    else return -1;
  }
  else return -1;
}

}//namespace hvs
