//
// Created by yaowen on 5/28/19.
// 北航系统结构所-存储组
//


#pragma once //HVSONE_ZONE_MOD_H

#include "client.h"

// 主要包含区域信息检索前端模块和区域定位模块

namespace hvs {
    class ClientZone : public ClientModule {
    private:
        virtual void start() override;
        virtual void stop() override;

    private:
        //std::shared_mutex graph_mutex;
    public:
        ClientZone (const char* name, Client* cli) : ClientModule(name, cli) {
            isThread = false;
            ownID = "101";
        }
        //区域信息检索前端模块
        std::vector<std::string> GetZoneInfo(std::string clientID); // 区域信息检索前端模块，返回区域信息；

    private:
        //临时变量 TODO：结合用户模块，获取账户信息
        std::string ownID;
        friend class Client;
    };
}
//HVSONE_ZONE_MOD_H
