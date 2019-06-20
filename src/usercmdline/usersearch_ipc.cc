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



#include "context.h"


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
    char* demo1[7] = {const_cast<char *>("usersearch"), 
                       const_cast<char *>("-u"), const_cast<char *>("lbq-7")};
    char* demo2[2] = {const_cast<char *>("usersearch"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string username;//= "lbq-7";
    std::string ownerid; // = "123456"; // 用户ID


    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = argv[0];
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("账户登录");
        command.add_options()
                ("user,u", po::value<std::string>(), "账户名")
                ;
        sp_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] =  [&](std::shared_ptr<po::variables_map> sp_variables_map)->void {
        if (sp_variables_map->count("user"))
        {
            username = (*sp_variables_map)["user"].as<std::string>();
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
        promise<bool> prom;
        auto fu = prom.get_future();
        // TODO:  调用IPC 客户端 进行同行，并获取返回结果
        IPCClient ipcClient("127.0.0.1", 6666);
        ipcClient.set_callback_func([&](IPCMessage msg)->void {
            std::string ipcresult (msg.body(), msg.body_length());
            if (ipcresult == "fail"){
                //std::cerr << "执行失败，请检查命令参数是否正确！详情请查看日志！" << std::endl;
                std::cerr << "search fail" << std::endl; // 执行结果
            } else {
                Account person;
                person.deserialize(ipcresult);
                std::cout << "账户名： " << person.accountName << std::endl;
                std::cout << "账户密码： " << person.Password << std::endl;
                std::cout << "邮箱： " << person.accountEmail << std::endl;
                std::cout << "电话： " << person.accountPhone << std::endl;
                std::cout << "地址： " << person.accountAddress << std::endl;
                std::cout << "单位： " << person.Department << std::endl;
            }
        });
        ipcClient.run(); // 停止的时候调用stop 函数
        std::cout << "正在执行命令..." << std::endl;

         // TODO: 构造请求结构体，并发送；
        IPCreq ipcreq;
        ipcreq.cmdname = "usersearch";  // 这块要和if 一致
        ipcreq.accountName = username; //账户名
        

        // TODO: 发送
        auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
        ipcClient.write(*msg); // 传递一个消息；
        sleep(1); // TODO: 等待客户端返回结果
        ipcClient.stop();

    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}





//  try
//     {
//         promise<bool> prom;
//         auto fu = prom.get_future();
//         // TODO:  调用IPC 客户端 进行同行，并获取返回结果
//         IPCClient ipcClient("127.0.0.1", 6666);
//         ipcClient.set_callback_func([&](IPCMessage msg) -> void {
//             string ipcresult(msg.body(), msg.body_length());
//             if (ipcresult == "")
//             {
//                 cout << "no match resource" << endl;
//             }
//             else
//             {
//                 vector<string> lists;
//                 json_decode(ipcresult, lists);
//                 for (auto res : lists)
//                 {
//                     StorageResource qres;
//                     std::string value = res.substr(8, res.length() - 9);
//                     qres.deserialize(value);
//                     int srcidlen = qres.storage_src_id.length();
//                     int prefixlen = StorageResource::prefix().length();
//                     string realsrcid = qres.storage_src_id.substr(prefixlen,srcidlen - prefixlen);
//                     cout<<"存储资源ID:"<<realsrcid<<endl;
//                     cout<<"存储资源名称:"<<qres.storage_src_name<<endl;
//                     cout<<"存储资源所在超算中心UUID:"<<qres.host_center_id<<endl;
//                     cout<<"存储资源所在超算中心名称:"<<qres.host_center_name<<endl;
//                     cout<<"存储资源空间容量大小:"<<qres.total_capacity<<endl;
//                     cout<<"存储资源已分配空间容量大小:"<<qres.assign_capacity<<endl;
//                     cout<<"存储资源MGS地址:"<<qres.mgs_address<<endl;
//                     cout<<"存储资源状态:"<<(StorageResState)qres.state<<endl;
//                 }
//             }

//             prom.set_value(true);
//         });
//         ipcClient.run(); // 停止的时候调用stop 函数
//         cout << "正在执行命令..." << endl;

//         // TODO: 构造请求结构体，并发送；
//         IPCreq ipcreq;
//         ipcreq.cmdname = "resourcequery";
//         ipcreq.storage_src_id = storage_src_id;

//         // TODO: 发送
//         auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
//         ipcClient.write(*msg); // 传递一个消息；
//         fu.get();              // TODO: 等待客户端返回结果
//         ipcClient.stop();
//     }