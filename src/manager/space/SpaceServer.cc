#include <iostream>
#include <vector>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"

#include "manager/space/SpaceServer.h"
#include "SpaceServer.h"


namespace hvs{
using namespace Pistache::Rest;
using namespace Pistache::Http;
//SpaceServer* SpaceServer::instance = nullptr;

void SpaceServer::start() {}

void SpaceServer::stop() {}

void SpaceServer::router(Router& router) {
   Routes::Post(router, "/space/rename", Routes::bind(&SpaceServer::SpaceRenameRest, this));
   Routes::Post(router, "/space/sizechange", Routes::bind(&SpaceServer::SpaceSizeChangeRest, this));
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
    //2、获取账户映射信息，创建空间目录,返回spacePath
    //3、权限增加模块
    //4、空间分配容量记录
    //5、写入数据库
    tmpm.hostCenterID = tmpm.hostCenterName;
    tmpm.storageSrcID = tmpm.storageSrcName;//资源聚合模块查找
    tmps.spaceName = spaceName; //
    tmps.spaceSize = spaceSize;
    tmps.hostCenterID = tmpm.hostCenterID;
    tmps.storageSrcID = tmpm.storageSrcID;
    tmps.spacePath = tmpm.spacePath;
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    const std::string tmp_uuid = boost::uuids::to_string(a_uuid);
    tmps.spaceID = tmp_uuid;
    std::string tmps_key = tmps.spaceID;
    std::string tmps_value = tmps.serialize();
    spacePtr->set(tmps_key, tmps_value);
    return tmps.spaceID;
}

std::string SpaceServer::SpaceCheck(std::string spaceName, std::string ownerID, std::vector<std::string> memberID, int64_t spaceSize, std::string spacePathInfo)
{
    Space tmps;
    std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("space_info"));
    spacePtr->init();
    SpaceMetaData tmpm;
    tmpm.deserialize(spacePathInfo);
    tmpm.hostCenterID = tmpm.hostCenterName;
    tmpm.storageSrcID = tmpm.storageSrcName;//资源聚合模块查找
    //find
    std::string query = "select * from `space_info` where SC_UUID = \"scuuid\" and Storage_UUID = \"stuuid\" and root_location = \"rootlocation\";";
    int pos1 = query.find("scuuid");
    query.erase(pos1, 6);
    query.insert(pos1, tmpm.hostCenterID);
    int pos2 = query.find("stuuid");
    query.erase(pos2, 6);
    query.insert(pos2, tmpm.storageSrcID);  
    int pos3 = query.find("rootlocation");
    query.erase(pos3, 12);
    query.insert(pos3, tmpm.spacePath);
    //std::cout << query <<std::endl;
    auto [vp, err] = spacePtr->n1ql(query);
    if (vp->size() != 1) return "false";
    else
    {
        std::vector<std::string>::iterator it = vp->begin();
        std::string n1ql_result = *it;
        std::string tmp_value = n1ql_result.substr(14, n1ql_result.length() - 15);
        tmps.deserialize(tmp_value);
        tmps.status = true;
        tmp_value = tmps.serialize();
        std::string tmp_key = tmps.spaceID;
        spacePtr->set(tmp_key,tmp_value);
        return tmps.spaceID;
    }
}

int SpaceServer::SpaceDelete(std::vector<std::string> spaceID)
{
    //是否要在实际集群中ownerID？
    for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
    {
        Space tmps;
        std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
            hvs::CouchbaseDatastore("space_info"));
        spacePtr->init();
        std::string tmps_key = *m;
        auto[vs, err] = spacePtr->get(tmps_key);
        std::string tmps_value = *vs;
        std::cout << tmps_value << std::endl;
        tmps.deserialize(tmps_value);
        std::cout << tmps.spaceID << std::endl;
        tmps.status = false;
        tmps_value = tmps.serialize();
        spacePtr->set(tmps_key, tmps_value);
        //修改空间分配容量
    }
    return 0;
}


void SpaceServer::SpaceRenameRest(const Rest::Request& request, Http::ResponseWriter response){
    std::cout << "====== start SpaceServer function: SpaceRenameRest ======"<< std::endl;
    auto info = request.body();

    SpaceRenameReq req;
    req.deserialize(info);
    std::string spaceID = req.spaceID;
    std::string newSpaceName = req.newSpaceName;

    int result_i = SpaceRename(spaceID, newSpaceName);
    std::string result;
    if (result_i == 0)
    result = "success";
    else result = "fail";

    response.send(Http::Code::Ok, result); //point
    std::cout << "====== end SpaceServer function: SpaceRenameRest ======"<< std::endl;
}

int SpaceServer::SpaceRename(std::string spaceID, std::string newSpaceName)
{
    //在lustre中改目录名是否需要ownerID?
    Space tmps;
    std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("space_info"));
    spacePtr->init();
    std::string tmp_key = spaceID;
    auto [vp, err] = spacePtr->get(tmp_key);
    std::string tmp_value = *vp;//待插入报错
    //std::cout << tmp_value << std::endl;
    tmps.deserialize(tmp_value);
    if (tmps.status == false) return -1;
    else
    {
        std::string oldSpaceName = tmps.spaceName;
        //TODO：在lustre中修改目录名
        tmps.spaceName = newSpaceName;
        int pos = tmps.spacePath.find(oldSpaceName);
        tmps.spacePath.replace(pos, oldSpaceName.length(), newSpaceName);

        tmp_value = tmps.serialize();
        spacePtr->set(tmp_key, tmp_value);
        return 0;
    }
}

    //YaoXu: 空间扩容缩容
    void SpaceServer::SpaceSizeChangeRest(const Rest::Request &request, Http::ResponseWriter response) {
        auto info = request.body();

        SpaceSizeChangeReq req;
        req.deserialize(info);
        std::string spaceID = req.spaceID;
        int64_t newSpaceSize = req.newSpaceSize;

        int result_i = SpaceSizeChange(spaceID, newSpaceSize);

        std::string result;

        if (result_i == 0){
            result = "success";
        }
        else {
            result = "fail";
        }
        response.send(Http::Code::Ok, result); //result;
    }

    int SpaceServer::SpaceSizeChange(std::string spaceID, int64_t newSpaceSize) {
        // 1. 进行查找聚合表，进行记录空间容量的变化；
        bool isAdd = true; //如果没有超过阈值，则增加空间容量
        if (isAdd){
            return SpaceSizeAdd(spaceID, newSpaceSize);
        }else{
            return SpaceSizeDeduct(spaceID, newSpaceSize);
        }
    }

    int SpaceServer::SpaceSizeAdd(std::string spaceID, int64_t newSpaceSize) {
        std::cout << "SpaceSizeAdd: " << spaceID << " " << newSpaceSize << std::endl;
        return 0;
    }

    int SpaceServer::SpaceSizeDeduct(std::string spaceID, int64_t newSpaceSize) {
        std::cout << "SpaceSizeDeduct: " << spaceID << " " << newSpaceSize << std::endl;
        return 0;
    }

}//namespace hvs