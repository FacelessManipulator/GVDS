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



int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    // 1、用户登录
    char* demo1[17] = {const_cast<char *>("usersignup"), 
                       const_cast<char *>("--ip"), const_cast<char *>("192.168.10.219"),
                       const_cast<char *>("-p"), const_cast<char *>("44769"), 
                       const_cast<char *>("--user"), const_cast<char *>("lbq-9"),
                       const_cast<char *>("--pass"), const_cast<char *>("123456"),
                       const_cast<char *>("-e"), const_cast<char *>("XXXXXX@163.com"),
                       const_cast<char *>("--phone"), const_cast<char *>("15012349876"),
                       const_cast<char *>("--ad"), const_cast<char *>("xueyuanlu_xinzhulou"),
                       const_cast<char *>("--de"), const_cast<char *>("Beihang")};
    char* demo2[2] = {const_cast<char *>("usersignup"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string ip ;//= "127.0.0.1";
    int port ;//= 55107;
    std::string username;//= "lbq-7";
    std::string ownerid; // = "123456"; // 用户ID
    std::string pass;
    std::string email;
    std::string phone;
    std::string ad;
    std::string de;

    // TODO: 获取命令行信息
    CmdLineProxy commandline(17, demo1);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = "usersignup";
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("账户注册");
        command.add_options()
                ("ip", po::value<std::string>(), "管理节点IP")
                ("port,p", po::value<int>(), "管理节点端口号")
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

    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/users/registration", ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    // Account person("lbq-9", "123456", "129", "XXXXXX@163.com", "15012349876", "xueyuanlu",  "Beihang");
    // std::string value = person.serialize();
    Account person(username, pass, "129", email, phone, ad,  de);   //以后在服务端产生uuid，客户端这块传个空
    std::string value = person.serialize();
    std::cout << value << std::endl;

    auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(value).send();
        //dout(-1) << "Client Info: post request " << url << dendl;
    std::cout <<  "Client Info: post request " << url << std::endl;

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
          prom.set_value(true);
        },
        Async::IgnoreException);
    fu.get();

    client.shutdown();
}