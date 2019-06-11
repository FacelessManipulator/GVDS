//
// Created by yaowen on 6/11/19.
// 北航系统结构所-存储组
//

#ifndef HVSONE_IPC_STRUCT_H
#define HVSONE_IPC_STRUCT_H

#include "common/JsonSerializer.h"

namespace hvs{

    //命令行参数信息，用于IPC通信的消息
    class CmdlineParameters : public hvs::JsonSerializer{
    public:
        int argc;
        std::vector<std::string> argv;

    public:
        void serialize_impl() override {
            put("argc", argc);
            put("argv", argv);
        };

        void deserialize_impl() override {
            get("argc", argc);
            get("argv", argv);
        };
    public:
        CmdlineParameters() = default;

        CmdlineParameters(int arc, char* arv[]){
            argc = arc;
            for (int i = 0; i < arc; i++){
                argv.emplace_back(arv[i]); // 进行初始化
            }
        }
    };

    class  IPCreq : public hvs::JsonSerializer{
    public:
        std::string cmdname;  //命令行具体执行的功能
        std::string ip ; // ip
        int port ;  // 端口号
        std::string zonename ; // 空间名称
        std::string ownID; // 用户ID
        std::string oldspacename; // "NewWorld";
        std::string newspacename; // "BUAABUAA";
        std::string spaceuuid;

    public:
        void serialize_impl() override {
            put("cmdname", cmdname);
            put("ip", ip);
            put("port", port);
            put("zonename", zonename);
            put("ownID", ownID);
            put("oldspacename", oldspacename);
            put("newspacename", newspacename);
            put("spaceuuid", spaceuuid);
        };

        void deserialize_impl() override {
            get("cmdname", cmdname);
            get("ip", ip);
            get("port", port);
            get("zonename", zonename);
            get("ownID", ownID);
            get("oldspacename", oldspacename);
            get("newspacename", newspacename);
            get("spaceuuid", spaceuuid);
        };
    };
}

#endif //HVSONE_IPC_STRUCT_H
