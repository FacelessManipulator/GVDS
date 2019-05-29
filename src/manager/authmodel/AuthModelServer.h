/*
Author:Liubingqi
date:2019.03.21
*/

#ifndef AUTHODELSERVER_H
#define AUTHODELSERVER_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <iostream>
#include <map>


#include "datastore/couchbase_helper.h"
#include "manager/authmodel/Auth.h"
#include "manager/manager.h"

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

    //1、权限增加模块
    //区域初始权限记录接口
    int ZonePermissionAdd(std::string zoneID, std::string ownerID);
    
    //空间权限同步接口
    int SpacePermissionSyne(std::string spaceID, std::string zoneID, std::string ownerID);

    //TODO 副本权限同步接口
    int ReplacePermissionSyne();

    //成员权限增加接口
    int ZoneMemberAdd(std::string zoneID, std::string ownerID, std::vector<string> memberID);

    //2、权限删除模块
    //区域权限删除接口
    int ZonePermissionDeduct(std::string zoneID, std::string OwnerID);

    //成员权限删除接口
    int ZoneMemberDel(std::string zoneID, std::string OwnerID, std::vector<string> memberID);

    //空间权限删除接口
    int SpacePermissionDelete(std::string spaceID);

    //3、权限修改模块

    /*
    void UserRegisterRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string UserRegister(Account &person);
    
    void UserLoginRest(const Rest::Request& request, Http::ResponseWriter response);
    bool UserLogin(std::string account, std::string pass);

    void getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string getUserinfo(std::string uuid , bool &is_get_success);

    void modifyUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string modifyUserinfo(Account &person);

    void exitUserAccountRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string exitUserAccount(std::string mtoken , bool &is_exit_success);

    void cancellationUserAccountRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string cancellationUserAccount(std::string uuid, bool is_cancel_success);
    */
 //--------------------------------------------
public:
    AuthModelServer() : ManagerModule("auth") {};
    ~AuthModelServer() {};
};



}// namespace hvs


#endif



