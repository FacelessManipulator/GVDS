//
// Created by sy on 5/30/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include "hvs_struct.h"
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

// TODO: 添加的新头文件
#include "client/ipc_struct.h"
#include "common/ipc/IPCClient.h"
#include <errno.h>

using namespace hvs;

/*
 * zonerename 命令行客户端
 */



int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    char* demo1[11] = {const_cast<char *>("zonerename"), const_cast<char *>("--ip"), const_cast<char *>("192.168.5.222"),
                       const_cast<char *>("-p"), const_cast<char *>("49069"), const_cast<char *>("--zonename"),
                       const_cast<char *>("zonetest"), const_cast<char *>("--id"), const_cast<char *>("127"),
                       const_cast<char *>("-n"), const_cast<char *>("zonetest2")};
    char* demo2[2] = {const_cast<char *>("zonerename"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string ip ;//= "127.0.0.1";
    int port ;//= 55107;
    std::string zonename ;//= "syremotezone"; // 空间名称
    std::string newzonename;// = "BUAABUAA";
    std::string ownername;



    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = argv[0];//"zonerename";
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("区域查询模块");
        command.add_options()
            ("ownername,w", po::value<std::string>(), "主人账户名");  
        sp_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] =  [&](std::shared_ptr<po::variables_map> sp_variables_map)->void {
        if (sp_variables_map->count("ownername"))
        {
            ownername = (*sp_variables_map)["ownername"].as<std::string>();
        }
    };
    commandline.start(); //开始解析命令行参数

    try{
        std::promise<bool> prom;
        auto fu = prom.get_future();
        // TODO:  调用IPC 客户端 进行同行，并获取返回结果
        IPCClient ipcClient("127.0.0.1", GVDS_CLIENT_LISTENING_PORT());
        ipcClient.set_callback_func([&](IPCMessage msg)->void {
            // 客户端输出服务端发送来的消息
//            char tmp[IPCMessage::max_body_length] = {0};
//            std::memcpy(tmp, msg.body(), msg.body_length());
            std::string ipcresult (msg.body(), msg.body_length());
            if (ipcresult == "fail")
            {
                std::cout << "没有对应区域" << std::endl;
            }
            else
            {
                std::vector<Zone> res;
                json_decode(ipcresult, res);
                for(auto &it : res){
                    std::cout << "区域名称:" << it.zoneName << std::endl;
                    std::cout << "区域主人:" << it.ownerID << std::endl;
                    for(auto &m : it.memberID){
                        std::cout << "区域成员:" << m << std::endl;
                    }
                    for(auto s : it.spaceBicInfo){
                        std::cout << "空间名称:" << s->spaceName << std::endl;
                        std::cout << "空间容量:" << s->spaceSize << std::endl;
                        std::cout << "空间所在地:" << s->hostCenterName << std::endl;
                    }
                    std::cout << "=============================" << std::endl;
                }
            }

            prom.set_value(true);
        });
        ipcClient.run(); // 停止的时候调用stop 函数
        std::cout << "正在执行命令..." << std::endl;

        // TODO: 构造请求结构体，并发送；
        IPCreq ipcreq;
        ipcreq.cmdname = "zonelist_admin";
        ipcreq.ownName = ownername;





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