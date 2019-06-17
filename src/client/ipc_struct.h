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
        std::string zonename; // 空间名称
        std::string newzonename;
        std::string zoneuuid;
        std::string ownID; // 用户ID
        std::vector<std::string> memID;
        std::string newspacename; // "BUAABUAA";
        std::string spaceuuid;
        std::vector<std::string> spaceuuids;
        std::string spacename;
        std::vector<std::string> spacenames;
        int64_t newspacesize;
        int64_t spacesize;
        std::string spaceurl;
        //usermodel
        std::string accountName; //账户名
        std::string Password; //密码
        std::string hvsID;
        std::string email;
        std::string phone;
        std::string address;
        std::string department;

        //resource aggregation
        std::string storage_src_id;     // 存储资源UUID
        std::string storage_src_name;   // 存储资源名称
        std::string host_center_id;     // 存储资源所在超算中心UUID
        std::string host_center_name;   // 存储资源所在超算中心名称
        int64_t total_capacity;         // 存储资源空间容量大小
        std::string mgs_address;        // 存储资源MGS地址

    public:
        void serialize_impl() override {
            put("cmdname", cmdname);
            put("ip", ip);
            put("port", port);
            put("zonename", zonename);
            put("newzonename", newzonename);
            put("zoneuuid", zoneuuid);
            put("ownID", ownID);
            put("memID", memID);
            put("newspacename", newspacename);
            put("spaceuuid", spaceuuid);
            put("spaceuuids", spaceuuids);
            put("spacename", spacename);
            put("spacenames", spacenames);
            put("newspacesize", newspacesize);
            put("spacesize", spacesize);
            put("spaceurl", spaceurl);
            put("accountName", accountName);
            put("Password", Password);
            put("hvsID", hvsID);
            put("email",email);
            put("phone", phone);
            put("address", address);
            put("department", department);

            //resource aggregation
            put("storage_src_id", storage_src_id);
            put("storage_src_name", storage_src_name);
            put("host_center_id", host_center_id);
            put("host_center_name", host_center_name);
            put("total_capacity", total_capacity);
            put("mgs_address", mgs_address);
        };

        void deserialize_impl() override {
            get("cmdname", cmdname);
            get("ip", ip);
            get("port", port);
            get("zonename", zonename);
            get("newzonename", newzonename);
            get("zoneuuid", zoneuuid);
            get("ownID", ownID);
            get("memID", memID);
            get("newspacename", newspacename);
            get("spaceuuid", spaceuuid);
            get("spaceuuids", spaceuuids);
            get("spacename", spacename);
            get("spacenames", spacenames);
            get("newspacesize", newspacesize);
            get("spacesize", spacesize);
            get("spaceurl", spaceurl);   
            get("accountName", accountName);
            get("Password", Password);    
            get("hvsID", hvsID);
            get("email",email);
            get("phone", phone);
            get("address", address);
            get("department", department);

            //resource aggregation
            get("storage_src_id", storage_src_id);
            get("storage_src_name", storage_src_name);
            get("host_center_id", host_center_id);
            get("host_center_name", host_center_name);
            get("total_capacity", total_capacity);
            get("mgs_address", mgs_address);               
        };
    };
}

#endif //HVSONE_IPC_STRUCT_H
