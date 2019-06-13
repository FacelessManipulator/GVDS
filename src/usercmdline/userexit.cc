
#include <iostream>
#include "manager/space/Space.h"
#include "manager/zone/Zone.h"
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

#include "manager/usermodel/Account.h"
#include <map>

using namespace Pistache;
using namespace hvs;
using namespace std;

map<string, string> user_id;
string mtoken = "1";

int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    // 1、用户登录
    char* demo1[7] = {const_cast<char *>("userexit"), 
                       const_cast<char *>("--ip"), const_cast<char *>("127.0.0.1"),
                       const_cast<char *>("-p"), const_cast<char *>("9090"), 
                       const_cast<char *>("--user"), const_cast<char *>("lbq-7")};
    char* demo2[2] = {const_cast<char *>("userexit"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string ip ;//= "127.0.0.1";
    int port ;//= 55107;
    std::string username;//= "lbq-7";
    std::string ownerid; // = "123456"; // 用户ID


    // TODO: 获取命令行信息
    CmdLineProxy commandline(7, demo1);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = "userexit";
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("账户退出");
        command.add_options()
                ("ip", po::value<std::string>(), "管理节点IP")
                ("port,p", po::value<int>(), "管理节点端口号")
                ("user,u", po::value<std::string>(), "账户名")
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
    };
    commandline.start(); //开始解析命令行参数

    user_id["lbq-7"] = "127";



    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/users/exit/%s", ip.c_str(), port, user_id[username].c_str());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    
    auto response_1 = client.get(url).cookie(Http::Cookie("token", mtoken)).send();
            //dout(-1) << "Client Info: get request " << url << dendl;
    std::cout << "Client Info: get request " << url << std::endl;
    std::promise<bool> prom_1;
    auto fu_1 = prom_1.get_future();
    response_1.then(
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
            prom_1.set_value(true);
        },
        Async::IgnoreException);
    fu_1.get();

    client.shutdown();

}