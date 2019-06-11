//
// Created by yaowen on 6/11/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include "manager/space/Space.h"
#include "manager/zone/Zone.h"
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

// TODO: 添加的新头文件
#include "client/ipc_struct.h"
#include "ipc/IPCClient.h"

using namespace hvs;

/*
 * spacerename 命令行客户端
 */

int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    // ./spacerename_ipc --ip 192.168.5.222 -p 43107 --zonename syremotezone --id 202 -o BIGBOSSSY -n BUAABUAA
    char* demo1[13] = {const_cast<char *>("spacerename"), const_cast<char *>("--ip"), const_cast<char *>("192.168.5.222"),
                       const_cast<char *>("-p"), const_cast<char *>("43107"), const_cast<char *>("--zonename"),
                       const_cast<char *>("syremotezone"), const_cast<char *>("--id"), const_cast<char *>("202"),
                       const_cast<char *>("-o"), const_cast<char *>("BIGBOSSSY"), const_cast<char *>("-n"),
                       const_cast<char *>("BUAABUAA")};
    char* demo2[2] = {const_cast<char *>("spacerename"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string ip ;//= "127.0.0.1";
    int port ;//= 55107;
    std::string zonename ;//= "syremotezone"; // 空间名称
    std::string ownID;// = "202"; // 用户ID
    std::string oldspacename;// = "NewWorld";
    std::string newspacename;// = "BUAABUAA";
    std::string spaceuuid;

    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(13, demo1); // TODO 命令行赋值
    std::string cmdname = argv[0];
//    std::string cmdname = demo1[0]; // TODO 命令名字
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("空间重命名模块");
        command.add_options()
                ("ip", po::value<std::string>(), "管理节点IP")
                ("port,p", po::value<int>(), "管理节点端口号")
                ("zonename", po::value<std::string>(), "区域名称")
                ("id", po::value<std::string>(), "管理员ID")
                ("oldname,o", po::value<std::string>(), "空间旧名称")
                ("newname,n", po::value<std::string>(), "空间新名称")
                ;
        sp_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] =  [&](std::shared_ptr<po::variables_map> sp_variables_map)->void {
        if (sp_variables_map->count("ip"))
        {
            ip = (*sp_variables_map)["ip"].as<std::string>();
        }
        if (sp_variables_map->count("port"))
        {
            port = (*sp_variables_map)["port"].as<int>();
        }
        if (sp_variables_map->count("zonename"))
        {
            zonename = (*sp_variables_map)["zonename"].as<std::string>();
        }
        if (sp_variables_map->count("id"))
        {
            ownID = (*sp_variables_map)["id"].as<std::string>();
        }
        if (sp_variables_map->count("oldname"))
        {
            oldspacename = (*sp_variables_map)["oldname"].as<std::string>();
        }
        if (sp_variables_map->count("newname"))
        {
            newspacename = (*sp_variables_map)["newname"].as<std::string>();
        }
    };
    commandline.start(); //开始解析命令行参数

    //TODO :判断是否有参数，如果没有，则报错
    if (commandline.argc <= 1 || ip.empty() ) {
        std::cerr << "请输入命令参数！" << std::endl;
        commandline.print_options();
        exit(-1);
    }

    try{
        // TODO:  调用IPC 客户端 进行同行，并获取返回结果
        IPCClient ipcClient("127.0.0.1", 6666);
        ipcClient.set_callback_func([&](IPCMessage msg)->void {
            // 客户端输出服务端发送来的消息
//            char tmp[IPCMessage::max_body_length] = {0};
//            std::memcpy(tmp, msg.body(), msg.body_length());
            std::string ipcresult (msg.body(), msg.body_length());
            if (ipcresult != "success"){
                //std::cerr << "执行失败，请检查命令参数是否正确！详情请查看日志！" << std::endl;
                std::cerr << ipcresult << std::endl; // 执行结果
            } else {
                std::cout << "执行结果：" << ipcresult << std::endl;
            }
        });
        ipcClient.run(); // 停止的时候调用stop 函数
        std::cout << "正在执行命令..." << std::endl;

        // TODO: 构造请求结构体，并发送；
        IPCreq ipcreq;
        ipcreq.cmdname = "spacerename";
        ipcreq.ip = ip ; // ip
        ipcreq.port = port;  // 端口号
        ipcreq.zonename = zonename; // 空间名称
        ipcreq.ownID = ownID; // 用户ID
        ipcreq.oldspacename = oldspacename; // "NewWorld";
        ipcreq.newspacename = newspacename; // "BUAABUAA";
        ipcreq.spaceuuid = spaceuuid; // uuid

        // TODO: 发送
        auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
        ipcClient.write(*msg); // 传递一个消息；
        sleep(1); // TODO: 等待客户端返回结果
        ipcClient.stop();

    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}