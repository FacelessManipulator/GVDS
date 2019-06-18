#include <iostream>
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

// TODO: 添加的新头文件
// #include "client/clientuser/ClientUser_struct.h"
#include "ipc/IPCClient.h"
#include "client/ipc_struct.h"



using namespace Pistache;
using namespace hvs;
//bool GetZoneInfo(std::string ip, int port, std::string clientID);
/*
 * zonerename 命令行客户端
 */
//std::unordered_map<std::string, std::string> zonemap;


int main(int argc, char* argv[]){

 //权限查询
    char* demo1[11] = {const_cast<char *>("authmodify_ipc"), 
                       const_cast<char *>("--ip"), const_cast<char *>("127.0.0.1"),
                       const_cast<char *>("-p"), const_cast<char *>("9090"), 
                       const_cast<char *>("--user"), const_cast<char *>("lbq-7"),
                       const_cast<char *>("--zonename"), const_cast<char *>("zone-auth-test1"),
                       const_cast<char *>("--changeauth"), const_cast<char *>("0")};
    char* demo2[2] = {const_cast<char *>("authmodify_ipc"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string ip ;//= "127.0.0.1";
    int port ;//= 55107;
    std::string username;//= "lbq-7";
    std::string zonename;
    std::string changeauth;


    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = argv[0];
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("账户登录");
        command.add_options()
                ("ip", po::value<std::string>(), "管理节点IP")
                ("port,p", po::value<int>(), "管理节点端口号")
                ("user,u", po::value<std::string>(), "账户名")
                ("zonename", po::value<std::string>(), "区域名")
                ("changeauth", po::value<std::string>(), "设置组权限")
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
        if (sp_variables_map->count("user"))
        {
            username = (*sp_variables_map)["user"].as<std::string>();
        }
        if (sp_variables_map->count("zonename"))
        {
            zonename = (*sp_variables_map)["zonename"].as<std::string>();
        }
        if (sp_variables_map->count("changeauth"))
        {
            changeauth = (*sp_variables_map)["changeauth"].as<std::string>();
        }
    };
    commandline.start(); //开始解析命令行参数

    //TODO :判断是否有参数，如果没有，则报错
    if (commandline.argc <= 1 ) {
        std::cerr << "请输入命令参数！" << std::endl;
        commandline.print_options();
        exit(-1);
    }

    
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
        ipcreq.cmdname = "authmodify";
        ipcreq.ip = ip ; // ip
        ipcreq.port = port;  // 端口号

        ipcreq.accountName = username; //账户名
        ipcreq.zonename = zonename;
        ipcreq.changeauth = changeauth;
    
        

        // TODO: 发送
        auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
        ipcClient.write(*msg); // 传递一个消息；
        sleep(1); // TODO: 等待客户端返回结果
        ipcClient.stop();

    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

}