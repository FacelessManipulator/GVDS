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
#include "include/aggregation_struct.h"


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
    std::string SpaceCheck(std::string ownerID, std::vector<std::string> memberID, std::string spacePathInfo);
    
    //空间删除模块：空间删除接口
    int SpaceDelete(std::vector<std::string> spaceID);

    //TODO：空间位置选择模块：空间位置选择接口
    std::string GetSpaceCreatePath(int64_t spaceSize, std::string spacePathInfo);

    //空间重命名模块：空间重命名接口
    void SpaceRenameRest(const Rest::Request& request, Http::ResponseWriter response);
    int SpaceRename(std::string spaceID, std::string newSpaceName);

    //空间缩放模块：空间缩放接口
    void SpaceSizeChangeRest(const Rest::Request& request, Http::ResponseWriter response);
    int SpaceSizeChange(std::string spaceID, int64_t newSpaceSize);
    int SpaceSizeAdd(std::string StorageID, int64_t newSpaceSize);
    int SpaceSizeDeduct(std::string StorageID, int64_t newSpaceSize);

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
    SpaceServer() : ManagerModule("space") {
        storagebucket = *(hvs::HvsContext::get_context()->_config->get<std::string>("couchbase.bucket"));
        spacebucket = "space_info";
        localstoragepath = *(HvsContext::get_context()->_config->get<std::string>("storage"));
    };
    ~SpaceServer() {};

    static SpaceServer* instance;  //single object
private:
    std::string storagebucket;
    std::string spacebucket;
    std::string localstoragepath; // 本机存储集群路径
};

//std::string md5(std::string strPlain);

}// namespace hvs


#endif




