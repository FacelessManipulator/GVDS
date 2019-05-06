#ifndef SPACESERVER_H
#define SPACESERVER_H

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <iostream>
#include <map>


#include "datastore/couchbase_helper.h"
#include "space/Space.h" 
//class Account;

using namespace Pistache;


namespace hvs{
class SpaceServer {
public:
    static SpaceServer* getInstance(){
        if (instance == nullptr)
            instance = new SpaceServer();
	    return instance;
    };
 //--------------------------------------------
    //define your function here
    
    //空间定位模块：空间定位接口
    void GetSpacePosition(std::vector<std::string> &result, std::vector<std::string> spaceID);

    //空间信息检索模块：空间信息检索接口
    void GetSpaceInfo(std::string &result_s, std::vector<std::string> spaceID);

    //空间删除模块：空间删除接口；
    int SpaceDelete(std::vector<std::string> spaceID);

/*
    void UserRegisterRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string UserRegister(Account &person);
    
    void UserLoginRest(const Rest::Request& request, Http::ResponseWriter response);
    bool UserLogin(std::string account, std::string pass);

    void getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string getUserinfo(std::string uuid , bool &is_get_success);

    void modifyUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string modifyUserinfo(Account &person);*/

 //--------------------------------------------
private:
    SpaceServer() = default;
    ~SpaceServer();

    static SpaceServer* instance;  //single object
};

//std::string md5(std::string strPlain);


}// namespace hvs


#endif




