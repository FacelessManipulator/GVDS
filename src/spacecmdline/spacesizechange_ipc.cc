//
// Created by yaowen on 5/29/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include "hvs_struct.h"
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

// TODO: 添加的新头文件
#include "client/ipc_struct.h"
#include "ipc/IPCClient.h"

using namespace hvs;
/*
 * spacesizechange 命令行客户端
 */



int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    char* demo1[13] = {const_cast<char *>("spacesizechange"), const_cast<char *>("--ip"), const_cast<char *>("192.168.10.219"),
                       const_cast<char *>("-p"), const_cast<char *>("51867"), const_cast<char *>("--zonename"),
                       const_cast<char *>("compute-zone2"), const_cast<char *>("--id"), const_cast<char *>("000"),
                       const_cast<char *>("--spacename"), const_cast<char *>("compute4"), const_cast<char *>("-n"),
                       const_cast<char *>("100")};
    char* demo2[2] = {const_cast<char *>("spacesizechange"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string zonename ;//= "syremotezone"; // 空间名称
    std::string spacename;// = "NewWorld";
    int64_t newspacesize;// = "BUAABUAA";

    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = argv[0];
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("空间缩放模块");
        command.add_options()
                ("zonename,z", po::value<std::string>(), "区域名称")
                ("spacename,s", po::value<std::string>(), "空间名称")
                ("newsize,n", po::value<int64_t>(), "空间新容量")
                ;
        sp_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] =  [&](std::shared_ptr<po::variables_map> sp_variables_map)->void {
        if (sp_variables_map->count("zonename"))
        {
            zonename = (*sp_variables_map)["zonename"].as<std::string>();
        }
        if (sp_variables_map->count("spacename"))
        {
            spacename = (*sp_variables_map)["spacename"].as<std::string>();
        }
        if (sp_variables_map->count("newsize"))
        {
            newspacesize = (*sp_variables_map)["newsize"].as<int64_t>();
        }
    };
    commandline.start(); //开始解析命令行参数

    //TODO :判断是否有参数，如果没有，则报错
    if (commandline.argc <= 1) {
        std::cerr << "请输入命令参数！" << std::endl;
        commandline.print_options();
        exit(-1);
    }

    try{
        std::promise<bool> prom;
        auto fu = prom.get_future();
        // TODO:  调用IPC 客户端 进行同行，并获取返回结果
        IPCClient ipcClient("127.0.0.1", 6666);
        ipcClient.set_callback_func([&](IPCMessage msg)->void {
            // 客户端输出服务端发送来的消息
//            char tmp[IPCMessage::max_body_length] = {0};
//            std::memcpy(tmp, msg.body(), msg.body_length());
            std::string ipcresult (msg.body(), msg.body_length());
            std::cout << ipcresult << std::endl;
            prom.set_value(true);
        });
        ipcClient.run(); // 停止的时候调用stop 函数
        std::cout << "正在执行命令..." << std::endl;

        // TODO: 构造请求结构体，并发送；
        IPCreq ipcreq;
        ipcreq.cmdname = "spacesizechange";
        ipcreq.zonename = zonename; // 空间名称
        ipcreq.spacename = spacename; // "NewWorld";
        ipcreq.newspacesize = newspacesize; //

        // TODO: 发送
        auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
        ipcClient.write(*msg); // 传递一个消息；
        // TODO: 添加延迟，防止命令长时间等待
        auto status = fu.wait_for(std::chrono::seconds(20));
        if(status == std::future_status::timeout){
            std::cout << "命令行执行20s，超时；请确认当前fuse client进程正在运行！" << std::endl;
            exit(-1);
        }else if(status == std::future_status::ready){
            ipcClient.stop();
        }

    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}