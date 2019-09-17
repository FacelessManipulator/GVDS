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
#include "hvs_struct.h"
#include "client/clientuser/ClientAuth_struct.h"
#include "client/zone_mod.h"


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
            if(ipcreq.cmdname == "spacerename_admin"){
                return dospacerename_admin(ipcreq);
            }
            if(ipcreq.cmdname == "spacesizechange"){
                return dospacesizechange(ipcreq);
            }
            if(ipcreq.cmdname == "spaceusage"){
                return dospaceusage(ipcreq);
            }
            if(ipcreq.cmdname == "spaceusage_admin"){
                return dospaceusage_admin(ipcreq);
            }
            if(ipcreq.cmdname == "mapadd"){
                return domapadd(ipcreq);
            }
            if(ipcreq.cmdname == "mapdeduct"){
                return domapdeduct(ipcreq);
            }
            if(ipcreq.cmdname == "mapdeduct_admin"){
                return domapdeduct_admin(ipcreq);
            }
            if(ipcreq.cmdname == "zoneadd_admin"){
                return dozoneadd_admin(ipcreq);
            }
            if(ipcreq.cmdname == "zonecancel"){
                return dozonecancel(ipcreq);
            }
            if(ipcreq.cmdname == "zonecancel_admin"){
                return dozonecancel_admin(ipcreq);
            }
            if(ipcreq.cmdname == "zoneregister"){
                return dozoneregister(ipcreq);
            }
            if(ipcreq.cmdname == "zonerename"){
                return dozonerename(ipcreq);
            }
            if(ipcreq.cmdname == "zonerename_admin"){
                return dozonerename_admin(ipcreq);
            }
            if(ipcreq.cmdname == "zoneshare"){
                return dozoneshare(ipcreq);
            }
            if(ipcreq.cmdname == "zoneshare_admin"){
                return dozoneshare_admin(ipcreq);
            }
            if(ipcreq.cmdname == "zonesharecancel"){
                return dozonesharecancel(ipcreq);
            }
            if(ipcreq.cmdname == "zonesharecancel_admin"){
                return dozonesharecancel_admin(ipcreq);
            }
            if(ipcreq.cmdname == "zonelist"){
                return dozonelist(ipcreq);
            }
            if(ipcreq.cmdname == "zonelist_admin"){
                return dozonelist_admin(ipcreq);
            }
            //auth
            if(ipcreq.cmdname == "userlogin"){
                return douserlogin(ipcreq);
            }
            if(ipcreq.cmdname == "usersearch"){
                return dousersearch(ipcreq);
            }
            if(ipcreq.cmdname == "usersignup"){
                return dousersignup(ipcreq);
            }
            if(ipcreq.cmdname == "usermodify"){
                return dousermodify(ipcreq);
            }
            if(ipcreq.cmdname == "userexit"){
                return douserexit(ipcreq);
            }
            if(ipcreq.cmdname == "usercancel"){
                return dousercancel(ipcreq);
            }
            if(ipcreq.cmdname == "authsearch"){
                return doauthsearch(ipcreq);
            }
            if(ipcreq.cmdname == "authmodify"){
                return doauthmodify(ipcreq);
            }
            if(ipcreq.cmdname == "modifycenter"){
                return domodifycenter(ipcreq);
            }
            if(ipcreq.cmdname == "searchcenter"){
                return dosearchcenter(ipcreq);
            }
            if(ipcreq.cmdname == "deletecenter"){
                return dodeletecenter(ipcreq);
            }
            //admin
            if(ipcreq.cmdname == "adminsignup"){
                return doadminsignup(ipcreq);
            }
            if(ipcreq.cmdname == "listapply"){
                return dolistapply(ipcreq);
            }
            if(ipcreq.cmdname == "suggestion"){
                return dosuggestion(ipcreq);
            }
            if(ipcreq.cmdname == "adcam"){
                return doadcam(ipcreq);
            }
            if(ipcreq.cmdname == "aduam"){
                return doaduam(ipcreq);
            }
            if(ipcreq.cmdname == "adsearcham"){
                return doadsearcham(ipcreq);
            }
            if(ipcreq.cmdname == "adseepool"){
                cout << "doadseepool here" <<endl;
                return doadseepool(ipcreq);
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
    if(ownID == "") return "login first";
    std::string ownName = client->user->getAccountName();
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
        if(zoneinfo.ownerID != ownName) return "权限不足";
        for(auto it : zoneinfo.spaceBicInfo){
            if (it->spaceName == newspacename) return "已存在的空间名";
        }
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

std::string ClientIPC::dospacerename_admin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if(ipcreq.ownName == "")
    {
        return "no ownername";
    }
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::string spacename = ipcreq.spacename;
    std::string newspacename = ipcreq.newspacename;
    std::string spaceuuid;
    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        auto zoneinfo = mapping->second;
        if(zoneinfo.ownerID != ownName[0]) return "权限不足";
        for(auto it : zoneinfo.spaceBicInfo){
            if (it->spaceName == newspacename) return "已存在的空间名";
        }
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

std::string ClientIPC::dospaceusage(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if(ownID == "") return "login first";
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;


    //获取区域信息
    cout << ownID << endl;
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        Zone zoneinfo = mapping->second;
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

    SpaceRequest req;
    req.spaceIDs = spaceuuids;

    string response = client->rpc->post_request(client->get_manager(), "/space/spaceusagecheck", req.serialize());//TODO：修改按需选择管理节点
    return "1" + response;
    
    // if (!result) return "success";
    // else return std::strerror(result);
}

std::string ClientIPC::dospaceusage_admin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if(ipcreq.ownName == "") return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;


    //获取区域信息
    int ret = GetZoneInfo(ownID[0]);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        Zone zoneinfo = mapping->second;
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

    SpaceRequest req;
    req.spaceIDs = spaceuuids;

    string response = client->rpc->post_request(client->get_manager(), "/space/spaceusagecheck", req.serialize());//TODO：修改按需选择管理节点
    return "1" + response;
    
    // if (!result) return "success";
    // else return std::strerror(result);
}

std::string ClientIPC::dospacesizechange(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if(ownID == "") return "login first";
    std::string spacename = ipcreq.spacename;
    int newspacesize = ipcreq.newspacesize;
    std::string spaceuuid;
    std::string centerid;
    
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
                centerid = it->hostCenterID;
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
    req.newSpaceSize = newspacesize;

    std::string centerinfo = client->optNode->getCenterInfo();
    CenterInfo mycenter;
    mycenter.deserialize(centerinfo);
    char url[256];
    //snprintf(url, 256, "http://%s:%s", mycenter.centerIP[centerid].c_str(), mycenter.centerPort[centerid].c_str());
    string response = client->rpc->post_request(client->get_manager(), "/space/changesizeapply", req.serialize());
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return "fail";//std::strerror(result); 
}

std::string ClientIPC::domapadd(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if(ownID == "") return "login first";
    std::string spacename = ipcreq.spacename;
    int64_t spacesize = ipcreq.spacesize;
    std::string spaceurl = ipcreq.spaceurl;
    Space space;
    space.deserialize(spaceurl);
    std::string zoneuuid;


    // TODO: 获取区域信息
    int ret = GetZoneInfo(ownID);
    if(!ret){
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if(mapping !=  zonemap.end()) {
        zoneuuid = mapping->second.zoneID;
        auto zoneinfo = mapping->second;
        for(auto it : zoneinfo.spaceBicInfo){
            if (it->spaceName == spacename) return "已存在的空间名";
        }
    } else{
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }
    if(spacename == "" || spacesize == 0){
        std::cerr << "信息不全，无法创建映射" << std::endl;
        return  "信息不全，无法创建映射";
    }
    else{
        ZoneRequest req;
        req.zoneID = zoneuuid;
        req.ownerID = ownID;
        req.spaceName = spacename;
        req.spaceSize = spacesize;
        if (space.hostCenterName == "")
        {
            auto centers = client->optNode->getNode(1);
            auto center = centers[0];
            space.hostCenterName = center.location;
            space.hostCenterID = center.center_id;
            req.spacePathInfo = space.serialize();
            string response = client->rpc->post_request(client->get_manager(), "/zone/mapaddapply", req.serialize());
            int result;
            json_decode(response, result);
            if (!result) return "success";
            else return std::strerror(result); 
            // for(int i = 0; i < centers.size(); i++)
            // {
            //     auto center = centers[i];
            //     char url[256];
            //     snprintf(url, 256, "http://%s:%s", center.ip_addr.c_str(), center.port.c_str());
            //     space.hostCenterName = center.location;
            //     req.spacePathInfo = space.serialize();
            //     string response = client->rpc->post_request(string(url), "/zone/mapaddapply", req.serialize());
            //     int result;
            //     json_decode(response, result);
            //     if (!result) 
            //     {
            //         res = "success";
            //         break;
            //     }
            //     else
            //     {
            //         res = std::strerror(result);
            //     } 
            // }
            // return res;
        }
        else
        {
            std::string centerinfo = client->optNode->getCenterInfo();
            CenterInfo mycenter;
            mycenter.deserialize(centerinfo); 
            std::string centerid = client->optNode->getmapIdName(space.hostCenterName);
            if (centerid == "fail") return "no such center";
            else{
                // char url[256];
                // snprintf(url, 256, "http://%s:%s", mycenter.centerIP[centerid].c_str(), mycenter.centerPort[centerid].c_str());
                space.hostCenterID = centerid;
                req.spacePathInfo = space.serialize();
                string response = client->rpc->post_request(client->get_manager(), "/zone/mapaddapply", req.serialize());
                int result;
                json_decode(response, result);
                if (!result) return "success";
                else return std::strerror(result);     
            }
        }
    }
}

std::string ClientIPC::domapdeduct(IPCreq &ipcreq) {
    // TODO: 提前准备的数据

    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if(ownID == "") return "login first";
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;
    std::string zoneuuid;

    //获取区域信息
    cout << ownID << endl;
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

std::string ClientIPC::domapdeduct_admin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据

    std::string zonename = ipcreq.zonename;
    if(ipcreq.ownName == "") return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;
    std::string zoneuuid;

    //获取区域信息

    int ret = GetZoneInfo(ownID[0]);
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
    req.ownerID = ownID[0];
    req.spaceID = spaceuuids;

    string response = client->rpc->post_request(client->get_manager(), "/zone/mapdeduct", req.serialize());//TODO：修改按需选择管理节点
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);
}

std::string ClientIPC::dozoneadd_admin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if(ipcreq.ownName == "") return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> memID;
    bool tm2 = client->user->getMemberID(ipcreq.memName, memID);
    if(!tm2){
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string spaceurl = ipcreq.spaceurl;
    Space space;
    space.deserialize(spaceurl);

    // TODO: 构造请求


    ZoneRequest req;
    req.zoneName = zonename;
    req.ownerID = ownID[0];
    req.memberID = memID;
    req.spacePathInfo = spaceurl;

    if (space.hostCenterName == "") return "no center name";
    else
    {
        if (space.storageSrcName == "") return "no storage name";
        else
        {
            std::string centerinfo = client->optNode->getCenterInfo();
            CenterInfo mycenter;
            mycenter.deserialize(centerinfo); 
            std::string centerid = client->optNode->getmapIdName(space.hostCenterName);
            if (centerid == "fail") return "no such center";
            else{
                // char url[256];
                // snprintf(url, 256, "http://%s:%s", mycenter.centerIP[centerid].c_str(), mycenter.centerPort[centerid].c_str());
                string response = client->rpc->post_request(client->get_manager(), "/zone/add", req.serialize());
                int result;
                json_decode(response, result);
                if (!result) return "success";
                else return std::strerror(result);     
            }
        }
    }
}


std::string ClientIPC::dozonecancel(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if(ownID == "") return "login first";
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

std::string ClientIPC::dozonecancel_admin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if(ipcreq.ownName == "") return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
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
    req.ownerID = ownID[0];
    string response = client->rpc->post_request(client->get_manager(), "/zone/cancel", req.serialize());//TODO：修改按需选择管理节点
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);
}

std::string ClientIPC::dozoneregister(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if(ownID == "") return "login first";
    std::vector<std::string> memID;
    bool tm = client->user->getMemberID(ipcreq.memName, memID);
    if(!tm){
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string spacename = ipcreq.spacename;
    int64_t spacesize = ipcreq.spacesize;
    std::string spaceurl = ipcreq.spaceurl;
    Space space;
    space.deserialize(spaceurl);

    ZoneRequest req;
    req.zoneName = zonename;
    req.ownerID = ownID;
    req.memberID = memID;
    req.spaceName = spacename;
    req.spaceSize = spacesize;



    if (space.hostCenterName == "")
    {
        auto centers = client->optNode->getNode(1);
        auto center = centers[0];
        space.hostCenterName = center.location;
        space.hostCenterID = center.center_id;
        req.spacePathInfo = space.serialize();
        std::cout << req.serialize() << std::endl;
        string response = client->rpc->post_request(client->get_manager(), "/zone/registerapply", req.serialize());
        int result;
        json_decode(response, result);
        if (!result) return "success";
        else return std::strerror(result); 
        // for(int i = 0; i < centers.size(); i++)
        // {
        //     auto center = centers[i];
        //     char url[256];
        //     snprintf(url, 256, "http://%s:%s", center.ip_addr.c_str(), center.port.c_str());
        //     space.hostCenterName = center.location;
        //     req.spacePathInfo = space.serialize();
        //     string response = client->rpc->post_request(string(url), "/zone/registerapply", req.serialize());
        //     int result;
        //     json_decode(response, result);
        //     cout << center.location << "  " << result << endl;
        //     if (!result) 
        //     {
        //         res = "success";
        //         break;
        //     }
        //     else
        //     {
        //         res = std::strerror(result);
        //     } 
        // }
        // return res;
    }
    else
    {
        std::string centerinfo = client->optNode->getCenterInfo();
        CenterInfo mycenter;
        mycenter.deserialize(centerinfo); 
        std::string centerid = client->optNode->getmapIdName(space.hostCenterName);
        if (centerid == "fail") return "no such center";
        else{
            // char url[256];
            // snprintf(url, 256, "http://%s:%s", mycenter.centerIP[centerid].c_str(), mycenter.centerPort[centerid].c_str());
            space.hostCenterID = centerid;
            req.spacePathInfo = space.serialize();
            string response = client->rpc->post_request(client->get_manager(), "/zone/registerapply", req.serialize());
            int result;
            json_decode(response, result);
            if (!result) return "success";
            else return std::strerror(result);        
        }
    }
}

std::string ClientIPC::dozonerename(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if(ownID == "") return "login first";
    std::string newzonename = ipcreq.newzonename;
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

std::string ClientIPC::dozonerename_admin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if(ipcreq.ownName == "") return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::string newzonename = ipcreq.newzonename;
    std::string zoneuuid;

        // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
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
    req.ownerID = ownID[0];
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
    if(ownID == "") return "login first";
    std::vector<std::string> memID;
    bool tm = client->user->getMemberID(ipcreq.memName, memID);
    if(!tm){
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
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
    req.memberID = memID;
    string response = client->rpc->post_request(client->get_manager(), "/zone/share", req.serialize());
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);   
}

std::string ClientIPC::dozoneshare_admin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if(ipcreq.ownName == "") return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> memID;
    bool tm2 = client->user->getMemberID(ipcreq.memName, memID);
    if(!tm2){
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
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
    req.ownerID = ownID[0];
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
    if(ownID == "") return "login first";
    std::vector<std::string> memID;
    bool tm = client->user->getMemberID(ipcreq.memName, memID);
    if(!tm){
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
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
    req.memberID = memID;
    string response = client->rpc->post_request(client->get_manager(), "/zone/sharecancel", req.serialize());
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);   
}

std::string ClientIPC::dozonesharecancel_admin(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if(ipcreq.ownName == "") return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> memID;
    bool tm2 = client->user->getMemberID(ipcreq.memName, memID);
    if(!tm2){
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
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
    req.ownerID = ownID[0];
    req.memberID = memID;
    string response = client->rpc->post_request(client->get_manager(), "/zone/sharecancel", req.serialize());
    int result;
    json_decode(response, result);
    if (!result) return "success";
    else return std::strerror(result);   
}

std::string ClientIPC::dozonelist(IPCreq &ipcreq) {
    std::string ownID = client->user->getAccountID();
    if(ownID == "") return "login first";
    std::string endpoint = client->get_manager();
    std::string inforesult = client->rpc->post_request(endpoint, "/zone/info", ownID);
    return inforesult;
}

std::string ClientIPC::dozonelist_admin(IPCreq &ipcreq) {
    if(ipcreq.ownName == "") return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if(!tm){
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::string endpoint = client->get_manager();
    std::string inforesult = client->rpc->post_request(endpoint, "/zone/info", ownID[0]);
    return inforesult;
}

// 调用获取区域信息；
bool ClientIPC::GetZoneInfo(std::string clientID) {
    vector<Zone> zoneinfores;
    string endpoint = client->get_manager();
    string inforesult = client->rpc->post_request(endpoint, "/zone/info", clientID);
    //cout << inforesult << endl;
    if (!inforesult.empty()) {
        json_decode(inforesult, zoneinfores); //获取返回的结果
    }
    if(zoneinfores.empty()){
        return false;
    }
    std::unordered_map<std::string, Zone> zonemap_new;
    for(const auto &it : zoneinfores) {
        zonemap_new[it.zoneName] = it;
    }
    std::lock_guard<std::mutex> lock(mutex);
    zonemap.swap(zonemap_new);
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

    // cout << myaccount.accountName << endl;

    std::string mtoken;
    std::string return_value = "-1";


    //auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(mes).send();
    auto response = Pclient.post(url).body(mes).send();
            //dout(-1) << "Client Info: post request " << url << dendl;

    std::promise<bool> prom;
    auto fu = prom.get_future();
    response.then(
        [&](Http::Response res) {
            // std::cout << "Response code = " << res.code() << std::endl;
            auto body = res.body();
            if (body == "-1"){
                // std::cout << "Response body = " << body << std::endl;
                return_value = "-1";
            }
            else{  //登录成果
                // std::cout<< "Response cookie = ";
                auto cookies = res.cookies();
                for (const auto& c: cookies) {
                    // std::cout << c.name << " : " << c.value << std::endl;
                    mtoken = c.value;
                }
                client->user->setToken(mtoken);
                client->user->setAccountName(myaccount.accountName);

                cout << "body: " << body << endl;
                cout << "body.substr(2): " << body.substr(2) << endl;
                cout << "body.substr(0,1): " << body.substr(0,1) << endl;

                client->user->setAccountID(body.substr(2));

                string hvsID = client->user->getAccountID();
                client->optNode->getAuthFromServer(hvsID);
                client->zone->GetZoneInfo(hvsID);
                // //【接口示例】                
                // cout << "getToken(): " << client->user->getToken() << endl;
                // cout << "getAccountName(): " << client->user->getAccountName() << endl;
                // cout << "getAccountID(): " << client->user->getAccountID() << endl;
                // cout << "centerName= Beijing   id= " << client->optNode->getmapIdName("Beijing") << endl;

                // std::vector<std::string> memberName; 
                // memberName.push_back("lbq-9");
                // memberName.push_back("lbq-8");
                // std::vector<std::string> memberID;
                // bool tm = client->user->getMemberID(memberName, memberID);
                // if(tm){
                //     for(int j=0; j<memberID.size(); j++){
                //         cout << "memID: " << memberID[j] << endl;
                //     }
                // } //login success
                //client->zone->GetZoneInfo("202");
                return_value = "0";
            }

            prom.set_value(true);
        },
        Async::IgnoreException);
    fu.get();
    
    Pclient.shutdown();

    // //【接口示例】 客户端获取权限代码
    // AuthSearch myauth = client->optNode->getAuthFromClient();
    // cout << "aaa: "<<myauth.hvsID << endl;
    // if(!myauth.hvsID.empty()){
    //     vector<string>::iterator iter;
    //     for (iter = myauth.vec_ZoneID.begin(); iter != myauth.vec_ZoneID.end(); iter++){
    //         cout << "区域id： " << *iter << endl;
    //         cout << "区域读权限： " << myauth.read[*iter] << endl;
    //         cout << "区域写权限： " << myauth.write[*iter] << endl;
    //         cout << "区域执行权限： "<< myauth.exe[*iter] << endl;  
    //         cout << "身份： ";
    //         if(myauth.isowner[*iter]=="1"){
    //             cout << "区域拥有者" << endl;  
    //         }
    //         else{
    //             cout << "区域成员" << endl;  
    //         }
    //         cout << endl;
    //     }

    // }
    

    return return_value;
}


std::string ClientIPC::dousersearch(IPCreq &ipcreq) {
    // TODO: 提前准备的数据
    string username = ipcreq.accountName;

    if(username != client->user->getAccountName() && !client->user->getAccountName().empty()){ //用户没有登录的时候是 false
        return "client_input_error";
    }

     //账户查询
    string return_value = "fail";
    string endpoint = client->get_manager();   
    string routepath = "/users/search/" + client->user->getAccountID();    ///users/search/用戶id
    string res = client->rpc->get_request(endpoint, routepath);

    cout << "response: " << res <<endl;
    return res;
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
    string routepath = "/users/bufferuserregister";    ///users/search/用戶id

    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "response: " << res <<endl;
    return res;
}
//管理员账户注册
std::string ClientIPC::doadminsignup(IPCreq &ipcreq){
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
    string routepath = "/users/adminregistration";    

    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "response: " << res <<endl;
    return res;
}

std::string ClientIPC::dousermodify(IPCreq &ipcreq){
 // TODO: 提前准备的数据

    string username = ipcreq.accountName; //账户名
    string hvsID = client->user->getAccountID(); //在服务端产生     // 不能修改
    string pass = ipcreq.Password;
    string email = ipcreq.email;
    string phone = ipcreq.phone;
    string ad = ipcreq.address;
    string de = ipcreq.department;

    if(username != client->user->getAccountName() && !client->user->getAccountName().empty()){ //用户没有登录的时候是 false
        return "client_input_error";
    }

  //账户修改
    Account person(username, pass, hvsID, email, phone, ad, de);
    std::string person_value = person.serialize();
    std::cout << person_value << std::endl;

    string endpoint = client->get_manager();   
    string routepath = "/users/modify";    
    string res = client->rpc->post_request(endpoint, routepath, person_value);

    cout << "response: " << res <<endl;
    return res;
}

std::string ClientIPC::douserexit(IPCreq &ipcreq){
    // TODO: 提前准备的数据
    string username = ipcreq.accountName; //账户名

    if(username != client->user->getAccountName() && !client->user->getAccountName().empty()){ //用户没有登录的时候是 false
        return "client_input_error";
    }

    string endpoint = client->get_manager();   
    string routepath = "/users/exit/" + client->user->getAccountID();    ///users/search/用戶id
    string res = client->rpc->get_request(endpoint, routepath);

    return res;
}

std::string ClientIPC::dousercancel(IPCreq &ipcreq){
     // TODO: 提前准备的数据
    string username = ipcreq.accountName; //账户名
    if(username != client->user->getAccountName() && !client->user->getAccountName().empty()){ //用户没有登录的时候是 false
        return "client_input_error";
    }


    string endpoint = client->get_manager();   
    string routepath = "/users/cancel/" + client->user->getAccountID();    ///users/search/用戶id
    string res = client->rpc->get_request(endpoint, routepath);

    return res;
}

//auth
 std::string ClientIPC::doauthsearch(IPCreq &ipcreq){
     // TODO: 提前准备的数据
    string username = ipcreq.accountName; //账户名
    if(username != client->user->getAccountName() && !client->user->getAccountName().empty()){ //用户没有登录的时候是 false
        return "client_input_error";
    }

     
    std::string hvsID = client->user->getAccountID(); //在服务端产生 

    //权限查询
    std::string return_value = "authsearch success";
    string endpoint = client->get_manager();
    string routepath = "/auth/search";
    string res = client->rpc->post_request(endpoint, routepath, hvsID);
  
    return res;
 }

 std::string ClientIPC::doauthmodify(IPCreq &ipcreq){
         // TODO: 提前准备的数据

    string username = ipcreq.accountName; //账户名
    if(username != client->user->getAccountName() && !client->user->getAccountName().empty()){ //用户没有登录的时候是 false
        return "-2";
    }
    
    FEAuthModifygroupinfo FEgroup;
    FEgroup.hvsID = client->user->getAccountID(); //在服务端产生 
    FEgroup.zonename = ipcreq.zonename;
    FEgroup.modify_groupauth = ipcreq.changeauth;

    string value = FEgroup.serialize();

    string endpoint = client->get_manager();
    string routepath = "/auth/modify";
    string res = client->rpc->post_request(endpoint, routepath, value);



    //++++++
    //修改完要获取最新权限
    string hvsID = client->user->getAccountID();
    client->optNode->getAuthFromServer(hvsID);
    //++++++
    if(res == "-1"){
        return "modify auth fail";
    }
    else if(res == "33"){
        return "Verification failed, access denied";
    }
    else{ //返回9
        return "modify auth success";
    }

 }

 std::string ClientIPC::domodifycenter(IPCreq &ipcreq){
          // TODO: 提前准备的数据

    FECenterInfo FEcenter;
    FEcenter.centerID = ipcreq.centerID;
    FEcenter.centerIP = ipcreq.centerIP;
    FEcenter.centerPort = ipcreq.centerPort;
    FEcenter.centerName = ipcreq.centerName;
    
    string value = FEcenter.serialize();

    string ip = *(HvsContext::get_context()->_config->get<std::string>("manager_addr"));
    string port = *(HvsContext::get_context()->_config->get<std::string>("manager_port"));
    string url = "http://" + ip + ":" + port;   //http://localhost:9090

    //string endpoint = client->get_manager();   
    string routepath = "/mconf/addCenter";    
    string res = client->rpc->post_request(url, routepath, value);

    return res;
 }

 std::string ClientIPC::dosearchcenter(IPCreq &ipcreq){
    // TODO: 提前准备的数据
    string endpoint = client->get_manager();   
    string routepath = "/mconf/searchCenter";    
    string res = client->rpc->get_request(endpoint, routepath);

    return res;
 }

std::string ClientIPC::dodeletecenter(IPCreq &ipcreq){
    // TODO: 提前准备的数据
    string value = ipcreq.centerID;

    cout << "value: " << value << endl;
    string endpoint = client->get_manager();   
    string routepath = "/mconf/deleteCenter";    
    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "res :"<< res <<endl;
    return res;
 }

 std::string ClientIPC::dolistapply(IPCreq &ipcreq){
    
    string hvsID = client->user->getAccountID(); 
    string endpoint = client->get_manager();   
    string routepath = "/users/listapply";    
    string res = client->rpc->post_request(endpoint, routepath, hvsID);

    if(res == "1"){
        return res;
    }
    else if(res == "33"){
        return res;
    }
    else{
        std::vector<std::string> my;
        json_decode(res, my);

        //先清空map
        client->user->clearApply();
        for(auto iter = my.begin(); iter!=my.end(); iter++){
            std::string con = *iter;
            struct_apply_info singel_content;
            singel_content.deserialize(con);

            client->user->setApply(singel_content.id, singel_content.data);
        }
        cout << "res: " << res << endl;
        return res;
    }
    
 }

 std::string ClientIPC::dosuggestion(IPCreq &ipcreq){
      // TODO: 提前准备的数据
    string sug = ipcreq.sug;
    string id = ipcreq.applyid;

    string endpoint = client->get_manager();

    if(sug == "reject"){
        string routepath = "/users/removeapply";    
        string res = client->rpc->post_request(endpoint, routepath, id);
        if(res == "0"){
            return "success";
        }
        else if(res =="1"){
            return "fail";
        }
        else{
            return "fail";
        }
    }
    else if (sug == "agree"){
        std::string tmp = id.substr(0,5);
        string value = client->user->getApply(id);

        std::cout << "tmp here :  " << tmp << std::endl;

        std::string res;
        std::string routepath;
        std::string new_route, new_res;
        if(tmp=="usign"){
            routepath = "/users/registration";
            res = client->rpc->post_request(endpoint, routepath, value);

            new_route = "/users/removeapply";
            new_res = client->rpc->post_request(endpoint, new_route, id);
        }
        else if(tmp=="zregi"){
            routepath = "/zone/registercheck";
            std::string response = client->rpc->post_request(endpoint, routepath, value);
            int result;
            json_decode(response, result);
            if (!result)
            {
                res = "success";
            }
            else
            {
                res = std::strerror(result);
            }
            new_route = "/users/removeapply";
            new_res = client->rpc->post_request(endpoint, new_route, id);
        }
        else if(tmp=="spadd"){
            routepath = "/zone/mapaddcheck";
            std::string response = client->rpc->post_request(endpoint, routepath, value);
            int result;
            json_decode(response, result);
            if (!result)
            {
                res = "success";
            }
            else
            {
                res = std::strerror(result);
            }
            new_route = "/users/removeapply";
            new_res = client->rpc->post_request(endpoint, new_route, id);
        }
        else if(tmp=="spsiz"){
            routepath = "/space/changesize";
            std::string response = client->rpc->post_request(endpoint, routepath, value);
            int result;
            json_decode(response, result);
            if (!result)
            {
                res = "success";
            }
            else
            {
                res = std::strerror(result);
            }
            new_route = "/users/removeapply";
            new_res = client->rpc->post_request(endpoint, new_route, id);
        }
        return res;
    }
    else{
        return "input error";
    }
 }

std::string ClientIPC::doadcam(IPCreq &ipcreq){
    string name = ipcreq.accountName;
    string centername = ipcreq.centerName;

    //获取 accountName 对应的hvsID
    std::vector<std::string> vec_name;
    vec_name.push_back(name);
    std::vector<std::string> memID;
    bool tmsuccess = client->user->getMemberID(vec_name, memID);
    if(!tmsuccess){
        std::cerr << "未获得对应账户信息，请确认信息正确！" << std::endl;
        return "未获得对应账户信息，请确认信息正确！";
    }

    struct_AdminAccountMap new_accountmap;
    new_accountmap.adhvsID = client->user->getAccountID();
    cout << "memID[0] :"<< memID[0] <<endl;
    new_accountmap.hvsID = memID[0];
    new_accountmap.hostCenterName = centername;
    string value = new_accountmap.serialize();

    string endpoint = client->get_manager();   
    string routepath = "/users/adcam";    
    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "res :"<< res <<endl;
    return res;
}

std::string ClientIPC::doaduam(IPCreq &ipcreq){
    string name = ipcreq.accountName;
    string centername = ipcreq.centerName;

    //获取 accountName 对应的hvsID
    std::vector<std::string> vec_name;
    vec_name.push_back(name);
    std::vector<std::string> memID;
    bool tmsuccess = client->user->getMemberID(vec_name, memID);
    if(!tmsuccess){
        std::cerr << "未获得对应账户信息，请确认信息正确！" << std::endl;
        return "未获得对应账户信息，请确认信息正确！";
    }

    struct_AdminAccountMap new_accountmap;
    new_accountmap.adhvsID = client->user->getAccountID();
    cout << "memID[0] :"<< memID[0] <<endl;
    new_accountmap.hvsID = memID[0];
    new_accountmap.hostCenterName = centername;
    string value = new_accountmap.serialize();

    string endpoint = client->get_manager();   
    string routepath = "/users/aduam";    
    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "res :"<< res <<endl;
    return res;
}

std::string ClientIPC::doadsearcham(IPCreq &ipcreq){
    string name = ipcreq.accountName;

    //获取 accountName 对应的hvsID
    std::vector<std::string> vec_name;
    vec_name.push_back(name);
    std::vector<std::string> memID;
    bool tmsuccess = client->user->getMemberID(vec_name, memID);
    if(!tmsuccess){
        std::cerr << "未获得对应账户信息，请确认信息正确！" << std::endl;
        return "-1";
    }

    struct_AdminAccountMap new_accountmap;
    new_accountmap.adhvsID = client->user->getAccountID();  //json需要adhvsID
    cout << "memID[0] :"<< memID[0] <<endl;
    new_accountmap.hvsID = memID[0];                          //json需要hvsID
    new_accountmap.hostCenterName = "";                 //json这块不需要
    string value = new_accountmap.serialize();

    string endpoint = client->get_manager();   
    string routepath = "/users/adsearcham";    
    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "res :"<< res <<endl;
    return res;
}

std::string ClientIPC::doadseepool(IPCreq &ipcreq){
    string adhvsID = client->user->getAccountID();

    string endpoint = client->get_manager();   
    string routepath = "/users/adsearchpool";    
    string res = client->rpc->post_request(endpoint, routepath, adhvsID);

    cout << "res :"<< res <<endl;
    return res;
}

}//namespace