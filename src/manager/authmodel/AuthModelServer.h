/*
Author:Liubingqi
date:2019.03.21
*/

#ifndef AUTHODELSERVER_H
#define AUTHODELSERVER_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>


#include "datastore/couchbase_helper.h"
#include "manager/authmodel/Auth.h"
#include "manager/manager.h"
#include "manager/zone/ZoneServer.h"

#include <iostream>
#include <map>



class Account;
class SCAccount;
class Auth;
//using namespace Pistache;



namespace hvs{
class AuthModelServer : public ManagerModule {

private:
  virtual void start() override;
  virtual void stop() override;
  virtual void router(Pistache::Rest::Router&) override;

public:
 //--------------------------------------------
    //define your function here
    //Rest
    void self_AuthmemberaddRest(const Rest::Request& request, Http::ResponseWriter response);
    int self_Authmemberadd(SelfAuthSpaceInfo &auth_space);

    void self_AuthmemberdelRest(const Rest::Request& request, Http::ResponseWriter response);
    int self_Authmemberdel(SelfAuthSpaceInfo &auth_space);

    void self_AuthgroupmodifyRest(const Rest::Request& request, Http::ResponseWriter response);
    int self_Authgroupmodify(AuthModifygroupinfo &groupinfo);

    void self_AuthSPDRest(const Rest::Request& request, Http::ResponseWriter response);
    int self_AuthSPD(SelfSPD &mySPD);

    //1、权限增加模块
    //区域初始权限记录接口
    int ZonePermissionAdd(std::string zoneID, std::string ownerID);
    
    //空间权限同步接口
    int SpacePermissionSyne(std::string spaceID, std::string zoneID, std::string ownerID, std::vector<std::string> memberID);

    //TODO 副本权限同步接口
    int ReplacePermissionSyne();

    //成员权限增加接口
    int ZoneMemberAdd(std::string zoneID, std::string ownerID, std::vector<string> memberID);

    //2、权限删除模块
    //区域权限删除接口
    int ZonePermissionDeduct(std::string zoneID, std::string OwnerID);

    //成员权限删除接口
    int ZoneMemberDel(std::string zoneID, std::string ownerID, std::vector<string> memberID);

    //空间权限删除接口
    int SpacePermissionDelete(std::string spaceID, std::string zoneID);

    //3、权限修改模块
    void AuthModifyRest(const Rest::Request& request, Http::ResponseWriter response);
    int AuthModify(std::string hvsID, std::string zonename, std::string modify_groupauth);
    void transform_auth(std::string &modify_groupauth, int &pr, int &pw, int &pe);

    //4、权限查询模块
    void AuthSearchModelRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string AuthSearchModel(std::string &hvsID);
    int subAuthSearchModel(Zone &myzone, std::string hvsID, std::string &r, std::string &w, std::string &x, std::string &identity,
                           std::string &ownergroupR, std::string &ownergroupW, std::string &ownergroupE);

 //--------------------------------------------

    

    string ManagerID = *(HvsContext::get_context()->_config->get<std::string>("manager.id"));  //TODO标识所述超算，此超算是北京
    string c_key = "center_information";
public:
    AuthModelServer() : ManagerModule("auth") {
        //aaaaa = *(HvsContext::get_context()->_config->get<std::string>("manager.data_path"));
        auto _config = HvsContext::get_context()->_config;
        bucket_auth_info = _config->get<std::string>("bucket.auth_info").value_or("auth_info");
        bucket_account_info = _config->get<std::string>("bucket.account_info").value_or("account_info");
        zonebucket = _config->get<std::string>("bucket.zone_info").value_or("zone_info");
    };
    ~AuthModelServer() {};
public:
    //std::string aaaaa; // 本机存储集群路径
    std::string bucket_auth_info;
    std::string bucket_account_info;
    std::string zonebucket;
    std::string  zone_prefix = "ZONE-";
};



}// namespace hvs


#endif



