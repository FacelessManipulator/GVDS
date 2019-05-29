//
// Created by yaowen on 5/29/19.
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
 * spacerename 命令行客户端
 */
std::unordered_map<std::string, std::string> zonemap;


int main(int argc, char* argv[]){
    // TODO: 1.获取账户登录信息 2.检索区域信息 3. 提交空间重命名申请
    char* demo1[13] = {const_cast<char *>("spacerename"), const_cast<char *>("--ip"), const_cast<char *>("192.168.5.224"),
                       const_cast<char *>("-p"), const_cast<char *>("44025"), const_cast<char *>("--zonename"),
                       const_cast<char *>("syremotezone"), const_cast<char *>("--id"), const_cast<char *>("202"),
                       const_cast<char *>("-o"), const_cast<char *>("NewWorld"), const_cast<char *>("-n"),
                       const_cast<char *>("BIGBOSSSY")};
    char* demo2[2] = {const_cast<char *>("spacerename"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string ip ;//= "127.0.0.1";
    int port ;//= 55107;
    std::string zonename ;//= "syremotezone"; // 空间名称
    std::string ownID;// = "202"; // 用户ID
    std::string oldspacename;// = "NewWorld";
    std::string newspacename;// = "BUAABUAA";
    std::string spaceuuid;

    // TODO: 获取命令行信息
    CmdLineProxy commandline(13, demo1);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = "spacerename";
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("空间重命名模块");
        command.add_options()
                ("ip", po::value<std::string>(), "管理节点IP")
                ("port,p", po::value<int>(), "管理节点端口号")
                ("zonename", po::value<std::string>(), "区域名称")
                ("id", po::value<std::string>(), "管理员ID")
                ("oldname,o", po::value<std::string>(), "空间旧名称")
                ("newname,n", po::value<std::string>(), "空间新名称")
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
        if (sp_variables_map->count("zonename"))
        {
            zonename = (*sp_variables_map)["zonename"].as<std::string>();
        }
        if (sp_variables_map->count("id"))
        {
            ownID = (*sp_variables_map)["id"].as<std::string>();
        }
        if (sp_variables_map->count("oldname"))
        {
            oldspacename = (*sp_variables_map)["oldname"].as<std::string>();
        }
        if (sp_variables_map->count("newname"))
        {
            newspacename = (*sp_variables_map)["newname"].as<std::string>();
        }
    };
    commandline.start(); //开始解析命令行参数

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ip, port, ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        exit(-1);
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        ZoneInfo zoneinfo;
        zoneinfo.deserialize(mapping->second);
        for(const auto &it : zoneinfo.spaceBicInfo.spaceID){
            if (zoneinfo.spaceBicInfo.spaceName[it] == oldspacename){
                spaceuuid = it;
                break;
            }
        }
        if(spaceuuid.empty()){
            std::cerr << "空间名不存在，请确认空间名称正确！" << std::endl;
            exit(-1);
        }
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        exit(-1);
    }
    // TODO: 构造间重命名请求
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/space/rename",ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    SpaceRenameReq req;
    req.spaceID = spaceuuid;
    req.newSpaceName = newspacename;
    std::string value = req.serialize();

    // TODO: 发送间重命名请求，并输出结果
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

// 调用获取区域信息；
bool GetZoneInfo(std::string ip, int port, std::string clientID) {
    Pistache::Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/zone/info", ip.c_str(), port);
    auto opts = Pistache::Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);
    std::string value = std::move(clientID);
    //std::cerr<< "Client Info: post request " << url << std::endl;
    auto response = client.post(url).body(value).send();
    std::promise<bool> prom;
    std::string inforesult;
    auto fu = prom.get_future();
    response.then(
    [&](Pistache::Http::Response res) {
        inforesult = res.body();
        prom.set_value(true);
    },Pistache::Async::IgnoreException);
    fu.get();
    client.shutdown();
    GetZoneInfoRes zoneinfores;
    zoneinfores.deserialize(inforesult); //获取返回的结果
    if(zoneinfores.zoneInfoResult.empty()){
        return false;
    }
    for(const auto &it : zoneinfores.zoneInfoResult){
        ZoneInfo zoneinfo;
        zoneinfo.deserialize(it);
        zonemap[zoneinfo.zoneName] = it;
    }
    return true;
}