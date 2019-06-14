//
// Created by yaowen on 5/28/19.
// 北航系统结构所-存储组
//


#pragma once //HVSONE_ZONE_MOD_H

#include "client.h"
#include "zone_struct.h"
#include <pistache/client.h>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <tuple>
#include <thread>
#include <unistd.h>

// 主要包含区域信息检索前端模块和区域定位模块

namespace hvs {
    class ClientZone : public ClientModule, public Thread {
    private:
        virtual void start() override;
        virtual void stop() override;
        void* entry() override;

    private:
        std::shared_mutex zonemap_mutex;
        std::shared_mutex spacemap_mutex;
    public:
        void check();
        ClientZone (const char* name, Client* cli) : ClientModule(name, cli), m_stop(true) {
            isThread = false;
        }
        //区域信息检索前端模块
        bool GetZoneInfo(std::string clientID); // 区域信息检索前端模块，返回区域信息；
        bool GetLocateInfo(std::string clientID, std::string zoneID, std::vector<std::string> &spaceIDs); // 空间位置信息检索前端模块，用来获取空间位置；
        std::string spaceuuid_to_spacerpath(std::string uuid); // TODO: 直接通过uuid，获取到空间远程路径;
        std::string spaceuuid_to_hostcenterID(std::string uuid); // TODO: 直接通过uuid，获取负责传输远程数据中心;
        std::unordered_map<std::string, std::string> zonemap;
        std::tuple<std::string, std::string, std::string, std::string> locatePosition(const std::string path);
        std::unordered_map<std::string, std::string> spaceuuid_to_metadatamap;

    private:
        friend class Client;
        bool m_stop;
    };
}
//HVSONE_ZONE_MOD_H
