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
#include "hvs_struct.h"
#include "config/ConfigureSettings.h"

class Account;
class SCAccount;
//using namespace Pistache;


namespace hvs{
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
    bool UserLogin(std::string account, std::string pass, std::string &userID);

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

public:
    UserModelServer() : ManagerModule("user") {
        auto _config = HvsContext::get_context()->_config;
        zonebucket = _config->get<std::string>("bucket.zone_info").value_or("zone_info");
        bucket_account_info = _config->get<std::string>("bucket.account_info").value_or("account_info");
        bucket_sc_account_info = _config->get<std::string>("bucket.sc_account_info").value_or("sc_account_info");
        c_key = "center_information";
    };
    ~UserModelServer() {};

private:
    bool addSCaccount();
    bool BuildAccountMapping_v2(std::string accountID);
    bool SubBuildAccountMapping_v2(SCAccount &person, std::string location, std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr);

    bool RemoveAccountMapping_v2(std::string accountID);
    bool SubRemoveAccountMapping_v2(SCAccount &person, std::string location, std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr);

    //不使用了////
    // bool SubBuildAccountMapping_old(SCAccount &person, std::string location, std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr);
    // bool BuildAccountMapping_old(std::string accountID);

    // bool RemoveAccountMapping_old(std::string accountID);
    // bool SubRemoveAccountMapping_old(SCAccount &person, std::string location, std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr);

    //检测本地是否存在账户，否则建立账户映射失败
    bool existlocalaccount(std::string valid);

private:


    string bucket_account_info;
    string bucket_sc_account_info;
    std::string  zonebucket;
    string c_key;

};

std::string md5(std::string strPlain);
void printCookies(const Http::Request& req);
bool auth_token(const Rest::Request& request);

}// namespace hvs


#endif



