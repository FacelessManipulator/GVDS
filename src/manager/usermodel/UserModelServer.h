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
    string getLocalAccountinfo(string ownerID, string hostCenterName);

public:
    UserModelServer() : ManagerModule("user") {};
    ~UserModelServer() {};

private:
    bool addSCaccount();
    bool SubBuildAccountMapping(SCAccount &person, std::string location, std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr);
    bool BuildAccountMapping(std::string accountID);

    bool RemoveAccountMapping(std::string accountID);
    bool SubRemoveAccountMapping(SCAccount &person, std::string location, std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr);

private:
    string supercomputing_A = "zhongkeyuan"; //"Beijing";
    string supercomputing_B = "beihang"; //"Shanghai";
    string supercomputing_C = "Guangzhou";
    string supercomputing_D = "Changsha";
    string supercomputing_E = "Jinan";

};

std::string md5(std::string strPlain);
void printCookies(const Http::Request& req);
bool auth_token(const Rest::Request& request);

}// namespace hvs


#endif



