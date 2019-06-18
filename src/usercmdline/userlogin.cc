//
// Created by sy on 5/30/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include "hvs_struct.h"
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

#include "manager/usermodel/Account.h"

using namespace Pistache;
using namespace hvs;
//bool GetZoneInfo(std::string ip, int port, std::string clientID);
/*
 * zonerename 命令行客户端
 */
//std::unordered_map<std::string, std::string> zonemap;


int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    // 1、用户登录
    char* demo1[9] = {const_cast<char *>("userlogin"), 
                       const_cast<char *>("--ip"), const_cast<char *>("127.0.0.1"),
                       const_cast<char *>("-p"), const_cast<char *>("9090"), 
                       const_cast<char *>("--user"), const_cast<char *>("lbq-7"), 
                       const_cast<char *>("--pass"), const_cast<char *>("123456")};
    char* demo2[2] = {const_cast<char *>("userlogin"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string ip ;//= "127.0.0.1";
    int port ;//= 55107;
    std::string username;//= "lbq-7";
    std::string password; // = "123456"; // 用户ID


    // TODO: 获取命令行信息
    CmdLineProxy commandline(9, demo1);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = "userlogin";
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("账户登录");
        command.add_options()
                ("ip", po::value<std::string>(), "管理节点IP")
                ("port,p", po::value<int>(), "管理节点端口号")
                ("user,u", po::value<std::string>(), "账户名")
                ("pass", po::value<std::string>(), "账户密码")
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
        if (sp_variables_map->count("pass"))
        {
            password = (*sp_variables_map)["pass"].as<std::string>();
        }
    };
    commandline.start(); //开始解析命令行参数

    //账户登录
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/users/login", ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    AccountPass myaccount;
    myaccount.accountName = username;
    myaccount.Password = password;
    std::string mes = myaccount.serialize();
    // std::string mes = "{\"HVSAccountName\":\"lbq-7\",\"HVSPassword\":\"123456\"}";
    // std::cout << mes << std::endl;

    std::string mtoken;

    //auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(mes).send();
    auto response = client.post(url).body(mes).send();
            //dout(-1) << "Client Info: post request " << url << dendl;

    std::cout << "2222" << std::endl;
    std::promise<bool> prom;
    auto fu = prom.get_future();
    response.then(
        [&](Http::Response res) {
            //dout(-1) << "Manager Info: " << res.body() << dendl;
            std::cout << "Response code = " << res.code() << std::endl;
            auto body = res.body();
            if (!body.empty()){
                std::cout << "Response body = " << body << std::endl;
                //====================
                //your code write here

                //====================
            }
            std::cout<< "Response cookie = ";
            auto cookies = res.cookies();
            for (const auto& c: cookies) {
                std::cout << c.name << " : " << c.value << std::endl;
                mtoken = c.value;
            }
            prom.set_value(true);
        },
        Async::IgnoreException);
    fu.get();
    
    client.shutdown();

    //+++++++++++++++++++++++++++++++++++++++
    return 0;
}
