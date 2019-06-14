#include <utility>

#include <utility>

#include <utility>

//
// Created by yaowen on 5/28/19.
// 北航系统结构所-存储组
//

#include "zone_mod.h"
#include "client/msg_mod.h"

using namespace std;
using namespace hvs;

void hvs::ClientZone::start(){
    m_stop = false;
    create("client-zone-mod");
}

void hvs::ClientZone::stop(){
    m_stop = true;
}

void* ClientZone::entry() {
    while(!m_stop) {
        client->zone->GetZoneInfo("202");
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

bool hvs::ClientZone::GetZoneInfo(std::string clientID) {
    GetZoneInfoRes zoneinfores;
    string endpoint = client->get_manager();
    string inforesult = client->rpc->post_request(endpoint, "/zone/info", clientID);
    if (!inforesult.empty()) {
        zoneinfores.deserialize(inforesult); //获取返回的结果
    }
    if(zoneinfores.zoneInfoResult.empty()){
        return false;
    }
    spacemap_mutex.lock_shared();
    spaceuuid_to_metadatamap.clear(); // 进行清空map
    spacemap_mutex.unlock_shared();
    zonemap_mutex.lock_shared();
    zonemap.clear();
    for(const auto &it : zoneinfores.zoneInfoResult){
        ZoneInfo zoneinfo;
        zoneinfo.deserialize(it);
        // TODO: 获取空间信息对每个空间，并更新到内存中；
        GetLocateInfo(clientID, zoneinfo.zoneID, zoneinfo.spaceBicInfo.spaceID);
        zonemap[zoneinfo.zoneName] = it;
    }
    zonemap_mutex.unlock_shared();
    return true;
}

bool ClientZone::GetLocateInfo(std::string clientID, std::string zoneID, std::vector<std::string> &spaceIDs) {
    GetZoneLocateInfoReq req;
    req.clientID = std::move(clientID);
    req.zoneID = std::move(zoneID);
    req.spaceID = spaceIDs;
    std::string req_value = req.serialize();
    GetZoneLocateInfoRes zonelocateresult;
    string endpoint = client->get_manager();
    string inforesult = client->rpc->post_request(endpoint, "/zone/locate", req_value);
    if (!inforesult.empty()) {
        zonelocateresult.deserialize(inforesult); //获取返回的结果
    }
    spacemap_mutex.lock_shared(); // 锁定 spaceuuid_to_metadatamap 保证线程安全
    for(const auto &it : zonelocateresult.zoneLocateInfoResult){
        SpaceMetaData space_metadata;
        space_metadata.deserialize(it);
        // TODO: 目前暂时存储在内存中的 spaceuuid_to_metadatamap 之中
        spaceuuid_to_metadatamap[space_metadata.spaceID] = it;
    }
    spacemap_mutex.unlock_shared();
    return false;
}

// TODO: 工具函数，分割字符串
std::vector<std::string> splitWithStl(const std::string str,const std::string pattern)
{
    std::vector<std::string> resVec;

    if ("" == str)
    {
        return resVec;
    }
    //方便截取最后一段数据
    std::string strs = str + pattern;

    size_t pos = strs.find(pattern);
    size_t size = strs.size();

    while (pos != std::string::npos)
    {
        std::string x = strs.substr(0,pos);
        resVec.push_back(x);
        strs = strs.substr(pos+1,size);
        pos = strs.find(pattern);
    }

    return resVec;
}

std::tuple<std::string, std::string, std::string, std::string> hvs::ClientZone::locatePosition(const std::string path){
    std::vector<std::string> namev = splitWithStl(path, "/");
    auto pos = path.find('/', 1);
    pos = path.find('/', pos+1);
    std::string zonename = namev[1];
    std::string spacename = namev[2];
    std::string remotepath;
    std::string spaceuuid;
    if(pos == -1){
        remotepath = "/";
    } else {
        remotepath = path.substr(pos);
    }
    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        ZoneInfo zoneinfo;
        zoneinfo.deserialize(mapping->second);
        for(auto it : zoneinfo.spaceBicInfo.spaceID){
            if (zoneinfo.spaceBicInfo.spaceName[it] == namev[2]){
                spaceuuid = it;

            }
        }
    }
    return {zonename, spacename, spaceuuid, remotepath};
}

std::string ClientZone::spaceuuid_to_spacerpath(std::string uuid) {
    std::string rpath;
    SpaceMetaData space_metadata;
    spacemap_mutex.lock_shared();
    space_metadata.deserialize(spaceuuid_to_metadatamap[uuid]);
    spacemap_mutex.unlock_shared();
    rpath = space_metadata.spacePath;
    return rpath; // 返回存储集群路径
}

std::string ClientZone::spaceuuid_to_hostcenterID(std::string uuid) {
    std::string centerID;
    spacemap_mutex.lock_shared();
    SpaceMetaData space_metadata;
    space_metadata.deserialize(spaceuuid_to_metadatamap[uuid]);
    centerID = space_metadata.hostCenterID;
    spacemap_mutex.unlock_shared();
    return centerID;

}
