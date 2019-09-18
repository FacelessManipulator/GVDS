#include <iostream>
#include <vector>
#include <sys/stat.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "stdio.h"
#include "datastore/datastore.h"

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>

#include "manager/space/SpaceServer.h"
#include "manager/usermodel/UserModelServer.h"
#include "common/centerinfo.h"
#include "hvs_struct.h"


namespace hvs{
    using namespace Pistache::Rest;
    using namespace Pistache::Http;
    //SpaceServer* SpaceServer::instance = nullptr;

    void SpaceServer::start() {}

    void SpaceServer::stop() {}

    void SpaceServer::router(Router& router) {
       Routes::Post(router, "/space/rename", Routes::bind(&SpaceServer::SpaceRenameRest, this));
       Routes::Post(router, "/space/changesizeapply", Routes::bind(&SpaceServer::SpaceSizeChangeApplyRest, this));
       Routes::Post(router, "/space/changesize", Routes::bind(&SpaceServer::SpaceSizeChangeRest, this));
       Routes::Post(router, "/space/spaceusage", Routes::bind(&SpaceServer::SpaceUsageRest, this));
       Routes::Post(router, "/space/spaceusagecheck", Routes::bind(&SpaceServer::SpaceUsageCheckRest, this));
    }

    void SpaceServer::GetSpacePosition(std::vector<Space> &result, std::vector<std::string> spaceID)
    {
        Space tmps;
        std::shared_ptr<hvs::Datastore> spacePtr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase, true);
        for(std::vector<std::string>::iterator m = spaceID.begin(); m != spaceID.end(); m++)
        {
            std::string tmps_key = *m;
            auto[vs, err] = spacePtr->get(tmps_key);
            if(err != 0){
                dout(10) << "GetSpacePosition-数据库错误：(可重试)" << err << dendl;
                return;
            }
            std::string tmps_value = *vs;
            tmps.deserialize(tmps_value);

            // TODO:资源聚合模块查询名字
            // 已补充资源聚合模块查询接口
            StorageResource storage;
            std::shared_ptr<hvs::Datastore> storPtr =hvs::DatastoreFactory::create_datastore(storagebucket, hvs::DatastoreType::couchbase, true);
            auto [vp, serr] = storPtr->get(tmps.storageSrcID);
            if(serr != 0){
                dout(10) << "GetSpacePosition-数据库错误：（可重试）" << serr << dendl;
                return;
            }
            std::string stor_value = *vp;
            storage.deserialize(stor_value);
            result.emplace_back(tmps);
        }
    }

    void SpaceServer::GetSpaceInfo(std::vector<Space> &result_s, std::vector<std::string> spaceID)
    {
        Space tmps;
        std::vector<Space> tmp_si;
        std::shared_ptr<hvs::Datastore> spacePtr = hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase, true);
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

    std::string SpaceServer::SpaceCreate(std::string spaceName, std::string ownerID, std::vector<std::string> memberID, int64_t spaceSize, std::string spacePathInfo, std::string groupname)
    {
        Space new_space;
        std::shared_ptr<hvs::Datastore> spacePtr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase, true);
        Space tmpm;
        tmpm.deserialize(spacePathInfo);
        //1、判断是否可以创建空间，将host和storage的name转换成ID
        // TODO: 空间位置选择接口
        auto [storid, hostid] = GetSpaceCreatePath(spaceSize, tmpm.hostCenterName, tmpm.storageSrcName);
        if(storid.empty() && hostid.empty()){
            dout(10) << "SpaceCreate：存储资源选择失败！" << dendl;
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
        std::string rootdir = localstoragepath + "/";
        rootdir += tmp_uuid; // TODO: 路径拼接，后期如果有子空间再修改；
        dout(10) << "rootdir" << rootdir <<dendl;
        int mkret = mkdir(rootdir.c_str(), 0770);

        if (mkret != 0 ) {
            dout(10) << "SpaceCreate失败！" << dendl;
            return "-2"; // 文件夹创建失败后，返回false字符串，用于之后的判断
        }
        UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());
        string m_value = p_usermodel->getLocalAccountinfo(ownerID, tmpm.hostCenterName); 
        if (m_value.compare("fail") == 0){
            dout(10) << "getLocalAccountinfo fail" <<dendl;
            return "-4";
        }
        LocalAccountPair owner_localpair;
        owner_localpair.deserialize(m_value);
        //创建组
        std::string gp = groupname; 
        std::string group_cmd = "groupadd " + gp;
        dout(10) << group_cmd << dendl;
        system(group_cmd.c_str());

        //设置权限
        string new_cmd = "chown -R " + owner_localpair.localaccount + ":" + gp + " " + rootdir;
        system(new_cmd.c_str());


        new_space.spacePath = tmp_uuid; // TODO: 默认当前 spacepath 在存储集群顶层，且空间的名字为相对lustre挂载点的路径，并且使用UUID作为文件夹的名字
        //3、权限增加模块 ： 不需要
        //4、空间分配容量记录到聚合模块中

        //5、写入数据库空间记录表中
        new_space.spaceName = spaceName;
        new_space.spaceSize = spaceSize;
        new_space.hostCenterID = tmpm.hostCenterID;
        new_space.hostCenterName = tmpm.hostCenterName;
        new_space.storageSrcID = tmpm.storageSrcID;
        new_space.storageSrcName = tmpm.storageSrcName;
        new_space.status = true;
        if (new_space.spaceName == "" || new_space.spaceSize == 0) return "-2";
        std::string tmps_key = new_space.spaceID;
        std::string tmps_value = new_space.serialize();
        int flag = spacePtr->set(tmps_key, tmps_value);
        if (flag != 0) return "-2";
        return new_space.spaceID;
    }

    struct greaters
    {
        bool operator()(const int _Left, const int _Right) const
        {
            return (_Left > _Right);
        }
    };

    std::tuple<std::string, std::string> SpaceServer::GetSpaceCreatePath(int64_t spaceSize, std::string& hostCenterName, std::string& storageSrcName) {
        std::shared_ptr<hvs::Datastore> dbPtr = hvs::DatastoreFactory::create_datastore(storagebucket, hvs::DatastoreType::couchbase, true);
        auto storagePtr = static_cast<CouchbaseDatastore*>(dbPtr.get());

        char query[256];

        //根据存储集群名称查找 存储集群ID
        if(storageSrcName.empty()){
            snprintf(query, 256, 
            "select assign_capacity,host_center_id,host_center_name,mgs_address,state,storage_src_id, storage_src_name, total_capacity from `%s` "
            "where host_center_name = \"%s\"", 
            storagebucket.c_str(), hostCenterName.c_str());
        }else{
            snprintf(query, 256, 
            "select assign_capacity,host_center_id,host_center_name,mgs_address,state,storage_src_id, storage_src_name, total_capacity from `%s` "
            "where host_center_name = \"%s\" and storage_src_name = \"%s\"", 
            storagebucket.c_str(), hostCenterName.c_str(), storageSrcName.c_str());
        }
        auto [vp, err] = storagePtr->n1ql(string(query));
        if(err != 0){
            dout(10) << "GetSpaceCreatePath-数据库错误(可重试)：" << err << dendl;
            return {"", ""};
        }
        if(vp->size() == 0){
            dout(10) << "GetSpaceCreatePath: 未查找到制定的存储ID，根据空间名和数据中心的名字！" << dendl;
            return {"", ""};
        }

        // 2019年08月11日 默认存储集群选择策略为：从查找到的所有 存储集群中选择剩余空间最多的集群，进行获取其 id 和 host_center_id
        map<int, string, greaters> ma;

        for(auto its = vp->begin(); its != vp->end(); ++its) {
            std::string tmp_value = *its;
            StorageResource storage;
            storage.deserialize(tmp_value);
            ma[storage.total_capacity-storage.assign_capacity] = tmp_value;
        }
        auto ite = ma.begin();
//        std::vector<std::string>::iterator it = vp->begin();
//        std::string n1ql_result = *it;
//        std::string tmp_value = n1ql_result.substr(sizeof("{\"\":")+storagebucket.size()-1, n1ql_result.length()-(sizeof("{\"\":")+storagebucket.size())); // 处理返回的字符串问题
        StorageResource storage;
        storage.deserialize(ite->second); // 使用剩余存储空间最多的集群进行作为默认集群；
        hostCenterName = storage.host_center_name;
        storageSrcName = storage.storage_src_name;
        // dout(10) << "SpaceCreatePath: " << tmp_value << dendl;
        // TODO: STOR- 这个标识后期需要修改，UUID格式要进行统一；
        return {"STOR-"+storage.storage_src_id, storage.host_center_id};
    }

    std::string SpaceServer::SpaceCheck(std::string ownerID, std::vector<std::string> memberID, std::string spacePathInfo)
    {
        std::shared_ptr<hvs::Datastore> dbPtr = hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase, true);
        auto spacePtr = static_cast<CouchbaseDatastore*>(dbPtr.get());
        Space tmpm;
        tmpm.deserialize(spacePathInfo);
        auto [storid, hostid] = GetSpaceCreatePath(0, tmpm.hostCenterName, tmpm.storageSrcName);
        if(storid.empty() && hostid.empty()){
            dout(10) << "SpaceCheck：空间位置选择失败！" << dendl;
            return "false";
        }
        tmpm.hostCenterID = hostid; // 记录数据中心ID
        tmpm.storageSrcID = storid; // 记录存储集群ID
        //find
        char query[256];
        snprintf(query, 256, 
        "select UUID, name, capcity, SC_UUID, Storage_UUID, root_location, hostCenterName, storageSrcName, Status from `%s` where SC_UUID = \"%s\" and Storage_UUID = \"%s\" and root_location = \"%s\"", 
        spacebucket.c_str(), tmpm.hostCenterID.c_str(), tmpm.storageSrcID.c_str(), tmpm.spacePath.c_str());

        auto [vp, err] = spacePtr->n1ql(string(query));
        if(err != 0){
            dout(10) << "SpaceCheck-数据库错误(可重试)：" << err << dendl;
            return "false";
        }
        if (vp->size() != 1) return "false";
        else
        {
            Space tmps;
            std::vector<std::string>::iterator it = vp->begin();
            std::string tmp_value = *it;
            dout(10) << "SpaceCheck: " << tmp_value << dendl;
            tmps.deserialize(tmp_value);
            if (tmps.status) return "false";
            else {
                tmps.status = true;
                // TODO：修改空间状态为可用状态，并增加聚合模块容量, 如果容量增加失败，则返回；
                if(SpaceSizeAdd(tmps.storageSrcID, tmps.spaceSize) != 0){
                    dout(10) << "SpaceCheck.SpaceSizeAdd wrong" << dendl;
                    return "false";
                }
                int flag = dbPtr->set(tmps.spaceID, tmps.serialize());
                if (flag != 0) return "false";
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
            std::shared_ptr<hvs::Datastore> spacePtr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase, true);
            std::string tmps_key = *m;
            auto[vs, err] = spacePtr->get(tmps_key);
            if(err != 0){
                dout(10) << "SpaceDelete-数据库错误(可重试)：" << err << dendl;
                return 0;
            }
            std::string tmps_value = *vs;
            tmps.deserialize(tmps_value);
            tmps.status = false;
            tmps_value = tmps.serialize();
            // TODO:修改空间分配容量; 当前直接在聚合表中把空间容量删除； 未检测实际使用的容量
            if(SpaceSizeDeduct(tmps.storageSrcID, tmps.spaceSize) != 0){
                dout(10) << "SpaceDelete.SpaceSizeDeduct wrong"  << dendl;
                continue; // 跳过数据库保存；
            }
            int flag = spacePtr->set(tmps_key, tmps_value);
            if (flag != 0) return -1;
        }
        return 0;
    }

    void SpaceServer::SpaceRenameRest(const Rest::Request& request, Http::ResponseWriter response){
        dout(10) << "====== start SpaceServer function: SpaceRenameRest ======"<< dendl;
        auto info = request.body();

        SpaceRequest req;
        req.deserialize(info);
        std::string spaceID = req.spaceID;
        std::string newSpaceName = req.newSpaceName;


        dout(10) << "info: " << info << dendl;

        int result_i = SpaceRename(spaceID, newSpaceName);
        std::string result;
        if (result_i == 0)
        result = "success";
        else result = "fail";

        response.send(Http::Code::Ok, result); //point
        dout(10) << "====== end SpaceServer function: SpaceRenameRest ======"<< dendl;
    }

    int SpaceServer::SpaceRename(std::string spaceID, std::string newSpaceName)
    {
        //TODO：在lustre中改目录名是否需要ownerID？
        Space space;
        std::shared_ptr<hvs::Datastore> spaceptr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase, true);
        auto [vp, err] = spaceptr->get(spaceID);
        if(err)
        {
            return -1;
        }
        std::string space_value = *vp;
        space.deserialize(space_value);
        if (!space.status) return -1;
        else
        {
            if (newSpaceName.size() > 50)
            {
                newSpaceName = newSpaceName.substr(50);
            }
            std::string oldSpaceName = space.spaceName;
            std::string oldpath = localstoragepath+oldSpaceName;
            std::string newpath = localstoragepath+newSpaceName;
            if(oldpath == newpath){
                dout(10) << "空间前后名称相同:" << oldpath << " -> " << newpath << dendl;
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
            int flag = spaceptr->set(spaceID, space.serialize());
            if (flag != 0) return -1;
            else return 0;
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

    // 容量分配的粒度为 GB
    int SpaceServer::SpaceSizeChange(std::string spaceID, int64_t newSpaceSize) {
        if (newSpaceSize < 0) {
            return -1;
        }
        //查找空间表中当前空间,判断当前是否能缩放空间，是增大空间还是减少空间。
        Space spacejson;
        std::shared_ptr<hvs::Datastore> spaceptr =hvs::DatastoreFactory::create_datastore(spacebucket, hvs::DatastoreType::couchbase, true);
        auto [vp, err] = spaceptr->get(spaceID); // 通过spaceID获取到描述空间的json数据；
        if(err)
        {
            return -1;
        }
        std::string space_value = *vp;
        spacejson.deserialize(space_value);
        //查看本机当前空间占用的数据的文件夹的大小，默认为2GB TODO: 查询本本机空间节点占用容量
        //TODO:加入usage判断------------------
        std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore(bucket_account_info));
        f0_dbPtr->init();
        string c_key = "center_information";
        auto [pcenter_value, c_error] = f0_dbPtr->get(c_key);
        if (c_error){
            dout(10) << "authmodelserver: get center_information fail" <<dendl;
            return -1;
        }
        CenterInfo mycenter;
        mycenter.deserialize(*pcenter_value);
        //--------------------
        Http::Client client;
        char url[256];

        auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
        client.init(opts);
        int64_t realsize;
        //----------------------
        if(ManagerID == spacejson.hostCenterID){ //本地
            //调本地
            realsize = (SpaceUsage(spacejson.spacePath));
        }
        else{ //远端
            dout(10) <<"发送到远端" <<dendl;
            string tmp_ip = mycenter.centerIP[spacejson.hostCenterID];
            string tmp_port = mycenter.centerPort[spacejson.hostCenterID];
            
            snprintf(url, 256, "http://%s:%s/space/spaceusage", tmp_ip.c_str() ,tmp_port.c_str());

            auto response = client.post(url).body(spacejson.spacePath).send();
            dout(-1) << "Client Info: post request " << url << dendl;

            std::promise<bool> prom;
            auto fu = prom.get_future();
            response.then(
                [&](Http::Response resp) {
                dout(10) << "Response code = " << resp.code() << dendl;
                auto body = resp.body();
                int64_t spaceuseage;
                json_decode(body, spaceuseage);
                realsize = spaceuseage;
                prom.set_value(true);
            },
            Async::IgnoreException);
            //阻塞
            fu.get();
            dout(10) << "fu.get()"<<dendl;
        }
        client.shutdown();
        //不需要记录数据库，查询容量完毕
        //-------------------
        int64_t realsizetoGB =  (realsize/(1000*1000)+1)/1; // 不够1GB向上取整，获取最终值，容量分配的粒度为GB

        if (newSpaceSize <= realsizetoGB) {
            return -1;
        }

        if(newSpaceSize > spacejson.spaceSize){
            int ret = SpaceSizeAdd(spacejson.storageSrcID, newSpaceSize-spacejson.spaceSize);
            if(ret == 0){
                spacejson.spaceSize = newSpaceSize;
                int flag = spaceptr->set(spaceID, spacejson.serialize());
                if (flag != 0) return -1;
            }
            return ret;
        }else if (newSpaceSize < spacejson.spaceSize){
            int ret = SpaceSizeDeduct(spacejson.storageSrcID, spacejson.spaceSize-newSpaceSize);
            if(ret == 0){
                spacejson.spaceSize = newSpaceSize;
                int flag2 = spaceptr->set(spaceID, spacejson.serialize());
                if (flag2 != 0) return -1;
            }
            return ret;
        }else{
            return 0;
        }
    }

    int SpaceServer::SpaceSizeAdd(std::string StorageID, int64_t add_size) {
        StorageResource storage;
        std::shared_ptr<hvs::Datastore> storptr =hvs::DatastoreFactory::create_datastore(storagebucket, hvs::DatastoreType::couchbase, true);
        auto [vp, err] = storptr->get(StorageID);
        if(err)
        {
            return -1;
        }
        std::string stor_value = *vp;
        storage.deserialize(stor_value);
        if (storage.assign_capacity+add_size > storage.total_capacity){
            return -1;
        } else {
            //设置当前新的容量；
            storage.assign_capacity = storage.assign_capacity+add_size;
            int flag = storptr->set(StorageID, storage.serialize()); // 容量改变，并重新存储到数据库中；
            if (flag != 0) return -1;
        }
        return 0;
    }

    int SpaceServer::SpaceSizeDeduct(std::string StorageID, int64_t deduct_size) {
        StorageResource storage;
        std::shared_ptr<hvs::Datastore> storptr =hvs::DatastoreFactory::create_datastore(storagebucket, hvs::DatastoreType::couchbase, true);
        auto [vp, err] = storptr->get(StorageID);
        if(err)
        {
            return -1;
        }
        std::string stor_value = *vp;
        storage.deserialize(stor_value);
        if (storage.assign_capacity-deduct_size < 0){
            return -1;
        } else{
            //设置当前新的容量；
            storage.assign_capacity = storage.assign_capacity-deduct_size;
            int flag = storptr->set(StorageID, storage.serialize()); // 容量改变，并重新存储到数据库中；
            if (flag != 0) return -1;
        }
        return 0;
    }

    // 注意此接口查找的 文件的大小 返回的单位 以 KB 为单位返回，121000表示 121000KB 121MB 注意此处 KB 与 KiB的区别；
    void SpaceServer::SpaceUsageCheckRest(const Rest::Request& request, Http::ResponseWriter response){
        dout(10) << "====== start SpaceServer function: SpaceUsageCheckRest ======"<< dendl;
        auto info = request.body();

        SpaceRequest req;
        req.deserialize(info);
        std::vector<std::string> spaceID = req.spaceIDs;

        std::vector<int64_t> result_i = SpaceUsageCheck(spaceID);
        std::string result;
        if (!result_i.empty()){
            result = json_encode(result_i);
        }
        else{
            result = "fail";
        }

        response.send(Http::Code::Ok, result); //point
        dout(10) << "====== end SpaceServer function: SpaceUsageCheckRest ======"<< dendl;
    }

    std::vector<int64_t> SpaceServer::SpaceUsageCheck(std::vector<std::string> spaceID){
        std::vector<int64_t> res;
        //string localstoragepath = *(HvsContext::get_context()->_config->get<std::string>("manager.data_path"));

        std::vector<Space> result; //里面存储每个空间的string信息 需要反序列化
        GetSpacePosition(result, std::move(spaceID));

        //---提前读取center_information信息，为后面查找各中心信息做准备----
        std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
            hvs::CouchbaseDatastore(bucket_account_info));
        f0_dbPtr->init();
        string c_key = "center_information";
        auto [pcenter_value, c_error] = f0_dbPtr->get(c_key);
        if (c_error){
            dout(10) << "authmodelserver: get center_information fail" <<dendl;
            return res;
        }
        CenterInfo mycenter;
        mycenter.deserialize(*pcenter_value);
        //--------------------
        Http::Client client;
        char url[256];

        auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
        client.init(opts);
        //----------------------
        for (const auto &space_iter : result){ //对每个空间进行查询
            Space spacemeta = space_iter;

            // string spacepath = localstoragepath + "/" + spacemeta.spacePath;
            string spacepath = spacemeta.spacePath;  //TODO：每个空间的local spacepath 各异，都不是一样的，具体路径在配置文件中配置；

            if(ManagerID == spacemeta.hostCenterID){ //本地
                //调本地
                int64_t spaceuse = SpaceUsage(spacepath);
                res.push_back(spaceuse);
            }
            else{ //远端
                dout(10) <<"发送到远端" <<dendl;
                string tmp_ip = mycenter.centerIP[spacemeta.hostCenterID];
                string tmp_port = mycenter.centerPort[spacemeta.hostCenterID];
                
                snprintf(url, 256, "http://%s:%s/space/spaceusage", tmp_ip.c_str() ,tmp_port.c_str());

                auto response = client.post(url).body(spacepath).send();
                dout(-1) << "Client Info: post request " << url << dendl;

                std::promise<bool> prom;
                auto fu = prom.get_future();
                response.then(
                    [&](Http::Response resp) {
                    dout(10) << "Response code = " << resp.code() << dendl;
                    auto body = resp.body();
                    int64_t spaceuseage;
                    json_decode(body, spaceuseage);
                    res.push_back(spaceuseage);
                    prom.set_value(true);
                },
                Async::IgnoreException);
                //阻塞
                fu.get();
                dout(10) << "fu.get()"<<dendl;
            }
        }
        client.shutdown();
        //不需要记录数据库，查询容量完毕；
        return res;
    }

    void SpaceServer::SpaceUsageRest(const Rest::Request& request, Http::ResponseWriter response){
        dout(10) << "====== start SpaceServer function: SpaceUsageRest ======"<< dendl;
        std::string spacepath = request.body();
        dout(10) << "SpacePath:" << spacepath << dendl;
        int64_t result_i = SpaceUsage(spacepath);
        std::string result = json_encode(result_i);
        response.send(Http::Code::Ok, result); //point
        dout(10) << "====== end SpaceServer function: SpaceUsageRest ======"<< dendl;
    }

    int64_t SpaceServer::SpaceUsage(std::string spacepath){ // 检测出来的文件的大小以KB为单位返回；注意此接口中spacepath为相对本机的路径，比如 /tmp 就是 本机的 /tmp目录；
        string localstoragepath = *(HvsContext::get_context()->_config->get<std::string>("manager.data_path"));
        dout(10) <<"localstoragepath: " << localstoragepath <<dendl;
        spacepath = localstoragepath + spacepath;  // 获取全局根路径
        dout(10) <<"localstoragepath-spacepath: " << spacepath << dendl;
        int cmdsize = 150; // 默认命令的总长度不超过150个字符
        int bufsize = 200;
        char cmd[cmdsize];
        char buf[bufsize];
        memset(cmd, '\0', static_cast<size_t>(cmdsize));
        memset(buf, '\0', static_cast<size_t>(bufsize));
        if(sprintf(cmd, "du -h -d 0 %s", spacepath.c_str()) <0 ){
            dout(10) << "sprintf() wrong" << dendl;
            return -1;
        }
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            dout(10) << "popen() wrong" << dendl;
            return -1;
        }
        fgets(buf, bufsize-1, fp);
        std::string cmdresult(buf);
        // dout(10) << "du 运行结果：" << cmdresult;
        // 下面进行根据获取的返回值解析容量大小
        int formal_size = 0;
        size_t start = 0;
        start = cmdresult.find_first_of(" ", start);
        size_t end = cmdresult.find_first_of("\t", start+1);
        // 获取容量字符串
        std::string size_str = cmdresult.substr(start+1, end-start-1);
        // 获取容量数字大小
        int unit = size_str.size()-1;
        // dout(10)  << size_str.substr(0, unit).c_str() << dendl;
        sscanf(size_str.substr(0, unit).c_str(), "%d", &formal_size);
        // dout(10) << "解析后容量大小(数字):"<< formal_size << size_str[unit]<< dendl;
        // 根据容量后单位G,M,K统一转化为KB为单位的容量大小
        int64_t size = 0;
        if(size_str[unit] == 'G'){
            size = formal_size * 1000 * 1000;
        }
        if(size_str[unit] == 'M'){
            size = formal_size*1000;
        }
        if(size_str[unit] == 'K'){
            size = formal_size;
        }
        // 输出字符串形式的容量大小
        // dout(10) << "解析后容量大小(字符串):" << size_str << dendl;
        return size;
    }

    void SpaceServer::SpaceSizeChangeApplyRest(const Rest::Request& request, Http::ResponseWriter response){
        dout(10) << "====== start ZoneServer function: SpaceSizeChangeApplyRest ======"<< dendl;
        auto info = request.body();

        dout(10) << "info: " << info << dendl;

        int result = SpaceSizeChangeApply(info);
        response.send(Http::Code::Ok, json_encode(result));
        dout(10) << "====== end ZoneServer function: SpaceSizeChangeApplyRest ======"<< dendl;
    }

    int SpaceServer::SpaceSizeChangeApply(std::string apply)
    {
        std::shared_ptr<hvs::Datastore> applyPtr =hvs::DatastoreFactory::create_datastore(applybucket, hvs::DatastoreType::couchbase, true);
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        const std::string uuid = boost::uuids::to_string(a_uuid);
        std::string key = "spsiz-" + uuid;
        struct_apply_info applyinfo;
        applyinfo.id = key;
        applyinfo.data = apply;
        std::string value = applyinfo.serialize();
        int flag = applyPtr->set(key, value);
        if(flag != 0)
        {
            return EAGAIN;
        }
        else return 0;
    }

}//namespace hvs