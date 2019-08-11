//
// Created by sy on 5/30/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

// TODO: 添加的新头文件
#include "client/clientuser/ClientUser_struct.h"
#include "ipc/IPCClient.h"
#include "client/ipc_struct.h"



using namespace Pistache;
using namespace hvs;
using namespace std;
//bool GetZoneInfo(std::string ip, int port, std::string clientID);
/*
 * zonerename 命令行客户端
 */
//std::unordered_map<std::string, std::string> zonemap;


int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    // 1、用户登录
    char* demo1[17] = {const_cast<char *>("adminsignup"), 
                       const_cast<char *>("--user"), const_cast<char *>("lbq-9"),
                       const_cast<char *>("--pass"), const_cast<char *>("123456"),
                       const_cast<char *>("-e"), const_cast<char *>("XXXXXX@163.com"),
                       const_cast<char *>("--phone"), const_cast<char *>("15012349876"),
                       const_cast<char *>("--ad"), const_cast<char *>("xueyuanlu_xinzhulou"),
                       const_cast<char *>("--de"), const_cast<char *>("Beihang")};
    char* demo2[2] = {const_cast<char *>("adminsignup"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据

    std::string username;//= "lbq-7";
    std::string ownerid; // = "123456"; // 用户ID
    std::string pass;
    std::string email;
    std::string phone;
    std::string ad;
    std::string de;

    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = argv[0];
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("账户注册");
        command.add_options()
                ("user,u", po::value<std::string>(), "账户名")
                ("pass", po::value<std::string>(), "密码")
                ("email,e", po::value<std::string>(), "邮箱")
                ("phone", po::value<std::string>(), "电话")
                ("ad", po::value<std::string>(), "地址")
                ("de", po::value<std::string>(), "部门")
                ;
        sp_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] =  [&](std::shared_ptr<po::variables_map> sp_variables_map)->void {
        if (sp_variables_map->count("user"))
        {
            username = (*sp_variables_map)["user"].as<std::string>();
        }
        if (sp_variables_map->count("pass"))
        {
            pass = (*sp_variables_map)["pass"].as<std::string>();
        }
        if (sp_variables_map->count("email"))
        {
            email = (*sp_variables_map)["email"].as<std::string>();
        }
        if (sp_variables_map->count("phone"))
        {
            phone = (*sp_variables_map)["phone"].as<std::string>();
        }
        if (sp_variables_map->count("ad"))
        {
            ad = (*sp_variables_map)["ad"].as<std::string>();
        }
        if (sp_variables_map->count("de"))
        {
            de = (*sp_variables_map)["de"].as<std::string>();
        }
        
    };
    commandline.start(); //开始解析命令行参数

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
            std::string ipcresult (msg.body(), msg.body_length());
            // if(ipcresult == "0"){
            //     std::cout << "请求成功" << std::endl;
            // }
            // else{  //"11"
            //     std::cout << "请求失败，请再尝试" << std::endl;
            // }
            std::cout << ipcresult << std::endl;
            prom.set_value(true);
        });
        ipcClient.run(); // 停止的时候调用stop 函数
        std::cout << "正在执行命令..." << std::endl;

         // TODO: 构造请求结构体，并发送；
        IPCreq ipcreq;
        ipcreq.cmdname = "adminsignup";  // 这块要和if 一致

        ipcreq.accountName = username; //账户名
        ipcreq.hvsID = "1"; //在服务端产生，并覆盖此值;
        ipcreq.Password = pass;
        ipcreq.email = email;
        ipcreq.phone = phone;
        ipcreq.address = ad;
        ipcreq.department = de;

        // TODO: 发送
        auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
        ipcClient.write(*msg); // 传递一个消息；
        fu.get();              // 等待客户端返回结果
        // sleep(1); // TODO: 等待客户端返回结果
        ipcClient.stop();

    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}