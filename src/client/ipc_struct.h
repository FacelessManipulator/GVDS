//
// Created by yaowen on 6/11/19.
// 北航系统结构所-存储组
//

#ifndef GVDS_IPC_STRUCT_H
#define GVDS_IPC_STRUCT_H

#include "common/JsonSerializer.h"

namespace gvds
{

//命令行参数信息，用于IPC通信的消息
class CmdlineParameters : public gvds::JsonSerializer
{
public:
    int argc;
    std::vector<std::string> argv;

public:
    void serialize_impl() override
    {
        put("argc", argc);
        put("argv", argv);
    };

    void deserialize_impl() override
    {
        get("argc", argc);
        get("argv", argv);
    };

public:
    CmdlineParameters() = default;

    CmdlineParameters(int arc, char *arv[])
    {
        argc = arc;
        for (int i = 0; i < arc; i++)
        {
            argv.emplace_back(arv[i]); // 进行初始化
        }
    }
};

class IPCreq : public gvds::JsonSerializer
{
public:
    std::string cmdname;  //命令行具体执行的功能
    std::string ip;       // ip
    int port;             // 端口号
    std::string zonename; // 空间名称
    std::string newzonename;
    //std::string zoneuuid;
    std::string ownName; // 用户
    //std::vector<std::string> memID;
    std::vector<std::string> memName;
    std::string newspacename; // "BUAABUAA";
    //std::string spaceuuid;
    //std::vector<std::string> spaceuuids;
    std::string spacename;
    std::vector<std::string> spacenames;
    int64_t newspacesize;
    int64_t spacesize;
    std::string spaceurl;
    //usermodel
    std::string accountName; //账户名
    std::string Password;    //密码
    std::string gvdsID;
    std::string email;
    std::string phone;
    std::string address;
    std::string department;

    std::string changeauth;
    //mconf
    std::string centerID;
    std::string centerIP;
    std::string centerPort;
    std::string centerName;

    //admin
    std::string sug;     // 审批意见类型
    std::string applyid; // id

    //resource aggregation
    std::string storage_src_id = "#default";   // 存储资源UUID
    std::string storage_src_name = "#default"; // 存储资源名称
    std::string host_center_id = "#default";   // 存储资源所在超算中心UUID
    std::string host_center_name = "#default"; // 存储资源所在超算中心名称
    int64_t total_capacity = -1;               // 存储资源空间容量大小
    int64_t assign_capacity = -1;              // 已经分配容量
    std::string mgs_address = "#default";      // 存储资源MGS地址
    int state = -1;                            // 存储资源状态

public:
    void serialize_impl() override
    {
        put("cmdname", cmdname);
        put("ip", ip);
        put("port", port);
        put("zonename", zonename);
        put("newzonename", newzonename);
        //put("zoneuuid", zoneuuid);
        put("ownName", ownName);
        //put("memID", memID);
        put("memName", memName);
        put("newspacename", newspacename);
        //put("spaceuuid", spaceuuid);
        //put("spaceuuids", spaceuuids);
        put("spacename", spacename);
        put("spacenames", spacenames);
        put("newspacesize", newspacesize);
        put("spacesize", spacesize);
        put("spaceurl", spaceurl);
        put("accountName", accountName);
        put("Password", Password);
        put("gvdsID", gvdsID);
        put("email", email);
        put("phone", phone);
        put("address", address);
        put("department", department);
        put("changeauth", changeauth);

        put("centerID", centerID);
        put("centerIP", centerIP);
        put("centerPort", centerPort);
        put("centerName", centerName);

        put("sug", sug);
        put("applyid", applyid);

        //resource aggregation
        put("storage_src_id", storage_src_id);
        put("storage_src_name", storage_src_name);
        put("host_center_id", host_center_id);
        put("host_center_name", host_center_name);
        put("total_capacity", total_capacity);
        put("assign_capacity", assign_capacity);
        put("mgs_address", mgs_address);
        put("state", state);
    };

    void deserialize_impl() override
    {
        get("cmdname", cmdname);
        get("ip", ip);
        get("port", port);
        get("zonename", zonename);
        get("newzonename", newzonename);
        //get("zoneuuid", zoneuuid);
        get("ownName", ownName);
        //get("memID", memID);
        get("memName", memName);
        get("newspacename", newspacename);
        //get("spaceuuid", spaceuuid);
        //get("spaceuuids", spaceuuids);
        get("spacename", spacename);
        get("spacenames", spacenames);
        get("newspacesize", newspacesize);
        get("spacesize", spacesize);
        get("spaceurl", spaceurl);
        get("accountName", accountName);
        get("Password", Password);
        get("gvdsID", gvdsID);
        get("email", email);
        get("phone", phone);
        get("address", address);
        get("department", department);
        get("changeauth", changeauth);

        get("centerID", centerID);
        get("centerIP", centerIP);
        get("centerPort", centerPort);
        get("centerName", centerName);

        get("sug", sug);
        get("applyid", applyid);

        //resource aggregation
        get("storage_src_id", storage_src_id);
        get("storage_src_name", storage_src_name);
        get("host_center_id", host_center_id);
        get("host_center_name", host_center_name);
        get("total_capacity", total_capacity);
        get("assign_capacity", assign_capacity);
        get("mgs_address", mgs_address);
        get("state", state);
    };
};
} // namespace gvds

#endif //GVDS_IPC_STRUCT_H
