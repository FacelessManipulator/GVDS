#include <iostream>
#include <vector>
#include <sys/stat.h>
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
   Routes::Post(router, "/space/changesize", Routes::bind(&SpaceServer::SpaceSizeChangeRest, this));
}

    std::string jsonfilter(const std::string &serialize_json){
        return serialize_json.substr(0,serialize_json.find_last_of("}")+1);
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
    Space new_space;
    std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(hvs::CouchbaseDatastore("space_info"));
    spacePtr->init();
    SpaceMetaData tmpm;
    tmpm.deserialize(spacePathInfo);
    //1、判断是否可以创建空间，将host和storage的name转换成ID
    // TODO
    //2、获取账户映射信息，创建空间目录,返回spacePath
    // TODO
    //创建新建空间对应的空间文件夹，默认权限为0777,默认权限和用户有关
    std::string rootdir = *(HvsContext::get_context()->_config->get<std::string>("storage"));
    rootdir += spaceName;
    int mkret = mkdir(rootdir.c_str(), 0777);
    if (mkret != 0 ) {
        perror("SpaceCreate 文件夹创建错误！");
        return "false"; // 文件夹创建失败后，返回false字符串，用于之后的判断
    }
    new_space.spacePath = spaceName; // TODO: 默认当前 spacepath 在存储集群顶层，且空间的名字为相对lustre挂载点的路径；
    //3、权限增加模块
    // TODO：
    //4、空间分配容量记录到聚合模块中
    tmpm.hostCenterID = tmpm.hostCenterName;
    tmpm.storageSrcID = tmpm.storageSrcName;//资源聚合模块查找
    int addret = SpaceSizeAdd(tmpm.storageSrcID, spaceSize); // 把新修改的空间用量记录到聚合模块中
    if(addret != 0){
        return "false"; // 聚合模块修改失败，返回false字符串，用于之后的判断；
    }
    //5、写入数据库空间记录表中
    new_space.spaceName = spaceName;
    new_space.spaceSize = spaceSize;
    new_space.hostCenterID = tmpm.hostCenterID;
    new_space.storageSrcID = tmpm.storageSrcID;
//    new_space.spacePath = tmpm.spacePath; // ？ TODO: ？
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    const std::string tmp_uuid = boost::uuids::to_string(a_uuid);
    new_space.spaceID = tmp_uuid;
    std::string tmps_key = new_space.spaceID;
    std::string tmps_value = new_space.serialize();
    spacePtr->set(tmps_key, tmps_value);
    return new_space.spaceID;
}

std::string SpaceServer::SpaceCheck(std::string ownerID, std::vector<std::string> memberID, std::string spacePathInfo)
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
        if (newSpaceSize < 0) {
            return -1;
        }
        //查找空间表中，当前空间
        Space spacejson;
        std::shared_ptr<hvs::CouchbaseDatastore> spaceptr =std::make_shared<hvs::CouchbaseDatastore>(hvs::CouchbaseDatastore("space_info"));
        spaceptr->init();
        auto [vp, err] = spaceptr->get(spaceID); // 通过spaceID获取到描述空间的json数据；
        std::string space_value = *vp;
        spacejson.deserialize(jsonfilter(space_value));
//      查看本机当前空间占用的数据的文件夹的大小，默认为2GB TODO: 查询本本机空间节点占用容量
        if (newSpaceSize <= 2) {
            return -1;
        }
        if(newSpaceSize > spacejson.spaceSize){
            int ret = SpaceSizeAdd(spacejson.storageSrcID, newSpaceSize-spacejson.spaceSize);
            if(ret == 0){
                spacejson.spaceSize = newSpaceSize;
                spaceptr->set(spaceID, spacejson.serialize());
            }
            return ret;
        }else if (newSpaceSize < spacejson.spaceSize){
            int ret = SpaceSizeDeduct(spacejson.storageSrcID, spacejson.spaceSize-newSpaceSize);
            if(ret == 0){
                spacejson.spaceSize = newSpaceSize;
                spaceptr->set(spaceID, spacejson.serialize());
            }
            return ret;
        }else{
            return 0;
        }
    }

    int SpaceServer::SpaceSizeAdd(std::string StorageID, int64_t add_size) {
        StorageResource storage;
        std::shared_ptr<hvs::CouchbaseDatastore> storptr =std::make_shared<hvs::CouchbaseDatastore>(hvs::CouchbaseDatastore("test"));
        storptr->init();
        auto [vp, err] = storptr->get(StorageID);
        std::string stor_value = *vp;
        storage.deserialize(jsonfilter(stor_value));
        if (storage.assign_capacity+add_size > storage.total_capacity){
            return -1;
        } else {
            //设置当前新的容量；
            storage.assign_capacity = storage.assign_capacity+add_size;
            storptr->set(StorageID, storage.serialize()); // 容量改变，并重新存储到数据库中；
        }
        return 0;
    }

    int SpaceServer::SpaceSizeDeduct(std::string StorageID, int64_t deduct_size) {
        StorageResource storage;
        std::shared_ptr<hvs::CouchbaseDatastore> storptr =std::make_shared<hvs::CouchbaseDatastore>(hvs::CouchbaseDatastore("test"));
        storptr->init();
        auto [vp, err] = storptr->get(StorageID);
        std::string stor_value = *vp;
        storage.deserialize(jsonfilter(stor_value));
        if (storage.assign_capacity-deduct_size < 0){
            return -1;
        } else{
            //设置当前新的容量；
            storage.assign_capacity = storage.assign_capacity-deduct_size;
            storptr->set(StorageID, storage.serialize()); // 容量改变，并重新存储到数据库中；
        }
        return 0;
    }

}//namespace hvs