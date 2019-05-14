#ifndef SPACESERVER_H
#define SPACESERVER_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <iostream>
#include <map>


#include "datastore/couchbase_helper.h"
#include "manager/space/Space.h" 
#include "manager/manager.h"


using namespace Pistache;


namespace hvs{
class SpaceServer : public ManagerModule{
private:
  virtual void start() override;
  virtual void stop() override;
  virtual void router(Pistache::Rest::Router&) override;

public:
 //--------------------------------------------
    //define your function here
    
    //空间定位模块：空间定位接口
    void GetSpacePosition(std::vector<std::string> &result, std::vector<std::string> spaceID);

    //空间信息检索模块：空间信息检索接口
    void GetSpaceInfo(std::string &result_s, std::vector<std::string> spaceID);

    //空间创建模块：空间创建接口
    std::string SpaceCreate(std::string spaceName, std::string ownerID, std::vector<std::string> memberID, int64_t spaceSize, std::string spacePathInfo);

    //空间创建模块：添加区域空间校验接口
    std::string SpaceCheck(std::string spaceName, std::string ownerID, std::vector<std::string> memberID, int64_t spaceSize, std::string spacePathInfo);
    
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
public:
    SpaceServer() : ManagerModule("space") {};
    ~SpaceServer() {};

    static SpaceServer* instance;  //single object
};

//std::string md5(std::string strPlain);


}// namespace hvs


#endif




