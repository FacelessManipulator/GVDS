#include <iostream>
#include <vector>
#include <stdio.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"

#include "manager/zone/ZoneServer.h"
#include "manager/authmodel/AuthModelServer.h"

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
  using namespace Pistache::Rest;
  using namespace Pistache::Http;

  void ZoneServer::start() {}

  void ZoneServer::stop() {}

  void ZoneServer::router(Router& router) {
    Routes::Post(router, "/zone/rename", Routes::bind(&ZoneServer::ZoneRenameRest, this));
    Routes::Post(router, "/zone/locate", Routes::bind(&ZoneServer::GetZoneLocateInfoRest, this));
    Routes::Post(router, "/zone/info", Routes::bind(&ZoneServer::GetZoneInfoRest, this));
    Routes::Post(router, "/zone/share", Routes::bind(&ZoneServer::ZoneShareRest, this));
    Routes::Post(router, "/zone/sharecancel", Routes::bind(&ZoneServer::ZoneShareCancelRest, this));
    Routes::Post(router, "/zone/register", Routes::bind(&ZoneServer::ZoneRegisterRest, this));
    Routes::Post(router, "/zone/cancel", Routes::bind(&ZoneServer::ZoneCancelRest, this));
    Routes::Post(router, "/zone/mapadd", Routes::bind(&ZoneServer::MapAddRest, this));
    Routes::Post(router, "/zone/mapdeduct", Routes::bind(&ZoneServer::MapDeductRest, this));
    Routes::Post(router, "/zone/add", Routes::bind(&ZoneServer::ZoneAddRest, this));
  }

  std::string zonejsonfilter(const std::string &serialize_json){
    return serialize_json.substr(0,serialize_json.find_last_of("}")+1);
  }

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
      else
          result = "fail";

      response.send(Http::Code::Ok, result);
      std::cout << "====== end ZoneServer function: ZoneRenameRest ======"<< std::endl;
  }

  int ZoneServer::ZoneRename(std::string zoneID, std::string ownerID, std::string newZoneName)
  {
      Zone tmp;
      std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
            hvs::CouchbaseDatastore(zonebucket));
      zonePtr->init();
      auto [vp, err] = zonePtr->get(zoneID);
      std::string tmp_value = *vp;
      tmp.deserialize(zonejsonfilter(tmp_value));
      if(tmp.ownerID == ownerID)
      {
        tmp.zoneName = std::move(newZoneName);
        if(zonePtr->set(zoneID, tmp.serialize()) != 0) return -1;//插入报错
        else return 0;
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
    response.send(Http::Code::Ok, result);
    std::cout << "====== end ZoneServer function: GetZoneLocateInfoRest ======"<< std::endl;
  }

  bool ZoneServer::GetZoneLocateInfo(std::vector<std::string> &result, std::string clientID, std::string zoneID, std::vector<std::string> spaceID)
  {
      Zone tmp;
      std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
            hvs::CouchbaseDatastore(zonebucket));
      zonePtr->init();
      auto [vz, err] = zonePtr->get(zoneID);
      std::string tmp_value = *vz;
      tmp.deserialize(tmp_value);
      if(clientID == tmp.ownerID||std::find(tmp.memberID.begin(), tmp.memberID.end(), clientID) != tmp.memberID.end())//clientID是zone的成员或主人
      {
          if(isSubset(tmp.spaceID, spaceID))
          {
              SpaceServer* tmp_server = static_cast<SpaceServer*>(mgr->get_module("space").get());
              tmp_server->GetSpacePosition(result, spaceID);
              return true;
          }
          else return false;
      }
      else return false;
  }

  //区域信息检索
  void ZoneServer::GetZoneInfoRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(15) << "INFO: ZoneServer: GetZoneInfoRest request."<< dendl;
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
    response.send(Http::Code::Ok, result);
  }

  bool ZoneServer::GetZoneInfo(std::vector<std::string> &result_z, std::string clientID)
  {
    //查找是owner的区域
    std::string query = "select * from `"+zonebucket+"` where owner = \"ownerID\";";
    int pos = query.find("ownerID");
    query.erase(pos, 7);
    query.insert(pos, clientID);
    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
            hvs::CouchbaseDatastore(zonebucket));
    zonePtr->init();
    auto [vp, err] = zonePtr->n1ql(query);

    //查找是member的区域
    std::string query2 = "select * from `"+zonebucket+"` where \"clientID\" within members;";
    int pos2 = query2.find("clientID");
    query2.erase(pos2, 8);
    query2.insert(pos2, clientID);
    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr2 = std::make_shared<hvs::CouchbaseDatastore>(
            hvs::CouchbaseDatastore(zonebucket));
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
        SpaceServer* tmp_server = dynamic_cast<SpaceServer*>(mgr->get_module("space").get());
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
        SpaceServer* tmp_server = dynamic_cast<SpaceServer*>(mgr->get_module("space").get());
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

    response.send(Http::Code::Ok, result);
    std::cout << "====== end ZoneServer function: ZoneShareRest ======"<< std::endl;
  }

  int ZoneServer::ZoneShare(std::string zoneID, std::string ownerID, std::vector<std::string> memberID)
  {
    Zone tmp;
    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore(zonebucket));
    zonePtr->init();
    auto [vp, err] = zonePtr->get(zoneID);
    std::string tmp_value = *vp;
    tmp.deserialize(tmp_value);
    if(tmp.ownerID == ownerID)
    {
      //TODO:插入调用lbq模块
      AuthModelServer *p_auth = static_cast<AuthModelServer*>(mgr->get_module("auth").get());
      int memadd = p_auth->ZoneMemberAdd(zoneID, ownerID, memberID);
      if(memadd == 0)
      {
        for(std::vector<std::string>::iterator it = memberID.begin(); it != memberID.end(); it++)
        {
          std::string tmp_mem = *it;
          if(std::find(tmp.memberID.begin(), tmp.memberID.end(), tmp_mem) != tmp.memberID.end()) continue;
          else
            tmp.memberID.emplace_back(tmp_mem);
        }
        tmp_value = tmp.serialize();
        zonePtr->set(zoneID, tmp_value);
        return 0;
      }
      else return -1;
    }
    else
      return -1;
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

    response.send(Http::Code::Ok, result);
    std::cout << "====== end ZoneServer function: ZoneShareCancelRest ======"<< std::endl;
  }

  int ZoneServer::ZoneShareCancel(std::string zoneID, std::string ownerID, std::vector<std::string> memberID)
  {
    Zone tmp;
    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore(zonebucket));
    zonePtr->init();
    auto [vp, err] = zonePtr->get(zoneID);
    if(err != 0){
        std::cerr << "ZoneShareCancel:未找到对应的空间！" << std::endl;
        return -1;
    }
    std::string tmp_value = *vp;
    tmp.deserialize(tmp_value);
    if(!isSubset(tmp.memberID, memberID)){
        return -1;
    }
    if(tmp.ownerID == ownerID)
    {
      //插入调用lbq模块
      AuthModelServer *p_auth = static_cast<AuthModelServer*>(mgr->get_module("auth").get());
      int memdel = p_auth->ZoneMemberDel(zoneID, ownerID, memberID);
      if(memdel == 0)
      {
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
        zonePtr->set(zoneID, tmp_value);
        return 0;
      }
      else return -1;
    }
    else
        return -1;
  }

  //区域注册
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

    response.send(Http::Code::Ok, result);
    std::cout << "====== end ZoneServer function: ZoneRegisterRest ======"<< std::endl;
  }

  int ZoneServer::ZoneRegister(std::string zoneName, std::string ownerID, std::vector<std::string> memberID,
      std::string spaceName, int64_t spaceSize, std::string spacePathInfo)
  {

    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore(zonebucket));
    zonePtr->init();
    //插入判断，加的这个区域是否已经存在
    std::string query = "select * from `"+zonebucket+"` where owner = \"ownerID\" and name = \"zonename\";";
    int pos = query.find("ownerID");
    query.erase(pos, 7);
    query.insert(pos, ownerID);
    int pos2 = query.find("zonename");
    query.erase(pos2, 8);
    query.insert(pos2, zoneName);
    auto [vp, err] = zonePtr->n1ql(query);
    if(vp->size() != 0) return -1;
    else{
      //1、TODO: 调用spacecreate接口（涉及到跨域创建空间的情况，则返还客户端，并再次发送） , 目前在区域初始注册的时候，只能创建一个默认的空间
      //跨域空间创建情况，考虑采用各超算管各自的创建，本超算不成功则返回客户端发送请求到下一顺位
      //2、TODO：调用权限模块
      SpaceServer* tmp_server = dynamic_cast<SpaceServer*>(mgr->get_module("space").get());//获取空间服务端模块
      std::string res_sc = tmp_server->SpaceCreate(std::move(spaceName), ownerID, memberID, spaceSize, std::move(spacePathInfo));
      if (res_sc == "false")
      {
        return -1;
      }
      else
      {
        Zone tmp;
        std::string spaceID = res_sc;
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        const std::string tmp_uuid = boost::uuids::to_string(a_uuid);
        tmp.zoneID = tmp_uuid;
        tmp.zoneName = std::move(zoneName);
        tmp.ownerID = ownerID;
        tmp.memberID = memberID;
        tmp.spaceID.emplace_back(spaceID);

        zonePtr->set(tmp.zoneID, tmp.serialize());

        AuthModelServer *p_auth = static_cast<AuthModelServer*>(mgr->get_module("auth").get());
        int res_za = p_auth->ZonePermissionAdd(tmp.zoneID, tmp.ownerID);
        if(res_za == 0)
        {
          if(tmp.memberID.empty())
          {
            return 0;
          }
          else
          {            
            int memadd = p_auth->ZoneMemberAdd(tmp.zoneID, tmp.ownerID, tmp.memberID);
            if(memadd == 0) return 0;
            else
            {
              std::cerr << "ZoneRegister:添加成员失败！" << std::endl;
              return -1;
            } 
          }
        }
        else
        {
          std::cerr << "ZoneRegister:添加初始权限失败！" << std::endl;
          return -1;
        }
      }
    }
  }

  //管理员区域添加
  void ZoneServer::ZoneAddRest(const Rest::Request& request, Http::ResponseWriter response){
    std::cout << "====== start ZoneServer function: ZoneAddRest ======"<< std::endl;
    auto info = request.body();
    ZoneRegisterReq req;
    req.deserialize(info);
    std::string zoneName = req.zoneName;
    std::string ownerID = req.ownerID;
    std::vector<std::string> memberID = req.memberID;
    std::string spacePathInfo = req.spacePathInfo;//spacemetadata类，在客户端序列化为string。
    //std::string globalManageNodeInfo =req.globalManageNodeInfo;客户端判断

    int result_i = ZoneAdd(zoneName, ownerID, memberID, spacePathInfo);
    std::string result;
    if (result_i == 0)
    result = "success";
    else result = "fail";

    response.send(Http::Code::Ok, result); //point
    std::cout << "====== end ZoneServer function: ZoneAddRest ======"<< std::endl;
  }

  int ZoneServer::ZoneAdd(std::string zoneName, std::string ownerID, std::vector<std::string> memberID,
      std::string spacePathInfo)
  {
    Zone tmp;
    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore(zonebucket));
    zonePtr->init();
    //插入判断，加的这个区域是否已经存在
    std::string query = "select * from `"+zonebucket+"` where owner = \"ownerID\" and name = \"zonename\";";
    int pos = query.find("ownerID");
    query.erase(pos, 7);
    query.insert(pos, ownerID);
    int pos2 = query.find("zonename");
    query.erase(pos2, 8);
    query.insert(pos2, zoneName);
    auto [vp, err] = zonePtr->n1ql(query);

    if(vp->size() == 0)
    {
      SpaceServer* tmp_server = dynamic_cast<SpaceServer*>(mgr->get_module("space").get());//获取空间服务端
      std::string res_sc = tmp_server->SpaceCheck(ownerID, memberID, std::move(spacePathInfo));
      if (res_sc == "false")
      {
        return -1;
      }
      else
      {
        std::string spaceID = res_sc;
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        const std::string tmp_uuid = boost::uuids::to_string(a_uuid);
        tmp.zoneID = tmp_uuid;
        tmp.zoneName = std::move(zoneName);
        tmp.ownerID = ownerID;
        tmp.memberID = memberID;
        tmp.spaceID.emplace_back(spaceID);

        zonePtr->set(tmp.zoneID, tmp.serialize());
        AuthModelServer *p_auth = static_cast<AuthModelServer*>(mgr->get_module("auth").get());
        int res_za = p_auth->ZonePermissionAdd(tmp.zoneID, tmp.ownerID);
        if(res_za == 0)
        {
          //空间权限同步
          int spacesyne = p_auth->SpacePermissionSyne(spaceID, tmp.zoneID, ownerID);
          if(spacesyne == 0)
          {
            if(tmp.memberID.empty())
            {
              return 0;
            }
            else
            {            
              int memadd = p_auth->ZoneMemberAdd(tmp.zoneID, tmp.ownerID, tmp.memberID);
              if(memadd == 0) return 0;
              else
              {
                std::cerr << "ZoneAdd:添加成员失败！" << std::endl;
                return -1;
              } 
            }
          }
          else return -1;
        }
        else
        {
          std::cerr << "ZoneAdd:添加初始权限失败！" << std::endl;
          return -1;
        }
      }
    }
    else if(vp->size() == 1)
    {
      SpaceServer* tmp_server = dynamic_cast<SpaceServer*>(mgr->get_module("space").get());//获取空间服务端
      std::string res_sc = tmp_server->SpaceCheck(ownerID, memberID, std::move(spacePathInfo));
      if (res_sc == "false")
      {
        return -1;
      }
      else
      {
        std::string spaceID = res_sc;
        std::vector<std::string>::iterator it = vp->begin();
        std::string n1ql_result = *it;
        std::string tmp_value = n1ql_result.substr(13, n1ql_result.length() - 14);
        tmp.deserialize(tmp_value);
        tmp.spaceID.emplace_back(spaceID);
        zonePtr->set(tmp.zoneID, tmp.serialize());
        AuthModelServer *p_auth = static_cast<AuthModelServer*>(mgr->get_module("auth").get());
        int spacesyne = p_auth->SpacePermissionSyne(spaceID, tmp.zoneID, ownerID);
        if(spacesyne == 0) return 0;
        else
        {
          std::cerr << "ZoneAdd:空间权限同步失败！" << std::endl;
          return -1;
        }
      }
    }
    else return -1;
  }


  //区域注销
  void ZoneServer::ZoneCancelRest(const Rest::Request& request, Http::ResponseWriter response){
    std::cout << "====== start ZoneServer function: ZoneCancelRest ======"<< std::endl;
    auto info = request.body();
    ZoneCancelReq req;
    req.deserialize(info);
    std::string zoneID = req.zoneID;
    std::string ownerID = req.ownerID;

    int result_i = ZoneCancel(zoneID, ownerID);
    std::string result;
    if (result_i == 0)
      result = "success";
    else
      result = "fail";

    response.send(Http::Code::Ok, result);
    std::cout << "====== end ZoneServer function: ZoneCancelRest ======"<< std::endl;
  }

  int ZoneServer::ZoneCancel(std::string zoneID, std::string ownerID)
  {
    Zone tmp;
    std::shared_ptr<hvs::Datastore> zonePtr =hvs::DatastoreFactory::create_datastore(zonebucket, hvs::DatastoreType::couchbase);
    auto [vp, err] = zonePtr->get(zoneID);
    if( err != 0 ){
      return -1;
    }
    std::string tmp_value = *vp;
    tmp.deserialize(zonejsonfilter(tmp_value));
    if(tmp.ownerID == ownerID)
    {
      //TODO：插入lbq权限删除
      AuthModelServer *p_auth = static_cast<AuthModelServer*>(mgr->get_module("auth").get());
      int res_zd = p_auth->ZonePermissionDeduct(zoneID, ownerID);
      if(res_zd == 0)
      {
        SpaceServer* tmp_server = dynamic_cast<SpaceServer*>(mgr->get_module("space").get());
        if(tmp_server->SpaceDelete(tmp.spaceID) == 0)
        {
          zonePtr->remove(zoneID);
          return 0;
        }
        else return -1;
      }
      else return -1;
    }
    else return -1;
  }

  //区域映射编辑
  void ZoneServer::MapAddRest(const Rest::Request& request, Http::ResponseWriter response){
    std::cout << "====== start ZoneServer function: MapAddRest ======"<< std::endl;
    auto info = request.body();
    MapAddReq req;
    req.deserialize(info);
    std::string zoneID = req.zoneID;
    std::string ownerID = req.ownerID;
    std::string spaceName = req.spaceName;
    int64_t spaceSize = req.spaceSize;
    std::string spacePathInfo = req.spacePathInfo;//spacemetadata类，在客户端序列化为string。
    //std::string globalManageNodeInfo =req.globalManageNodeInfo;客户端判断

    int result_i = MapAdd(zoneID, ownerID, spaceName, spaceSize, spacePathInfo);
    std::string result;
    if (result_i == 0)
    result = "success";
    else result = "fail";

    response.send(Http::Code::Ok, result); //point
    std::cout << "====== end ZoneServer function: MapAddRest ======"<< std::endl;
  }

  int ZoneServer::MapAdd(std::string zoneID, std::string ownerID, std::string spaceName, int64_t spaceSize, std::string spacePathInfo)
  {
    Zone tmp;
    std::shared_ptr<hvs::Datastore> zonePtr =hvs::DatastoreFactory::create_datastore(zonebucket, hvs::DatastoreType::couchbase);
    std::string tmp_key = zoneID;
    auto [vp, err] = zonePtr->get(tmp_key);
    std::string tmp_value = *vp;
    tmp.deserialize(tmp_value);
    if(tmp.ownerID == ownerID)
    {
      std::vector<std::string> memberID = tmp.memberID;
      SpaceServer* tmp_server = static_cast<SpaceServer*>(mgr->get_module("space").get());//调用方法
      //SpaceServer* tmp_server = hvs::SpaceServer::getInstance();
      std::string res_sc = tmp_server->SpaceCreate(spaceName, ownerID, memberID, spaceSize, spacePathInfo);
      if (res_sc == "false")
      {
        return -1;
      }
      else
      {
        //得到返回的空间ID，并加入到区域的包含的空间ID的向量中；
        std::string spaceID = res_sc;
        tmp.spaceID.emplace_back(spaceID);
        zonePtr->set(tmp_key, tmp.serialize());
        AuthModelServer *p_auth = static_cast<AuthModelServer*>(mgr->get_module("auth").get());
        int spacesyne = p_auth->SpacePermissionSyne(spaceID, zoneID, ownerID);
        if(spacesyne == 0) return 0;
        else
        {
          std::cerr << "MapAdd:空间权限同步失败！" << std::endl;
          return -1;
        }
      }
    }
  }

  void ZoneServer::MapDeductRest(const Rest::Request& request, Http::ResponseWriter response){
    std::cout << "====== start ZoneServer function: MapDeductRest ======"<< std::endl;
    auto info = request.body();
    MapDeductReq req;
    req.deserialize(info);
    std::string zoneID = req.zoneID;
    std::string ownerID = req.ownerID;
    std::vector<std::string> spaceID = req.spaceID;

    int result_i = MapDeduct(zoneID, ownerID, spaceID);
    std::string result;
    if (result_i == 0)
    result = "success";
    else result = "fail";

    response.send(Http::Code::Ok, result);
    std::cout << "====== end ZoneServer function: MapDeductRest ======"<< std::endl;
  }

  int ZoneServer::MapDeduct(std::string zoneID, std::string ownerID, std::vector<std::string> spaceID)
  {
    Zone tmp;
    std::shared_ptr<hvs::Datastore> zonePtr =hvs::DatastoreFactory::create_datastore(zonebucket, hvs::DatastoreType::couchbase);
    auto [vp, err] = zonePtr->get(zoneID);
    std::string tmp_value = *vp;
    tmp.deserialize(zonejsonfilter(tmp_value));
    if(tmp.ownerID == ownerID)
    {
      if(isSubset(tmp.spaceID, spaceID))
      {
        int spaceauthfault = 0;
        for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
        {
          AuthModelServer *p_auth = static_cast<AuthModelServer*>(mgr->get_module("auth").get());
          int spaceauthdel = p_auth->SpacePermissionDelete(*m);
          if(spaceauthdel == 0) continue;
          else
          {
            std::cerr << "MapDeduct:空间权限删除失败！" << std::endl;
            spaceauthfault = 1;
            break;
          }

        }
        if(spaceauthfault == 0)
        {
          SpaceServer* tmp_server = dynamic_cast<SpaceServer*>(mgr->get_module("space").get());
          tmp_server->SpaceDelete(spaceID); // 调用空间模块，删除空间条目
          for(std::vector<std::string>::iterator it = spaceID.begin(); it != spaceID.end(); it++)
          {
            std::string will_removed_spaceID = *it;
            std::vector<std::string>::iterator m = tmp.spaceID.begin();
            while(m != tmp.spaceID.end())
            {
              if(*m == will_removed_spaceID)
              {
                m = tmp.spaceID.erase(m);
              }
              else
              {
                ++m;
              }
            }
          }
          zonePtr->set(zoneID, tmp.serialize());
          return 0;
        }
        else return -1;
      }
      else return -1;
    }
    else return -1;
  }

}//namespace hvs
