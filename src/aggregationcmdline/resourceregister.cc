//
// Created by weibing on 6/16/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include <hvs_struct.h>
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

// TODO: 添加的新头文件
#include "client/ipc_struct.h"
#include "ipc/IPCClient.h"

using namespace hvs;
using namespace std;

/*
 * resourceregister 命令行客户端
 */

int main(int argc, char *argv[])
{
    string cmdtitle = "resourceregister";
    char *demo1[17] = {const_cast<char *>(cmdtitle.c_str()), const_cast<char *>("--ri"), const_cast<char *>("resource_0001"),
                       const_cast<char *>("--rn"), const_cast<char *>("lustre_0001"),
                       const_cast<char *>("--ci"), const_cast<char *>("centerid_0001"),
                       const_cast<char *>("--cn"), const_cast<char *>("zhongkeyuan"),
                       const_cast<char *>("--tc"), const_cast<char *>("1000"),
                       const_cast<char *>("--ac"), const_cast<char *>("0"),
                       const_cast<char *>("--mgs"), const_cast<char *>("192.168.5.119"),
                       const_cast<char *>("--st"), const_cast<char *>("1")}; //BIGBOSSSY

    char *demo2[2] = {const_cast<char *>(cmdtitle.c_str()), const_cast<char *>("--help")};

    string storage_src_id = "#default";   // 存储资源UUID
    string storage_src_name = "#default"; // 存储资源名称
    string host_center_id = "#default";   // 存储资源所在超算中心UUID
    string host_center_name = "#default"; // 存储资源所在超算中心名称
    int64_t total_capacity = -1;          // 存储资源空间容量大小
    int64_t assign_capacity = -1;         // 存储资源已分配空间容量大小
    string mgs_address = "#default";      // 存储资源MGS地址
    int state = -1;                       // 存储资源状态

    //检查非法字符输入
    for (int i = 1; i < argc; i++)
    {
        if (!CmdLineProxy::is_validate(argv[i]))
        {
            cerr << "输入非法字符" << endl;
            return 0;
        }
    }

    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
    string cmdname = argv[0];
    commandline.cmd_desc_func_map[cmdname] = [](shared_ptr<po::options_description> res_cmdline_options) -> void {
        po::options_description command("资源注册模块");
        command.add_options()("ri", po::value<string>(), "存储资源UUID")("rn", po::value<string>(), "存储资源名称")("ci", po::value<string>(), "超算中心UUID")("cn", po::value<string>(), "超算中心名称")("tc", po::value<int64_t>(), "存储资源空间容量大小(MB)")("ac", po::value<int64_t>(), "已分配存储资源空间容量大小(MB)")("mgs", po::value<string>(), "资源mgs地址")("st", po::value<int>(), "资源状态,0:初始化中 1:正常使用 2:负载过载 3:注销退出中 4:已经退出");
        res_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] = [&](shared_ptr<po::variables_map> res_variables_map) -> void {
        if (res_variables_map->count("ri"))
            storage_src_id = (*res_variables_map)["ri"].as<string>();
        if (res_variables_map->count("rn"))
            storage_src_name = (*res_variables_map)["rn"].as<string>();
        if (res_variables_map->count("ci"))
            host_center_id = (*res_variables_map)["ci"].as<string>();
        if (res_variables_map->count("cn"))
            host_center_name = (*res_variables_map)["cn"].as<string>();
        if (res_variables_map->count("tc"))
            total_capacity = (*res_variables_map)["tc"].as<int64_t>();
        if (res_variables_map->count("ac"))
            assign_capacity = (*res_variables_map)["ac"].as<int64_t>();
        if (res_variables_map->count("mgs"))
            mgs_address = (*res_variables_map)["mgs"].as<string>();
        if (res_variables_map->count("st"))
            state = (*res_variables_map)["st"].as<int>();
    };
    commandline.start();
    if (commandline.argc <= 1)
    {
        cerr << "请输入命令参数！" << endl;
        commandline.print_options();
        exit(-1);
    }

    try
    {
        promise<bool> prom;
        auto fu = prom.get_future();
        //调用IPC 客户端 进行同行，并获取返回结果
        IPCClient ipcClient("127.0.0.1", GVDS_CLIENT_LISTENING_PORT());
        ipcClient.set_callback_func([&](IPCMessage msg) -> void {
            string ipcresult(msg.body(), msg.body_length());

            if (ipcresult == "")
                cout << "resource register fail" << endl;
            else
                cout << ipcresult << endl;

            prom.set_value(true);
        });
        ipcClient.run(); //停止的时候调用stop 函数
        cout << "正在执行命令..." << endl;

        //构造请求结构体，并发送
        IPCreq ipcreq;
        ipcreq.cmdname = cmdtitle;
        ipcreq.storage_src_id = storage_src_id;
        ipcreq.storage_src_name = storage_src_name;
        ipcreq.host_center_id = host_center_id;
        ipcreq.host_center_name = host_center_name;
        ipcreq.total_capacity = total_capacity;
        ipcreq.assign_capacity = assign_capacity;
        ipcreq.mgs_address = mgs_address;
        ipcreq.state = state;

        if (!(ipcreq.storage_src_id[0] != '#' && ipcreq.storage_src_name[0] != '#' &&
              (ipcreq.host_center_id[0] != '#' || ipcreq.host_center_name[0] != '#') && //输入超算中心id或name皆可，不过若两者皆有，则二者需指向同一超算中心
              ipcreq.total_capacity != -1 &&
              ipcreq.assign_capacity != -1 &&
              ipcreq.mgs_address[0] != '#' &&
              ipcreq.state != -1))
        {
            cerr << "请完整的命令参数！" << endl;
            commandline.print_options();
            exit(-1);
        }

        //发送
        auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
        ipcClient.write(*msg); //传递一个消息

        //等待客户端返回结果，超时则终止请求
        auto status = fu.wait_for(std::chrono::seconds(3));
        if (status == std::future_status::timeout)
        {
            cerr << "操作失败，请求超时！" << endl;
        }
        ipcClient.stop();
    }
    catch (exception &e)
    {
        cout << e.what() << endl;
    }
    return 0;
}