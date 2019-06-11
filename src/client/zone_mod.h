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
    public:
        void check();
        ClientZone (const char* name, Client* cli) : ClientModule(name, cli), m_stop(true) {
            isThread = false;
        }
        //区域信息检索前端模块
        bool GetZoneInfo(std::string clientID); // 区域信息检索前端模块，返回区域信息；
        std::unordered_map<std::string, std::string> zonemap;
        std::tuple<std::string, std::string, std::string, std::string> locatePosition(const std::string path);
    private:
        friend class Client;
        bool m_stop;
    };
}
//HVSONE_ZONE_MOD_H
