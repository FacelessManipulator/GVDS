//
// Created by yaowen on 6/11/19.
// 北航系统结构所-存储组
//

#include "ipc_mod.h"
#include "client/msg_mod.h"
#include "client/clientuser/ClientUser.h"
#include "manager/manager.h"
#include "aggregation_struct.h"


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
            // if(ipcreq.cmdname == "authsearch"){
            //     cout << "can get here1" <<endl;
            //     return doauthsearch(ipcreq);
            // }

            
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



            else{
                std::cerr << "警告：出现不支持的命令的请求！" << std::endl;
                return "警告：出现不支持的命令的请求！";
            }
        });
    } catch (std::exception &e){
        std::cout << e.what() << std::endl;
    }
}

std::string ClientIPC::doresourcedelete(IPCreq &ipcreq) {
  
//   string endpoint = client->get_manager();
//   string url  = "resource/delete/" + 
//   string res = client->rpc->post_request(endpoint, "/resource/register", newRes.serialize());
//   return res;

  std::cout<<"delete successfully";
  return "OK";
}

std::string ClientIPC::doresourceregister(IPCreq &ipcreq) {
  StorageResource newRes; 
  newRes.storage_src_id = ipcreq.storage_src_id;        // 存储资源UUID
  newRes.storage_src_name = ipcreq.storage_src_name;    // 存储资源名称
  newRes.host_center_id = ipcreq.host_center_id;        // 存储资源所在超算中心UUID
  newRes.host_center_name = ipcreq.host_center_name;    // 存储资源所在超算中心名称
  newRes.total_capacity = ipcreq.total_capacity;        // 存储资源空间容量大小
  newRes.assign_capacity = 0;                           // 已经分配空间容量大小
  newRes.mgs_address = ipcreq.mgs_address;              // 存储资源MGS地址
  newRes.state = Normal;                                // 存储资源状态
  string endpoint = client->get_manager();
  string res = client->rpc->post_request(endpoint, "/resource/register", newRes.serialize());
  return res;
}




std::string ClientIPC::dospacerename(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string ip = ipcreq.ip;
    int port = ipcreq.port;
    std::string zonename = ipcreq.zonename;
    std::string ownID = ipcreq.ownID;
    std::string spacename = ipcreq.spacename;
    std::string newspacename = ipcreq.newspacename;
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
        for(const auto &it : zoneinfo.spaceBicInfo){
            if (it.spaceName == spacename){
                spaceuuid = it.spaceID;
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
    snprintf(url, 256, "http://%s:%d/space/rename",ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    SpaceRequest req;
    req.spaceID = spaceuuid;
    req.newSpaceName = newspacename;
    std::string value = req.serialize();

    // TODO: 发送间重命名请求，并输出结果
    auto response = client.post(url).body(value).send();
    std::promise<bool> prom;
    auto fu = prom.get_future();
    response.then(
            [&](Http::Response res) {
                std::cout << url << std::endl;
                std::cout << res.body() << std::endl; //结果
                prom.set_value(true);
            },Async::IgnoreException);
    fu.get();
    client.shutdown();
    return "success";
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
        for(const auto &it : zoneinfo.spaceBicInfo){
            if (it.spaceName == spacename){
                spaceuuid = it.spaceID;
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
    std::string ip = ipcreq.ip;
    int port = ipcreq.port;
    std::string zonename = ipcreq.zonename;
    std::string ownID = ipcreq.ownID;
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids = ipcreq.spaceuuids;
    std::string zoneuuid = ipcreq.zoneuuid;

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
            for(const auto &it : zoneinfo.spaceBicInfo){
                if (it.spaceName == m){
                    spaceuuids.push_back(it.spaceID);
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

    // TODO: 构造映射请求
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/zone/mapdeduct",ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);



    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID;
    req.spaceID = spaceuuids;
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
    std::string ownID = ipcreq.ownID;
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
    string response = client->rpc->post_request(client->get_manager(), "/zone/cancel", req.serialize());//TODO：修改按需选择管理节点
    return response;
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
    std::string ownID = ipcreq.ownID;
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
    return response;    
}

std::string ClientIPC::dozoneshare(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = ipcreq.ownID;
    std::vector<std::string> memID = ipcreq.memID;
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
    return response;  
}

std::string ClientIPC::dozonesharecancel(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = ipcreq.ownID;
    std::vector<std::string> memID = ipcreq.memID;
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
    return response;  
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
    string ip = ipcreq.ip;
    int port = ipcreq.port;
    string username = ipcreq.accountName;
    string password = ipcreq.Password;

    //客户端处理流程
    cout << "start endpoint: " << endl;
    string endpoint = client->get_manager();
    cout << endpoint << endl;
    cout << "end endpoint: " << endl;
    //string res = client->rpc->post_request(endpoint, "/resource/register", newRes.serialize());

    //账户登录
    Http::Client Pclient;
    char url[256];
    //snprintf(url, 256, "http://%s:%d/users/login", ip.c_str(), port);
    snprintf(url, 256, "http://%s:%d/users/login", ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    Pclient.init(opts);


    AccountPass myaccount;
    myaccount.accountName = username;
    myaccount.Password = password;
    std::string mes = myaccount.serialize();

    std::string mtoken;
    std::string return_value = "login fail";

    //auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(mes).send();
    auto response = Pclient.post(url).body(mes).send();
            //dout(-1) << "Client Info: post request " << url << dendl;

    //client->user->setToken("mtoken");
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
                client->user->setToken(mtoken);
                client->user->setAccountName(myaccount.accountName);
                client->user->setAccountID(body);
                cout << "getToken(): " << client->user->getToken() << endl;
                cout << "getAccountName(): " << client->user->getAccountName() << endl;
                cout << "getAccountID(): " << client->user->getAccountID() << endl;
                
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
    string ip = ipcreq.ip;
    int port = ipcreq.port;
    string username = ipcreq.accountName;

    //客户端处理流程
        //TODO获取username对应的 uuid   发get请求要用
        //这块调用客户端用户模块接口    登录成功返回的token存下来，返回的账户uuid存下来 ！！！
        // 并且登录完 要在客户端存 token  uuid 账户名这三个
        //client->user->getToken()
        //client->user->getAccountID()
        //client->user->getAccountName()     //int getMemberID(std::vector<std::string> Name, std::vector<std::string> memberID)

    //账户查询
    std::string return_value = "search fail";
    Http::Client Pclient;
    char url[256];
    snprintf(url, 256, "http://%s:%d/users/search/%s", ip.c_str(), port, client->user->getAccountID().c_str());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    Pclient.init(opts);

     auto response_1 = Pclient.get(url).cookie(Http::Cookie("token", "mtoken")).send();
        //dout(-1) << "Client Info: get request " << url << dendl;
    cout << "Client Info: get request " << url << endl;
    std::promise<bool> prom_1;
    auto fu_1 = prom_1.get_future();
    response_1.then(
        [&](Http::Response res) {
            //dout(-1) << "Manager Info: " << res.body() << dendl;
            std::cout << "Response code = " << res.code() << std::endl;
            //if code== 
            return_value = "search success";
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
    std::cout << "end: dousersearch" << std::endl;
    return return_value;
}



std::string ClientIPC::dousersignup(IPCreq &ipcreq){
    // TODO: 提前准备的数据
    string ip = ipcreq.ip;
    int port = ipcreq.port;

        
    string username = ipcreq.accountName; //账户名
    string hvsID = ipcreq.hvsID; //在服务端产生
    string pass = ipcreq.Password;
    string email = ipcreq.email;
    string phone = ipcreq.phone;
    string ad = ipcreq.address;
    string de = ipcreq.department;

    //账户注册
    std::string return_value = "signup fail";
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://%s:%d/users/registration", ip.c_str(), port);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    // Account person("lbq-9", "123456", "129", "XXXXXX@163.com", "15012349876", "xueyuanlu",  "Beihang");
    // std::string value = person.serialize();
    Account person(username, pass, hvsID, email, phone, ad,  de);   //以后在服务端产生uuid，客户端这块传值不影响
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
          // code==  
          return_value ="signup success";
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

 }


}//namespace