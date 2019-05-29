#include <utility>

//
// Created by yaowen on 5/28/19.
// 北航系统结构所-存储组
//

#include "zone_mod.h"

void hvs::ClientZone::start(){
    std::cerr << "空间模块启动!" << std::endl;
    GetZoneInfo("127.0.0.1", 49517, "101");
}

void hvs::ClientZone::stop(){}

bool hvs::ClientZone::GetZoneInfo(std::string ip, int port, std::string clientID) {
    // 调用获取区域信息；
    Pistache::Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/zone/info", ip.c_str(), port);
    auto opts = Pistache::Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);
    std::string value = std::move(clientID);
    std::cerr<< "Client Info: post request " << url << std::endl;
    auto response = client.post(url).body(value).send();
    std::promise<bool> prom;
    std::string inforesult;
    auto fu = prom.get_future();
    response.then(
            [&](Pistache::Http::Response res) {
                inforesult = res.body();
                prom.set_value(true);
            },
            Pistache::Async::IgnoreException);
    fu.get();
    client.shutdown();
    GetZoneInfoRes zoneinfores;
    zoneinfores.deserialize(inforesult); //获取返回的结果
    if(zoneinfores.zoneInfoResult.empty()){
        return false;
    }
    zonemap_mutex.lock_shared();
    zonemap.clear();
    for(const auto &it : zoneinfores.zoneInfoResult){
        ZoneInfo zoneinfo;
        zoneinfo.deserialize(it);
        zonemap[zoneinfo.zoneName] = it;
        std::cout << it << std::endl;
    }
    zonemap_mutex.unlock_shared();
    return true;
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

