//
// Created by yaowen on 6/11/19.
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
 * spacerename 命令行客户端
 */

int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    // ./spacerename_ipc --ip 192.168.5.222 -p 43107 --zonename syremotezone --id 202 -o BIGBOSSSY -n BUAABUAA
    char* demo1[13] = {const_cast<char *>("spacerename"), const_cast<char *>("--ip"), const_cast<char *>("127.0.0.1"),
                       const_cast<char *>("-p"), const_cast<char *>("9090"), const_cast<char *>("--zonename"),
                       const_cast<char *>("zonetest"), const_cast<char *>("--id"), const_cast<char *>("127"),
                       const_cast<char *>("-o"), const_cast<char *>("spacetest"), const_cast<char *>("-n"),
                       const_cast<char *>("spacetest2")}; //BIGBOSSSY
    char* demo2[2] = {const_cast<char *>("spacerename"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据

    std::string zonename ;//= "syremotezone"; // 空间名称
    std::vector<std::string> spacenames;// = "NewWorld";


    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(13, demo1); // TODO 命令行赋值
    std::string cmdname = argv[0];
//    std::string cmdname = demo1[0]; // TODO 命令名字
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("空间已用容量查询模块");
        command.add_options()
                ("zonename,z", po::value<std::string>(), "区域名称")
                ("spacename,s", po::value<std::vector<std::string>>(), "空间名称")
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
            spacenames = (*sp_variables_map)["spacename"].as<std::vector<std::string>>();
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
            if (ipcresult.substr(0, 1) != "1"){
                //std::cerr << "执行失败，请检查命令参数是否正确！详情请查看日志！" << std::endl;
                std::cerr << ipcresult << std::endl; // 执行结果
            } else {
                std::vector<int64_t> result;
                std::string ipcres = ipcresult.substr(1, ipcresult.length() - 1);
                json_decode(ipcres, result);
                for(int i = 0; i < spacenames.size(); i++)
                {
                    std::cout << "空间名称：" << spacenames[i] << "空间已用容量：" << result[i] <<std::endl;
                }
            }
            prom.set_value(true);
        });
        ipcClient.run(); // 停止的时候调用stop 函数
        std::cout << "正在执行命令..." << std::endl;

        // TODO: 构造请求结构体，并发送；
        IPCreq ipcreq;
        ipcreq.cmdname = "spaceusage";
        ipcreq.zonename = zonename; // 空间名称
        ipcreq.spacenames = spacenames; // "NewWorld";

        // TODO: 发送
        auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
        ipcClient.write(*msg); // 传递一个消息；
        fu.get(); // TODO: 等待客户端返回结果
        ipcClient.stop();

    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}