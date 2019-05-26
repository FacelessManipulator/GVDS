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
    int ZonePermissionAdd(std::string zoneID, std::string ownerID);

    int SpacePermissionSyne(std::string spaceID, std::string zoneID, std::string ownerID);
    
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



