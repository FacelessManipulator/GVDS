//
// Created by yaowen on 6/11/19.
// 北航系统结构所-存储组
//


#pragma once //HVSONE_IPC_MOD_H
#include "client.h"
#include "ipc/IPCMessage.hpp"


namespace hvs{
    class ClientIPC :public ClientModule{
    private:
        virtual void start() override;
        virtual void stop() override;

    public:
        ClientIPC (const char* name, Client* cli) : ClientModule(name, cli) {
            isThread = true;
        }

    private:
        friend class Client;
    };
}

//HVSONE_IPC_MOD_H
