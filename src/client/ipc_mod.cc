//
// Created by yaowen on 6/11/19.
// 北航系统结构所-存储组
//

#include "ipc_mod.h"
#include "client/msg_mod.h"
#include "client/clientuser/ClientUser.h"
#include "client/OPTNode/opt_node.h"
#include "manager/manager.h"
#include "aggregation_struct.h"

#include <errno.h>


using namespace hvs;
using namespace Pistache;
using namespace std;

namespace hvs{
void ClientIPC::start() {
    sp_ipcserver->run(); // ipc 服务器启动
}

void ClientIPC::stop() {
    sp_ipcserver->stop(); // ipc 服务器停止
}

void ClientIPC::init() {
    try {
        sp_ipcserver = std::make_shared<IPCServer>(6666); // TODO : 端口需要再次确定！
        sp_ipcserver->set_callback_func([&](IPCMessage msg)-> std::string {
              //命令行处理逻辑需要添加
//            std::string cmdline(msg.body(), msg.body_length());
//            CmdlineParameters cmdlineParameters;
//            cmdlineParameters.deserialize(cmdline); // 进行反序列化解析参数
//            //构造命令行参数
//            char* argv[cmdlineParameters.argc];
//            for (int i= 0; i < cmdlineParameters.argc; i++){
//                argv[i] = (char*)alloca(cmdlineParameters.argv[i].size()+1); // 进行在栈上分配内存；
//                memcpy(argv[i], cmdlineParameters.argv[i].c_str(), cmdlineParameters.argv[i].size());
//            }
            // TODO: 根据发送来的消息判断命令并选择执行
            std::string cmdlinereq(msg.body(), msg.body_length());
            IPCreq ipcreq;
            ipcreq.deserialize(cmdlinereq);

            // TODO: 添加命令信息
            if(ipcreq.cmdname == "spacerename"){
                return dospacerename(ipcreq);
            }
            if(ipcreq.cmdname == "spacesizechange"){
                return dospacesizechange(ipcreq);
            }
            if(ipcreq.cmdname == "mapadd"){
                return domapadd(ipcreq);
            }
            if(ipcreq.cmdname == "mapdeduct"){
                return domapdeduct(ipcreq);
            }
            if(ipcreq.cmdname == "zoneadd"){
                return dozoneadd(ipcreq);
            }
            if(ipcreq.cmdname == "zonecancel"){
                return dozonecancel(ipcreq);
            }
            if(ipcreq.cmdname == "zoneregister"){
                return dozoneregister(ipcreq);
            }
            if(ipcreq.cmdname == "zonerename"){
                return dozonerename(ipcreq);
            }
            if(ipcreq.cmdname == "zoneshare"){
                return dozoneshare(ipcreq);
            }
            if(ipcreq.cmdname == "zonesharecancel"){
                return dozonesharecancel(ipcreq);
            }
            //auth
            if(ipcreq.cmdname == "userlogin"){
                cout << "can get here1" <<endl;
                return douserlogin(ipcreq);
            }
            if(ipcreq.cmdname == "usersearch"){
                cout << "can get here2" <<endl;
                return dousersearch(ipcreq);
            }
            if(ipcreq.cmdname == "usersignup"){
                cout << "can get here3" <<endl;
                return dousersignup(ipcreq);
            }
            if(ipcreq.cmdname == "usermodify"){
                cout << "can get here4" <<endl;
                return dousermodify(ipcreq);
            }
            if(ipcreq.cmdname == "userexit"){
                cout << "can get here5" <<endl;
                return douserexit(ipcreq);
            }
            if(ipcreq.cmdname == "usercancel"){
                cout << "can get here5" <<endl;
                return dousercancel(ipcreq);
            }
            if(ipcreq.cmdname == "authsearch"){
                cout << "can get here1" <<endl;
                return doauthsearch(ipcreq);
            }
            if(ipcreq.cmdname == "authmodify"){
                cout << "can get here1" <<endl;
                return doauthmodify(ipcreq);
            }

            
            //resource register
            if(ipcreq.cmdname == "resourceregister"){
                cout << "resource register..." <<endl;
                return doresourceregister(ipcreq);
            }
            //resource delete
            if(ipcreq.cmdname == "resourcedelete"){
                cout << "resource delete..." <<endl;
                return doresourcedelete(ipcreq);
            }
            //resource query
            if(ipcreq.cmdname == "resourcequery"){
                cout << "resource query..." <<endl;
                return doresourcequery(ipcreq);
            }
            //resource update
            if(ipcreq.cmdname == "resourceupdate"){
                cout << "resource update..." <<endl;
                return doresourceupdate(ipcreq);
            }



            else{
                std::cerr << "警告：出现不支持的命令的请求！" << std::endl;
                return "警告：出现不支持的命令的请求！";
            }
        });
    } catch (std::exception &e){
        std::cout << e.what() << std::endl;
    }
}

std::string ClientIPC::doregisterupdate(IPCreq &ipcreq,std::string url)
{
    StorageResource newRes; 
    newRes.storage_src_id = ipcreq.storage_src_id;        // 存储资源UUID
    newRes.storage_src_name = ipcreq.storage_src_name;    // 存储资源名称
    newRes.host_center_id = ipcreq.host_center_id;        // 存储资源所在超算中心UUID
    newRes.host_center_name = ipcreq.host_center_name;    // 存储资源所在超算中心名称
    newRes.total_capacity = ipcreq.total_capacity;        // 存储资源空间容量大小
    newRes.assign_capacity = ipcreq.assign_capacity;      // 已经分配空间容量大小
    newRes.mgs_address = ipcreq.mgs_address;              // 存储资源MGS地址
    newRes.state = (StorageResState)ipcreq.state;         // 存储资源状态

    std::string cinfor = client->optNode->getCenterInfo();
    CenterInfo mycenter;
    mycenter.deserialize(cinfor); 
    bool flag = false;
    for(vector<string>::iterator iter = mycenter.centerID.begin(); iter!=mycenter.centerID.end(); iter++){
        string centername = mycenter.centerName[*iter];
        string centerid =  *iter;
        if(centername == newRes.host_center_name && centerid == newRes.host_center_id )
        {
            flag = true;
            break;
        }
    }

    if(!flag) return "input center id or center name is wrong";
    string endpoint = client->get_manager();
    string res = client->rpc->post_request(endpoint, url, newRes.serialize());
    return res;
}

std::string ClientIPC::doresourceupdate(IPCreq &ipcreq) {

   return doregisterupdate(ipcreq,"/resource/update");
}

std::string ClientIPC::doresourcequery(IPCreq &ipcreq) {
    string url = "/resource/query/" + ipcreq.storage_src_id;
    string endpoint = client->get_manager();
    string res = client->rpc->get_request(endpoint, url);
    return res;
}

std::string ClientIPC::doresourcedelete(IPCreq &ipcreq) {
    string url = "/resource/delete/" + ipcreq.storage_src_id;
    string endpoint = client->get_manager();
    string res = client->rpc->delete_request(endpoint, url);
    return res;
}

std::string ClientIPC::doresourceregister(IPCreq &ipcreq) {

    return doregisterupdate(ipcreq,"/resource/register");
}

std::string ClientIPC::dospacerename(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    std::string spacename = ipcreq.spacename;
    std::string newspacename = ipcreq.newspacename;
    std::string spaceuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        auto zoneinfo = mapping->second;
        if(zoneinfo.ownerID != ownID) return "權限不足";
        for(auto it : zoneinfo.spaceBicInfo){
            if (it->spaceName == spacename){
                spaceuuid = it->spaceID;
                break;
            }
        }
        if(spaceuuid.empty()){
            std::cerr << "空间名不存在，请确认空间名称正确！" << std::endl;
            return "空间名不存在，请确认空间名称正确！";
        }
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    SpaceRequest req;
    req.spaceID = spaceuuid;
    req.newSpaceName = newspacename;
    string response = client->rpc->post_request(client->get_manager(), "/space/rename", req.serialize());
    return response;
}

std::string ClientIPC::dospacesizechange(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string ip = ipcreq.ip;
    int port = ipcreq.port;
    std::string zonename = ipcreq.zonename;
    std::string ownID = ipcreq.ownID;
    std::string spacename = ipcreq.spacename;
    int newspacesize = ipcreq.newspacesize;
    std::string spaceuuid = ipcreq.spaceuuid;
    
    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
       return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        Zone zoneinfo = mapping->second;
        for(const auto it : zoneinfo.spaceBicInfo){
            if (it->spaceName == spacename){
                spaceuuid = it->spaceID;
                break;
            }
        }
        if(spaceuuid.empty()){
            std::cerr << "空间名不存在，请确认空间名称正确！" << std::endl;
            return "空间名不存在，请确认空间名称正确！";
        }
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }
    // TODO: 构造间重命名请求
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/space/changesize",ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    SpaceRequest req;
    req.spaceID = spaceuuid;
    req.newSpaceSize = newspacesize;

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
    return "success";
}

std::string ClientIPC::domapadd(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string ip = ipcreq.ip;
    int port = ipcreq.port;
    std::string zonename = ipcreq.zonename;
    std::string ownID = ipcreq.ownID;
    std::string spacename = ipcreq.spacename;
    int64_t spacesize = ipcreq.spacesize;
    std::string spaceurl = ipcreq.spaceurl;
    std::string zoneuuid = ipcreq.zoneuuid;


    // TODO: 获取区域信息
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        zoneuuid = mapping->second.zoneID;
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }
    if(spacename == "" || spacesize == 0){
        std::cerr << "信息不全，无法创建映射" << std::endl;
        return  "信息不全，无法创建映射";
    }
    else{
        // TODO: 构造映射增加请求
        Http::Client client;
        char url[256];
        snprintf(url, 256, "http://%s:%d/zone/mapadd",ip.c_str(), port);
        auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
        client.init(opts);


        ZoneRequest req;
        req.zoneID = zoneuuid;
        req.ownerID = ownID;
        req.spaceName = spacename;
        req.spaceSize = spacesize;
        req.spacePathInfo = spaceurl;
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
        return "success";
    }
}

std::string ClientIPC::domapdeduct(IPCreq &ipcreq) {
    // TODO: 提前准备的数据

    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;
    std::string zoneuuid;

    //获取区域信息
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        Zone zoneinfo = mapping->second;
        zoneuuid = zoneinfo.zoneID;
        for(const auto &m : spacenames){
            bool found = false;
            for(const auto it : zoneinfo.spaceBicInfo){
                if (it->spaceName == m){
                    spaceuuids.push_back(it->spaceID);
                    found = true;
                    break;
                }
            }
            if (!found){
                std::cerr << "空间名" << m << "不存在，请确认空间名称正确！" << std::endl;
                return "空间名"+m+"不存在，请确认空间名称正确！";
            }
        }
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID;
    req.spaceID = spaceuuids;

    string response = client->rpc->post_request(client->get_manager(), "/zone/mapdeduct", req.serialize());//TODO：修改按需选择管理节点
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);
}

std::string ClientIPC::dozoneadd(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string ip = ipcreq.ip;
    int port = ipcreq.port;
    std::string zonename = ipcreq.zonename;
    std::string ownID = ipcreq.ownID;
    std::vector<std::string> memID = ipcreq.memID;
    std::string spaceurl = ipcreq.spaceurl;

    // TODO: 构造请求
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/zone/add",ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    ZoneRequest req;
    req.zoneName = zonename;
    req.ownerID = ownID;
    req.memberID = memID;
    req.spacePathInfo = spaceurl;


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
    return "success";
}

std::string ClientIPC::dozonecancel(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        zoneuuid = mapping->second.zoneID;
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID;
    string response = client->rpc->post_request(client->get_manager(), "/zone/cancel", req.serialize());//TODO：修改按需选择管理节点
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);
}

std::string ClientIPC::dozoneregister(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = ipcreq.ownID;
    std::vector<std::string> memID = ipcreq.memID;
    std::string spacename = ipcreq.spacename;
    int64_t spacesize = ipcreq.spacesize;
    std::string spaceurl = ipcreq.spaceurl;

    ZoneRequest req;
    req.zoneName = zonename;
    req.ownerID = ownID;
    req.memberID = memID;
    req.spaceName = spacename;
    req.spaceSize = spacesize;
    req.spacePathInfo = spaceurl;
    string response = client->rpc->post_request(client->get_manager(), "/zone/register", req.serialize());//TODO：修改按需选择管理节点
    return response;
}

std::string ClientIPC::dozonerename(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    std::string newzonename = ipcreq.newzonename;
    std::string zoneuuid = ipcreq.zoneuuid;

        // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        zoneuuid = mapping->second.zoneID;
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }
    // TODO: 构造间重命名请求

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID;
    req.newZoneName = newzonename;
    string response = client->rpc->post_request(client->get_manager(), "/zone/rename", req.serialize());
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);    
}

std::string ClientIPC::dozoneshare(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    std::vector<std::string> memID;
    bool tm = client->user->getMemberID(ipcreq.memName, memID);
    if(!tm){
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string zoneuuid = ipcreq.zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        zoneuuid = mapping->second.zoneID;
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID;
    req.memberID = memID;
    string response = client->rpc->post_request(client->get_manager(), "/zone/share", req.serialize());
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);   
}

std::string ClientIPC::dozonesharecancel(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    std::vector<std::string> memID;
    bool tm = client->user->getMemberID(ipcreq.memName, memID);
    if(!tm){
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string zoneuuid = ipcreq.zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        zoneuuid = mapping->second.zoneID;
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID;
    req.memberID = memID;
    string response = client->rpc->post_request(client->get_manager(), "/zone/sharecancel", req.serialize());
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);   
}

// 调用获取区域信息；
bool ClientIPC::GetZoneInfo(std::string clientID) {
    vector<Zone> zoneinfores;
    string endpoint = client->get_manager();
    string inforesult = client->rpc->post_request(endpoint, "/zone/info", clientID);
    if (!inforesult.empty()) {
        json_decode(inforesult, zoneinfores); //获取返回的结果
    }
    if(zoneinfores.empty()){
        return false;
    }
    for(const auto &it : zoneinfores) {
        zonemap[it.zoneName] = it;
    }
    return true;
}

//user客户端函数
std::string ClientIPC::douserlogin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    string username = ipcreq.accountName;
    string password = ipcreq.Password;

    //客户端处理流程

    //账户登录
    Http::Client Pclient;
    char url[256];
    //snprintf(url, 256, "http://%s:%d/users/login", ip.c_str(), port);
    snprintf(url, 256, "%s/users/login", client->get_manager().c_str());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    Pclient.init(opts);

    AccountPass myaccount;
    myaccount.accountName = username;
    myaccount.Password = password;
    std::string mes = myaccount.serialize();

    cout << myaccount.accountName << endl;

    std::string mtoken;
    std::string return_value = "login fail";

    //auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(mes).send();
    auto response = Pclient.post(url).body(mes).send();
            //dout(-1) << "Client Info: post request " << url << dendl;

    std::promise<bool> prom;
    auto fu = prom.get_future();
    response.then(
        [&](Http::Response res) {
            //dout(-1) << "Manager Info: " << res.body() << dendl;
            std::cout << "Response code = " << res.code() << std::endl;
            if (Http::Code::Ok ==  res.code()){
                auto body = res.body();
                if (!body.empty()){
                    std::cout << "Response body = " << body << std::endl;
                }
                std::cout<< "Response cookie = ";
                auto cookies = res.cookies();
                for (const auto& c: cookies) {
                    std::cout << c.name << " : " << c.value << std::endl;
                    mtoken = c.value;
                }
                client->user->setToken(mtoken);
                client->user->setAccountName(myaccount.accountName);
                client->user->setAccountID(body);
                
                cout << "getToken(): " << client->user->getToken() << endl;
                cout << "getAccountName(): " << client->user->getAccountName() << endl;
                cout << "getAccountID(): " << client->user->getAccountID() << endl;
                cout << "centerName= Beijing   id= " << client->optNode->getmapIdName("Beijing") << endl;

                std::vector<std::string> memberName; 
                memberName.push_back("lbq-7");
                memberName.push_back("lbq-8");
                std::vector<std::string> memberID;
                bool tm = client->user->getMemberID(memberName, memberID);
                if(tm){
                    for(int j=0; j<memberID.size(); j++){
                        cout << "memID: " << memberID[j] << endl;
                    }
                }
                //client->zone->GetZoneInfo("202");
                return_value = "login success";
            }//if

            prom.set_value(true);
        },
        Async::IgnoreException);
    fu.get();
    
    Pclient.shutdown();

    return return_value;
}


std::string ClientIPC::dousersearch(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    string username = ipcreq.accountName;

     //账户查询
    string return_value = "fail";
    string endpoint = client->get_manager();   
    string routepath = "/users/search/" + client->user->getAccountID();    ///users/search/用戶id
    string res = client->rpc->get_request(endpoint, routepath);
    if (res == "-1"){
        return return_value;
    }
    cout << "response: " << res <<endl;
    return res;

   
   
    // Http::Client Pclient;
    // char url[256];
    // snprintf(url, 256, "http://%s:%d/users/search/%s", ip.c_str(), port, client->user->getAccountID().c_str());
    // auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    // Pclient.init(opts);

    //  auto response_1 = Pclient.get(url).cookie(Http::Cookie("token", "mtoken")).send();
    //     //dout(-1) << "Client Info: get request " << url << dendl;
    // cout << "Client Info: get request " << url << endl;
    // std::promise<bool> prom_1;
    // auto fu_1 = prom_1.get_future();
    // response_1.then(
    //     [&](Http::Response res) {
    //         //dout(-1) << "Manager Info: " << res.body() << dendl;
    //         std::cout << "Response code = " << res.code() << std::endl;
    //         //if code== 
    //         return_value = "search success";
    //         auto body = res.body();
    //         if (!body.empty()){
    //             std::cout << "Response body = " << body << std::endl;
    //             //====================
    //             //your code write here

    //             //====================
    //         }
    //         prom_1.set_value(true);
    //     },
    //     Async::IgnoreException);
    // fu_1.get();

    // Pclient.shutdown();
    // std::cout << "end: dousersearch" << std::endl;
    // return return_value;
}



std::string ClientIPC::dousersignup(IPCreq &ipcreq){
    // TODO: 提前准备的数据

        
    string username = ipcreq.accountName; //账户名
    string hvsID = ipcreq.hvsID; //在服务端产生
    string pass = ipcreq.Password;
    string email = ipcreq.email;
    string phone = ipcreq.phone;
    string ad = ipcreq.address;
    string de = ipcreq.department;

    //账户注册
    Account person(username, pass, hvsID, email, phone, ad,  de);   //以后在服务端产生uuid，客户端这块传值不影响
    std::string value = person.serialize();
    std::cout << value << std::endl;

    string endpoint = client->get_manager();   
    string routepath = "/users/registration";    ///users/search/用戶id

    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "response: " << res <<endl;
    return res;

    // std::string return_value = "signup fail";
    // Http::Client client;
    // char url[256];
    // snprintf(url, 256, "http://%s:%d/users/registration", ip.c_str(), port);
    // auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    // client.init(opts);

    // // Account person("lbq-9", "123456", "129", "XXXXXX@163.com", "15012349876", "xueyuanlu",  "Beihang");
    // // std::string value = person.serialize();
    // Account person(username, pass, hvsID, email, phone, ad,  de);   //以后在服务端产生uuid，客户端这块传值不影响
    // std::string value = person.serialize();
    // std::cout << value << std::endl;

    // auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(value).send();
    //     //dout(-1) << "Client Info: post request " << url << dendl;
    // std::cout <<  "Client Info: post request " << url << std::endl;

    // std::promise<bool> prom;
    // auto fu = prom.get_future();
    // response.then(
    //     [&](Http::Response res) {
    //       //dout(-1) << "Manager Info: " << res.body() << dendl;
    //       std::cout << "Response code = " << res.code() << std::endl;
    //       // code==  
    //       return_value ="signup success";
    //       auto body = res.body();
    //       if (!body.empty()){
    //           std::cout << "Response body = " << body << std::endl;
    //           //====================
    //           //your code write here

    //           //====================
    //       }
    //       prom.set_value(true);
    //     },
    //     Async::IgnoreException);
    // fu.get();

    // client.shutdown();

    // return return_value;

}


std::string ClientIPC::dousermodify(IPCreq &ipcreq){
 // TODO: 提前准备的数据
    string ip = ipcreq.ip;
    int port = ipcreq.port;

    

    string username = ipcreq.accountName; //账户名
    string hvsID = client->user->getAccountID(); //在服务端产生     //TODO 账户修改这块，调用函数获取
    string pass = ipcreq.Password;
    string email = ipcreq.email;
    string phone = ipcreq.phone;
    string ad = ipcreq.address;
    string de = ipcreq.department;

  //账户修改



    std::string return_value = "modify fail";
    Account person(username, pass, hvsID, email, phone, ad, de);
    std::string person_value = person.serialize();
    std::cout << person_value << std::endl;

    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/users/modify", ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);


    auto response_1 = client.post(url).cookie(Http::Cookie("token", "mtoken")).body(person_value).send();
            //dout(-1) << "Client Info: post request " << url << dendl;
            std::cout << "Client Info: post request " << url << std::endl;

    std::promise<bool> prom_1;
    auto fu_1 = prom_1.get_future();
    response_1.then(
        [&](Http::Response res) {
            //dout(-1) << "Manager Info: " << res.body() << dendl;
            std::cout << "Response code = " << res.code() << std::endl;
            auto body = res.body();
            //if code ==  
            return_value = "modify success";
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

    return return_value;
}

std::string ClientIPC::douserexit(IPCreq &ipcreq){
    // TODO: 提前准备的数据
    string ip = ipcreq.ip;
    int port = ipcreq.port;

    //账户退出
    std::string return_value = "exit fail";
      Http::Client Pclient;
    char url[256];
    snprintf(url, 256, "http://%s:%d/users/exit/%s", ip.c_str(), port, client->user->getAccountID().c_str());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    Pclient.init(opts);

    
    auto response_1 = Pclient.get(url).cookie(Http::Cookie("token", "mtoken")).send();
            //dout(-1) << "Client Info: get request " << url << dendl;
    std::cout << "Client Info: get request " << url << std::endl;
    std::promise<bool> prom_1;
    auto fu_1 = prom_1.get_future();
    response_1.then(
        [&](Http::Response res) {
            //dout(-1) << "Manager Info: " << res.body() << dendl;
            std::cout << "Response code = " << res.code() << std::endl;
            return_value = "exit success";
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

    Pclient.shutdown();
    return return_value;
}


//auth
 std::string ClientIPC::doauthsearch(IPCreq &ipcreq){
     // TODO: 提前准备的数据
    string ip = ipcreq.ip;
    int port = ipcreq.port;

    std::string hvsID = client->user->getAccountID(); //在服务端产生 

    //权限查询
    std::string return_value = "authsearch success";
    string endpoint = client->get_manager();
    string routepath = "/auth/search";
    string res = client->rpc->post_request(endpoint, routepath, hvsID);
    cout << "respones:" << res << endl;

    //if()
    AuthSearch myauth;
    myauth.deserialize(res);

    cout << myauth.hvsID << endl;
    vector<string>::iterator iter;
    for (iter = myauth.vec_ZoneID.begin(); iter != myauth.vec_ZoneID.end(); iter++){
        cout << *iter << endl;
        cout << myauth.read[*iter] << endl;
        cout << myauth.write[*iter] << endl;
        cout << myauth.exe[*iter] << endl;  //可以加上显示，是这个区的成员 还是 主人，回头加吧
    }

    return return_value;

    // std::string return_value = "authsearch fail";
    //  Http::Client client;
    // char url[256];
    // //snprintf(url, 256, "http://localhost:%d/auth/search", manager->rest_port());
    // snprintf(url, 256, "http://localhost:9090/auth/search");

    // auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    // client.init(opts);

    // std::cout << "before" << endl;
    // auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(hvsID).send();
    //         dout(-1) << "Client Info: post request " << url << dendl;

    // std::cout << "after" << endl;
    // std::promise<bool> prom;
    // auto fu = prom.get_future();
    // response.then(
    //     [&](Http::Response res) {
    //         //dout(-1) << "Manager Info: " << res.body() << dendl;
    //         std::cout << "Response code = " << res.code() << std::endl;
    //         return_value = "authsearch success";
    //         auto body = res.body();
    //         if (!body.empty()){
    //             std::cout << "Response body = " << body << std::endl;
    //             //====================
    //             //your code write here
    //             AuthSearch myauth;
    //             myauth.deserialize(body);

    //             cout << myauth.hvsID << endl;
    //             vector<string>::iterator iter;
    //             for (iter = myauth.vec_ZoneID.begin(); iter != myauth.vec_ZoneID.end(); iter++){
    //                 cout << *iter << endl;
    //                 cout << myauth.read[*iter] << endl;
    //                 cout << myauth.write[*iter] << endl;
    //                 cout << myauth.exe[*iter] << endl;  //可以加上显示，是这个区的成员 还是 主人，回头加吧
    //             }
    //             //====================
    //         }
    //         prom.set_value(true);
    //     },
    //     Async::IgnoreException);
    // fu.get();

    // client.shutdown();

    // return return_value;
 }

 std::string ClientIPC::doauthmodify(IPCreq &ipcreq){
         // TODO: 提前准备的数据
    string ip = ipcreq.ip;
    int port = ipcreq.port;

    FEAuthModifygroupinfo FEgroup;
    FEgroup.hvsID = client->user->getAccountID(); //在服务端产生 
    FEgroup.zonename = ipcreq.zonename;
    FEgroup.modify_groupauth = ipcreq.changeauth;

    string value = FEgroup.serialize();

    //权限修改
    std::string return_value = "authmodify fail";
    Http::Client client;
    char url[256];
    //snprintf(url, 256, "http://localhost:%d/auth/modify", manager->rest_port());
    snprintf(url, 256, "http://localhost:9090/auth/modify");
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);


    std::cout << "before" << endl;
    auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(value).send();
            dout(-1) << "Client Info: post request " << url << dendl;

    std::cout << "after" << endl;
    std::promise<bool> prom;
    auto fu = prom.get_future();
    response.then(
        [&](Http::Response res) {
            //dout(-1) << "Manager Info: " << res.body() << dendl;
            std::cout << "Response code = " << res.code() << std::endl;
            return_value = "authmodify success";
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
    return return_value;
 }


std::string ClientIPC::dousercancel(IPCreq &ipcreq){
     // TODO: 提前准备的数据
    string ip = ipcreq.ip;
    int port = ipcreq.port;

    //用户注销
    std::string return_value = "usercancel fail";
    Http::Client Pclient;
    char url[256];
    snprintf(url, 256, "http://%s:%d/users/cancel/%s", ip.c_str(), port, client->user->getAccountID().c_str());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    Pclient.init(opts);

    auto response_1 = Pclient.get(url).cookie(Http::Cookie("token", "mtoken")).send();
            //dout(-1) << "Client Info: get request " << url << dendl;

    std::cout << "Client Info: get request " << url << std::endl;

    std::promise<bool> prom_1;
    auto fu_1 = prom_1.get_future();
    response_1.then(
        [&](Http::Response res) {
            //dout(-1) << "Manager Info: " << res.body() << dendl;
            std::cout << "Response code = " << res.code() << std::endl;
            return_value = "usercancel success";
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

    Pclient.shutdown();
    return return_value;
}

}//namespace