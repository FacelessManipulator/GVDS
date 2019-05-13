#include <iostream>
#include <vector>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"

#include "manager/space/SpaceServer.h"


namespace hvs{
using namespace Pistache::Rest;
using namespace Pistache::Http;
SpaceServer* SpaceServer::instance = nullptr;

void SpaceServer::start() {}

void SpaceServer::stop() {}

void SpaceServer::router(Router& router) {
//   Routes::Post(router, "/zone/rename", Routes::bind(&ZoneServer::ZoneRenameRest, this));
}

void SpaceServer::GetSpacePosition(std::vector<std::string> &result, std::vector<std::string> spaceID)
{
    Space tmps;
    std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("space_info"));
    spacePtr->init();
    for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
            {
                std::string tmps_key = *m;
                auto[vs, err] = spacePtr->get(tmps_key);
                std::string tmps_value = *vs;
                tmps.deserialize(tmps_value);
                SpaceMetaData tmpm;
                tmpm.spaceID = tmps.spaceID;
                tmpm.spaceName = tmps.spaceName;
                tmpm.hostCenterID = tmps.hostCenterID;
                tmpm.storageSrcID = tmps.storageSrcID;
                //TODO:资源聚合模块查询名字
                tmpm.spacePath = tmps.spacePath;
                std::string result_spa_m = tmpm.serialize();
                result.emplace_back(result_spa_m);
            }
}

void SpaceServer::GetSpaceInfo(std::string &result_s, std::vector<std::string> spaceID)
{
    Space tmps;
    SpaceInfo tmp_si;
    tmp_si.spaceID = spaceID;
    std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("space_info"));
    spacePtr->init();
    for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
    {
        std::string tmps_key = *m;
        auto[vs, err] = spacePtr->get(tmps_key);
        std::string tmps_value = *vs;
        tmps.deserialize(tmps_value);
        
        tmp_si.spaceName[tmps.spaceID] = tmps.spaceName;
        tmp_si.spaceSize[tmps.spaceID] = tmps.spaceSize;
    }
    result_s = tmp_si.serialize();
}

std::string SpaceServer::SpaceCreate(std::string spaceName, std::string ownerID, std::vector<std::string> memberID, int64_t spaceSize, std::string spacePathInfo)
{
    Space tmps;
    std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("space_info"));
    spacePtr->init();
    SpaceMetaData tmpm;
    tmpm.deserialize(spacePathInfo);
    //1、判断是否可以创建空间，将host和storage的name转换成ID
    //2、获取账户映射信息，创建空间目录
    //3、权限增加模块
    //4、空间分配容量记录
    //5、写入数据库
    tmps.spaceName = spaceName;
    tmps.spaceSize = spaceSize;
    tmps.hostCenterID = tmpm.hostCenterName;
    tmps.storageSrcID = tmpm.storageSrcName;//需要转换成ID；
    tmps.spacePath = tmpm.spacePath;
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    const std::string tmp_uuid = boost::uuids::to_string(a_uuid);
    tmps.spaceID = tmp_uuid;
    std::string tmps_key = tmps.spaceID;
    std::string tmps_value = tmps.serialize();
    spacePtr->set(tmps_key, tmps_value);
    return tmps.spaceID;
}

int SpaceServer::SpaceDelete(std::vector<std::string> spaceID)
{
    Space tmps;
    std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("space_info"));
    spacePtr->init();
    for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
            {
                std::string tmps_key = *m;
                auto[vs, err] = spacePtr->get(tmps_key);
                std::string tmps_value = *vs;
                std::cout << tmps_value << std::endl;
                tmps.deserialize(tmps_value);
                std::cout << tmps.spaceID << std::endl;
                tmps.status = false;
                tmps_value = tmps.serialize();
                spacePtr->set(tmps_key, tmps_value);
            }
    return 0;
}


}//namespace hvs