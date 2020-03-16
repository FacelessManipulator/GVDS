/*
Author:Liubingqi
date:2019.03.21
*/

#ifndef USERMODELSERVER_H
#define USERMODELSERVER_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <iostream>
#include <map>


#include "datastore/couchbase_helper.h"
#include "manager/usermodel/Account.h"
#include "manager/manager.h"
#include "gvds_struct.h"
#include "config/ConfigureSettings.h"

class Account;
class SCAccount;
//using namespace Pistache;


namespace gvds{
class UserModelServer : public ManagerModule {

private:
  virtual void start() override;
  virtual void stop() override;
  virtual void router(Pistache::Rest::Router&) override;

public:
 //--------------------------------------------
    //define your function here
    
    void UserRegisterRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string UserRegister(Account &person);
    
    void UserLoginRest(const Rest::Request& request, Http::ResponseWriter response);
    bool UserLogin(std::string account, std::string pass, std::string &userID, std::string &identity);

    void getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string getUserinfo(std::string uuid , bool &is_get_success);

    void modifyUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string modifyUserinfo(Account &person);

    void exitUserAccountRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string exitUserAccount(std::string mtoken , bool &is_exit_success);

    void cancellationUserAccountRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string cancellationUserAccount(std::string uuid, bool is_cancel_success);
 //--------------------------------------------

    //获取映射的本地账户接口
    string getLocalAccountinfo(std::string ownerID, std::string hostCenterName);

    //获取一组name 对应的 id
    void getMemberIDRest(const Rest::Request& request, Http::ResponseWriter response);
    bool getMemberID(std::vector<std::string> &memberName, std::vector<std::string> &memberID);

//---------------------
    //管理员
    //管理员注册接口
    void AdminUserRegisterRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string AdminUserRegister(Account &person);

    //用户注册接口
    void bufferUserRegisterRest(const Rest::Request& request, Http::ResponseWriter response);
    int bufferUserRegister(std::string apply);

    //管理员查看 apply_info 数据库中内容
    void viewbufferListRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string viewbufferList(std::string gvdsID);

    //删除请求记录
    void removeoneofApplyInfoRest(const Rest::Request& request, Http::ResponseWriter response);

    //验证身份是否为管理员
    bool validadminidentity(std::string gvdsID);

    //管理员添加 、删除账户映射接口
    void adminCreateAccountMapping(const Rest::Request& request, Http::ResponseWriter response);
    void adminDelAccountMapping(const Rest::Request& request, Http::ResponseWriter response);

    //管理员查看指定用户 账户映射情况
    void adminSearchAccountMapping(const Rest::Request& request, Http::ResponseWriter response);
    
    //管理员查看账户池的情况  
    void adminSearchAccountPoolRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string adminSearchAccountPool(std::string adgvdsID);
    
public:
    UserModelServer() : ManagerModule("user") {
        auto _config = HvsContext::get_context()->_config;
        zonebucket = _config->get<std::string>("manager.bucket").value_or("test");
        bucket_account_info = _config->get<std::string>("manager.bucket").value_or("test");
        bucket_sc_account_info = _config->get<std::string>("manager.bucket").value_or("test");
        applybucket = _config->get<std::string>("manager.bucket").value_or("test");
        c_key = "center_information";
        adminlist = "adminwhitelist";
        user_prefix = "USER-";
        sc_user_prefix = "SCUSER-";
    };
    ~UserModelServer() {};

private:

    std::string bucket_account_info;
    std::string bucket_sc_account_info;
    std::string zonebucket;
    std::string applybucket;
    std::string c_key;
    std::string adminlist;
    std::string user_prefix;
    std::string sc_user_prefix;

};

std::string md5(std::string strPlain);
void printCookies(const Http::Request& req);
bool auth_token(const Rest::Request& request);

}// namespace gvds


#endif



