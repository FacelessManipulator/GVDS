//
// Created by sy on 5/30/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include "manager/space/Space.h"
#include "manager/zone/Zone.h"
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

using namespace Pistache;
using namespace hvs;
bool GetZoneInfo(std::string ip, int port, std::string clientID);
/*
 * zoneregister 命令行客户端
 */
std::unordered_map<std::string, std::string> zonemap;


int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    char* demo1[17] = {const_cast<char *>("zoneregister"), const_cast<char *>("--ip"), const_cast<char *>("192.168.5.222"),
                       const_cast<char *>("-p"), const_cast<char *>("49069"), const_cast<char *>("--zonename"),
                       const_cast<char *>("zonetest"), const_cast<char *>("--id"), const_cast<char *>("127"),
                       const_cast<char *>("--member"), const_cast<char *>("128"), /*const_cast<char *>("--member"), const_cast<char *>("322"),*/
                       const_cast<char *>("--spacename"), const_cast<char *>("spacetest"), const_cast<char *>("--spacesize"), const_cast<char *>("10"),
                       const_cast<char *>("--center"), const_cast<char *>("beihang")};
    char* demo2[2] = {const_cast<char *>("zoneregister"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string ip ;//= "127.0.0.1";
    int port ;//= 55107;
    std::string zonename ;//= "syremotezone"; 
    //std::string zoneuuid;
    std::string ownID;// = "202"; 
    std::vector<std::string> memID;// memberID
    std::string spacename ;//= "syremotezone"; // 空间名称
    int64_t spacesize;//空间大小
    SpaceMetaData spaceurl;




    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = argv[0];//"zoneregister";
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("区域注册模块");
        command.add_options()
                ("ip", po::value<std::string>(), "管理节点IP")
                ("port,p", po::value<int>(), "管理节点端口号")
                ("zonename", po::value<std::string>(), "区域名称")
                ("id", po::value<std::string>(), "主人ID")
                ("member", po::value<std::vector<std::string>>(), "区域成员")
                ("spacename", po::value<std::string>(), "空间名称")
                ("spacesize", po::value<int64_t>(), "空间大小")
                ("center", po::value<std::string>(), "超算名称")
                ;
        sp_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] =  [&](std::shared_ptr<po::variables_map> sp_variables_map)->void {
        if (sp_variables_map->count("ip"))
        {
            ip = (*sp_variables_map)["ip"].as<std::string>();//TODO:全局最优节点
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
        if (sp_variables_map->count("member"))
        {
            memID = (*sp_variables_map)["member"].as<std::vector<std::string>>();
        }
        if (sp_variables_map->count("spacename"))
        {
            spacename = (*sp_variables_map)["spacename"].as<std::string>();
        }
        if (sp_variables_map->count("spacesize"))
        {
            spacesize = (*sp_variables_map)["spacesize"].as<int64_t>();
        }
        if (sp_variables_map->count("center"))//TODO:全局最优节点
        {
            spaceurl.hostCenterName = (*sp_variables_map)["center"].as<std::string>();
        }
    };
    commandline.start(); //开始解析命令行参数


    // TODO: 构造请求
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/zone/register",ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    ZoneRegisterReq req;
    req.zoneName = zonename;
    req.ownerID = ownID;
    req.memberID = memID;
    req.spaceName = spacename;
    req.spaceSize = spacesize;
    req.spacePathInfo = spaceurl.serialize();


    std::string value = req.serialize();

    // TODO: 发送请求，并输出结果
    auto response = client.post(url).body(value).send();
    std::promise<bool> prom;
    auto fu = prom.get_future();
    response.then(
            [&](Http::Response res) {
                std::cout << res.body() << std::endl; //结果
                prom.set_value(true);
            },Async::IgnoreException);
    fu.get();
    client.shutdown();
    return 0;
}
