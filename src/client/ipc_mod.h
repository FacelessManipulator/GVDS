//
// Created by yaowen on 6/11/19.
// 北航系统结构所-存储组
//


#pragma once //HVSONE_IPC_MOD_H
#include "client.h"
#include "ipc/IPCServer.hpp"
#include <hvs_struct.h>
#include "ipc_struct.h"
#include "client/clientuser/ClientUser_struct.h"
#include "client/clientuser/ClientAuth_struct.h"
#include <future>
#include <pistache/client.h>


namespace hvs{
    class ClientIPC :public ClientModule{
    private:
        virtual void start() override;
        virtual void stop() override;

    public:
        ClientIPC (const char* name, Client* cli) : ClientModule(name, cli) {
            isThread = true;
            init();
        }

        void init();

    private:
        friend class Client;
        std::shared_ptr<IPCServer> sp_ipcserver;
        std::unordered_map<std::string, Zone> zonemap; // 区域重命名使用
    

    private:
        // TODO: 客户端具体处理函数
        std::string dospacerename(IPCreq &ipcreq); // 处理客户端函数
        std::string dospacesizechange(IPCreq &ipcreq);
        std::string domapadd(IPCreq &ipcreq);
        std::string domapdeduct(IPCreq &ipcreq);
        std::string dozoneadd(IPCreq &ipcreq);
        std::string dozonecancel(IPCreq &ipcreq);
        std::string dozoneregister(IPCreq &ipcreq);
        std::string dozonerename(IPCreq &ipcreq);
        std::string dozoneshare(IPCreq &ipcreq);
        std::string dozonesharecancel(IPCreq &ipcreq);
        bool GetZoneInfo(std::string clientID);


        //资源聚合相关函数
        std::string doresourceregister(IPCreq &ipcreq);  //资源注册
        std::string doresourcedelete(IPCreq &ipcreq);    //资源删除
        std::string doresourcequery(IPCreq &ipcreq);     //资源查询
        std::string doresourceupdate(IPCreq &ipcreq);    //资源更新


        //user
        std::string douserlogin(IPCreq &ipcreq);
        std::string dousersearch(IPCreq &ipcreq);
        std::string dousersignup(IPCreq &ipcreq);
        std::string dousermodify(IPCreq &ipcreq);
        std::string douserexit(IPCreq &ipcreq);

        std::string doauthsearch(IPCreq &ipcreq);
        std::string doauthmodify(IPCreq &ipcreq);
    };
}

//HVSONE_IPC_MOD_H
