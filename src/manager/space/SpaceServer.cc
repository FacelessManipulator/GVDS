#include <iostream>
#include <vector>
#include <sys/stat.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "stdio.h"
#include "datastore/datastore.h"

#include "manager/space/SpaceServer.h"
#include "hvs_struct.h"


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

    void SpaceServer::GetSpacePosition(std::vector<Space> &result, std::vector<std::string> spaceID)
    {
        Space tmps;
        std::shared_ptr<hvs::Datastore> spacePtr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase);
        for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
        {
            std::string tmps_key = *m;
            auto[vs, err] = spacePtr->get(tmps_key);
            std::string tmps_value = *vs;
            std::cout << "1" << tmps_value << std::endl;
            tmps.deserialize(tmps_value);

            //TODO:资源聚合模块查询名字
            // 已补充资源聚合模块查询接口
            StorageResource storage;
            std::shared_ptr<hvs::Datastore> storPtr =hvs::DatastoreFactory::create_datastore(storagebucket, hvs::DatastoreType::couchbase);
            auto [vp, serr] = storPtr->get(tmps.storageSrcID);
            std::string stor_value = *vp;
            storage.deserialize(stor_value);
            result.emplace_back(tmps);
        }
    }

    void SpaceServer::GetSpaceInfo(std::vector<Space> &result_s, std::vector<std::string> spaceID)
    {
        Space tmps;
        std::vector<Space> tmp_si;
        std::shared_ptr<hvs::Datastore> spacePtr = hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase);
        for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
        {
            std::string tmps_key = *m;
            auto[vs, err] = spacePtr->get(tmps_key);
            if(!err)
            {
                std::string tmps_value = *vs;
                tmps.deserialize(tmps_value);
                tmp_si.push_back(tmps);
            }
        }
        result_s.swap(tmp_si);
    }

    std::string SpaceServer::SpaceCreate(std::string spaceName, std::string ownerID, std::vector<std::string> memberID, int64_t spaceSize, std::string spacePathInfo)
    {
        Space new_space;
        std::shared_ptr<hvs::Datastore> spacePtr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase);
        Space tmpm;
        tmpm.deserialize(spacePathInfo);
        //1、判断是否可以创建空间，将host和storage的name转换成ID
        // TODO: 空间位置选择接口
        auto [storid, hostid] = GetSpaceCreatePath(spaceSize, tmpm.hostCenterName, tmpm.storageSrcName);
        if(storid.empty() && hostid.empty()){
            std::cerr << "SpaceCreate：存储资源选择失败！" << std::endl;
            return "-3";
        }
        tmpm.storageSrcID = storid;// 资源聚合模块查找 TODO: 用户可能指定存储位置
        tmpm.hostCenterID = hostid; // 通过制定的ID返回
        int addret = SpaceSizeAdd(tmpm.storageSrcID, spaceSize); // 把新修改的空间用量记录到聚合模块中

        if(addret != 0){
            return "-1"; // 聚合模块修改失败，返回false字符串，用于之后的判断；
        }
        //2、获取账户映射信息，创建空间目录,返回spacePath
        // TODO： 把空间组和用户使用 chown 修改； owner; ownerID, hostCenterID bool (&string &string, owner, hostcenterid) || string (owner, hostcenterid);
        //创建新建空间对应的空间文件夹，默认权限为0777,默认权限和用户有关
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        const std::string tmp_uuid = boost::uuids::to_string(a_uuid);
        new_space.spaceID = tmp_uuid;
        std::string rootdir = localstoragepath;
        rootdir += tmp_uuid; // TODO: 路径拼接，后期如果有子空间再修改；
        int mkret = mkdir(rootdir.c_str(), 0770);
        if (mkret != 0 ) {
            perror("SpaceCreate");
            return "-2"; // 文件夹创建失败后，返回false字符串，用于之后的判断
        }
        new_space.spacePath = tmp_uuid; // TODO: 默认当前 spacepath 在存储集群顶层，且空间的名字为相对lustre挂载点的路径，并且使用UUID作为文件夹的名字
        //3、权限增加模块 ： 不需要
        //4、空间分配容量记录到聚合模块中

        //5、写入数据库空间记录表中
        new_space.spaceName = spaceName;
        new_space.spaceSize = spaceSize;
        new_space.hostCenterID = tmpm.hostCenterID;
        new_space.storageSrcID = tmpm.storageSrcID;
        new_space.status = true;
        if (new_space.spaceName == "" || new_space.spaceSize == 0) return "-2";
        std::string tmps_key = new_space.spaceID;
        std::string tmps_value = new_space.serialize();
        spacePtr->set(tmps_key, tmps_value);
        return new_space.spaceID;
    }

    std::tuple<std::string, std::string> SpaceServer::GetSpaceCreatePath(int64_t spaceSize, std::string hostCenterName, std::string storageSrcName) {
        std::shared_ptr<hvs::CouchbaseDatastore> storagePtr = std::make_shared<hvs::CouchbaseDatastore>(
                hvs::CouchbaseDatastore(storagebucket));
        storagePtr->init();
        std::string query = "";
        //根据存储集群名称查找 存储集群ID
        if(storageSrcName.empty()){
            query = "select * from `"+storagebucket+ R"(` where host_center_name = "hcname";)";
            int pos1 = query.find("hcname");
            query.erase(pos1, 6);
            query.insert(pos1, hostCenterName);
        }else{
            query = "select * from `"+storagebucket+ R"(` where host_center_name = "hcname" and storage_src_name = "ssname";)";
            int pos1 = query.find("hcname");
            query.erase(pos1, 6);
            query.insert(pos1, hostCenterName);
            int pos2 = query.find("ssname");
            query.erase(pos2, 6);
            query.insert(pos2, storageSrcName);
        }
        // TODO: 默认使用第一个作为查找到的结果
        auto [vp, err] = storagePtr->n1ql(query);
        if(vp->size() == 0){
            std::cerr << "GetSpaceCreatePath: 未查找到制定的存储ID，根据空间名和数据中心的名字！" << std::endl;
            return {"", ""};
        }
        std::vector<std::string>::iterator it = vp->begin();
        std::string n1ql_result = *it;
        std::string tmp_value = n1ql_result.substr(sizeof("{\"\":")+storagebucket.size()-1, n1ql_result.length()-(sizeof("{\"\":")+storagebucket.size())); // 处理返回的字符串问题
        StorageResource storage;
        storage.deserialize(tmp_value);
        // std::cerr << "SpaceCreatePath: " << tmp_value << std::endl;
        // TODO: STOR- 这个标识后期需要修改，UUID格式要进行统一；
        return {"STOR-"+storage.storage_src_id, storage.host_center_id};
    }

    std::string SpaceServer::SpaceCheck(std::string ownerID, std::vector<std::string> memberID, std::string spacePathInfo)
    {
        std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
              hvs::CouchbaseDatastore(spacebucket));
        spacePtr->init();
        Space tmpm;
        tmpm.deserialize(spacePathInfo);
        auto [storid, hostid] = GetSpaceCreatePath(0, tmpm.hostCenterName, tmpm.storageSrcName);
        if(storid.empty() && hostid.empty()){
            std::cerr << "SpaceCheck：空间位置选择失败！" << std::endl;
            return "false";
        }
        tmpm.hostCenterID = hostid; // 记录数据中心ID
        tmpm.storageSrcID = storid; // 记录存储集群ID
        //find
        std::string query = R"(select * from `space_info` where SC_UUID = "scuuid" and Storage_UUID = "stuuid" and root_location = "rootlocation";)";
        int pos1 = query.find("scuuid");
        query.erase(pos1, 6);
        query.insert(pos1, tmpm.hostCenterID);
        int pos2 = query.find("stuuid");
        query.erase(pos2, 6);
        query.insert(pos2, tmpm.storageSrcID);
        int pos3 = query.find("rootlocation");
        query.erase(pos3, 12);
        query.insert(pos3, tmpm.spacePath);
        auto [vp, err] = spacePtr->n1ql(query);
        if (vp->size() != 1) return "false";
        else
        {
            Space tmps;
            std::vector<std::string>::iterator it = vp->begin();
            std::string n1ql_result = *it;
            std::string tmp_value = n1ql_result.substr(14, n1ql_result.length() - 15);
            std::cerr << "SpaceCheck: " << tmp_value << std::endl;
            tmps.deserialize(tmp_value);
            if (tmps.status) return "false";
            else {
                tmps.status = true;
                // TODO：修改空间状态为可用状态，并增加聚合模块容量, 如果容量增加失败，则返回；
                if(SpaceSizeAdd(tmps.storageSrcID, tmps.spaceSize) != 0){
                    perror("SpaceCheck.SpaceSizeAdd");
                    return "false";
                }
                spacePtr->set(tmps.spaceID, tmps.serialize());
                return tmps.spaceID;
            }
        }
    }

    int SpaceServer::SpaceDelete(std::vector<std::string> spaceID)
    {
        // TODO：是否要在实际集群中ownerID
        for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
        {
            Space tmps;
            std::shared_ptr<hvs::Datastore> spacePtr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase);
            std::string tmps_key = *m;
            auto[vs, err] = spacePtr->get(tmps_key);
            std::string tmps_value = *vs;
            tmps.deserialize(tmps_value);
            tmps.status = false;
            tmps_value = tmps.serialize();
            // TODO:修改空间分配容量; 当前直接在聚合表中把空间容量删除； 未检测实际使用的容量
            if(SpaceSizeDeduct(tmps.storageSrcID, tmps.spaceSize) != 0){
                perror("SpaceDelete.SpaceSizeDeduct");
                continue; // 跳过数据库保存；
            }
            spacePtr->set(tmps_key, tmps_value);
        }
        return 0;
    }

    void SpaceServer::SpaceRenameRest(const Rest::Request& request, Http::ResponseWriter response){
        std::cout << "====== start SpaceServer function: SpaceRenameRest ======"<< std::endl;
        auto info = request.body();

        SpaceRequest req;
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
        //TODO：在lustre中改目录名是否需要ownerID？
        Space space;
        std::shared_ptr<hvs::Datastore> spaceptr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase);
        auto [vp, err] = spaceptr->get(spaceID);
        std::string space_value = *vp;
        space.deserialize(space_value);
        if (!space.status) return -1;
        else
        {
            std::string oldSpaceName = space.spaceName;
            std::string oldpath = localstoragepath+oldSpaceName;
            std::string newpath = localstoragepath+newSpaceName;
            if(oldpath == newpath){
                std::cerr << "空间前后名称相同:" << oldpath << " -> " << newpath << std::endl;
                return 0;
            }
            //TODO: 修改目录名称，目前直接修改数据库表
//            if(rename(oldpath.c_str(), newpath.c_str())!=0){
//                perror("SpaceRename");
//                return -1;
//            }
            space.spaceName = newSpaceName;
//            unsigned long pos = space.spacePath.find(oldSpaceName);
//            space.spacePath.replace(pos, oldSpaceName.length(), newSpaceName);
            spaceptr->set(spaceID, space.serialize());
            return 0;
        }
    }

    //YaoXu: 空间扩容缩容
    void SpaceServer::SpaceSizeChangeRest(const Rest::Request &request, Http::ResponseWriter response) {
        auto info = request.body();

        SpaceRequest req;
        req.deserialize(info);
        std::string spaceID = req.spaceID;
        int64_t newSpaceSize = req.newSpaceSize;

        int result = SpaceSizeChange(spaceID, newSpaceSize);

        response.send(Http::Code::Ok, json_encode(result)); //result;
    }

    int SpaceServer::SpaceSizeChange(std::string spaceID, int64_t newSpaceSize) {
        if (newSpaceSize < 0) {
            return -1;
        }
        //查找空间表中当前空间,判断当前是否能缩放空间，是增大空间还是减少空间。
        Space spacejson;
        std::shared_ptr<hvs::Datastore> spaceptr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase);
        auto [vp, err] = spaceptr->get(spaceID); // 通过spaceID获取到描述空间的json数据；
        std::string space_value = *vp;
        spacejson.deserialize(space_value);
        //查看本机当前空间占用的数据的文件夹的大小，默认为2GB TODO: 查询本本机空间节点占用容量
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
        std::shared_ptr<hvs::Datastore> storptr =hvs::DatastoreFactory::create_datastore(storagebucket, hvs::DatastoreType::couchbase);
        auto [vp, err] = storptr->get(StorageID);
        std::string stor_value = *vp;
        storage.deserialize(stor_value);
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
        std::shared_ptr<hvs::Datastore> storptr =hvs::DatastoreFactory::create_datastore(storagebucket, hvs::DatastoreType::couchbase);
        auto [vp, err] = storptr->get(StorageID);
        std::string stor_value = *vp;
        storage.deserialize(stor_value);
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