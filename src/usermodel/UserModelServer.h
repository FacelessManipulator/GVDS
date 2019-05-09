/*
Author:Liubingqi
date:2019.03.21
*/

#ifndef USERMODELSERVER_H
#define USERMODELSERVER_H

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <iostream>
#include <map>


#include "datastore/couchbase_helper.h"
#include "usermodel/Account.h"
class Account;

using namespace Pistache;


namespace hvs{
class UserModelServer {
public:
    static UserModelServer* getInstance(){
        if (instance == nullptr)
            instance = new UserModelServer();
	    return instance;
    };
 //--------------------------------------------
    //define your function here
    


    void UserRegisterRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string UserRegister(Account &person);
    
    void UserLoginRest(const Rest::Request& request, Http::ResponseWriter response);
    bool UserLogin(std::string account, std::string pass);

    void getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string getUserinfo(std::string uuid , bool &is_get_success);

    void modifyUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string modifyUserinfo(Account &person);

    bool auth_token(const Rest::Request& request);

 //--------------------------------------------
private:
    UserModelServer() = default;
    //UserModelServer(const UserModelServer&) = default;
    //UserModelServer& operator=(const UserModelServer&) {};
    ~UserModelServer();

    static UserModelServer* instance;  //single object
};

std::string md5(std::string strPlain);


}// namespace hvs


#endif



