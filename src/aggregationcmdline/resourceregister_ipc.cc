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

/*
 * resourceregister 命令行客户端
 */

int main(int argc, char* argv[]){
    
    char* demo1[13] = {const_cast<char *>("resourceregister"), const_cast<char *>("--ri"), const_cast<char *>("resource_0001"),
                       const_cast<char *>("--rn"), const_cast<char *>("lustre_0001"), 
                       const_cast<char *>("--ci"), const_cast<char *>("centerid_0001"),
                       const_cast<char *>("--cn"), const_cast<char *>("zhongkeyuan"),
                       const_cast<char *>("--tc"), const_cast<char *>("1000"),
                       const_cast<char *>("--mgs"), const_cast<char *>("http://192.168.5.119")
                       }; //BIGBOSSSY

    char* demo2[2] = {const_cast<char *>("resourceregister"), const_cast<char *>("--help")};

    std::string storage_src_id = "";            // 存储资源UUID
    std::string storage_src_name = "";   // 存储资源名称
    std::string host_center_id = "";             // 存储资源所在超算中心UUID
    std::string host_center_name = "";     // 存储资源所在超算中心名称
    int64_t total_capacity = 0;                // 存储资源空间容量大小
    std::string mgs_address = ""; // 存储资源MGS地址



    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(13, demo1); // TODO 命令行赋值
    std::string cmdname = argv[0];
//    std::string cmdname = demo1[0]; // TODO 命令名字
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> res_cmdline_options)->void {
        po::options_description command("资源注册模块");
        command.add_options()
                ("ri", po::value<std::string>(), "存储资源UUID")
                ("rn", po::value<std::string>(), "存储资源名称")
                ("ci", po::value<std::string>(), "超算中心UUID")
                ("cn", po::value<std::string>(), "超算中心名称")
                ("tc", po::value<int64_t>(), "存储资源空间容量大小")
                ("mgs", po::value<std::string>(), "资源的mgs地址")
                ;
        res_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] =  [&](std::shared_ptr<po::variables_map> res_variables_map)->void {
        if (res_variables_map->count("ri"))
        {
            storage_src_id = (*res_variables_map)["ri"].as<std::string>();
        }
        if (res_variables_map->count("rn"))
        {
            storage_src_name = (*res_variables_map)["rn"].as<std::string>();
        }
        if (res_variables_map->count("ci"))
        {
            host_center_id = (*res_variables_map)["ci"].as<std::string>();
        }
        if (res_variables_map->count("cn"))
        {
            host_center_name = (*res_variables_map)["cn"].as<std::string>();
        }
        if (res_variables_map->count("tc"))
        {
            total_capacity = (*res_variables_map)["tc"].as<int64_t>();
        }
        if (res_variables_map->count("mgs"))
        {
            mgs_address = (*res_variables_map)["mgs"].as<std::string>();
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
        ipcreq.cmdname = "resourceregister";
        ipcreq.storage_src_id = storage_src_id ; 
        ipcreq.storage_src_name = storage_src_name;  
        ipcreq.host_center_id = host_center_id; 
        ipcreq.host_center_name = host_center_name; 
        ipcreq.total_capacity = total_capacity; 
        ipcreq.mgs_address = mgs_address; 

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