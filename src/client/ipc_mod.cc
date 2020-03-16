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
#include "gvds_struct.h"
#include "client/clientuser/ClientAuth_struct.h"
#include "client/zone_mod.h"

#include <errno.h>

using namespace gvds;
using namespace Pistache;
using namespace std;

namespace gvds
{
void ClientIPC::start()
{
    sp_ipcserver->run(); // ipc 服务器启动
}

void ClientIPC::stop()
{
    sp_ipcserver->stop(); // ipc 服务器停止
}

void ClientIPC::init()
{
    auto _config = HvsContext::get_context()->_config;
    int listen_port = _config->get<int>("client.listen_port").value_or(6666);
    try
    {
        
        sp_ipcserver = std::make_shared<IPCServer>(listen_port); // TODO : 端口需要再次确定！
        sp_ipcserver->set_callback_func([&](IPCMessage msg) -> std::string {
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
            if (ipcreq.cmdname == "spacerename")
            {
                return dospacerename(ipcreq);
            }
            if (ipcreq.cmdname == "spacerename_admin")
            {
                return dospacerename_admin(ipcreq);
            }
            if (ipcreq.cmdname == "spacesizechange")
            {
                return dospacesizechange(ipcreq);
            }
            if (ipcreq.cmdname == "spaceusage")
            {
                return dospaceusage(ipcreq);
            }
            if (ipcreq.cmdname == "spaceusage_admin")
            {
                return dospaceusage_admin(ipcreq);
            }
            if (ipcreq.cmdname == "mapadd")
            {
                return domapadd(ipcreq);
            }
            if (ipcreq.cmdname == "mapdeduct")
            {
                return domapdeduct(ipcreq);
            }
            if (ipcreq.cmdname == "mapdeduct_admin")
            {
                return domapdeduct_admin(ipcreq);
            }
            if (ipcreq.cmdname == "zoneadd_admin")
            {
                return dozoneadd_admin(ipcreq);
            }
            if (ipcreq.cmdname == "zonecancel")
            {
                return dozonecancel(ipcreq);
            }
            if (ipcreq.cmdname == "zonecancel_admin")
            {
                return dozonecancel_admin(ipcreq);
            }
            if (ipcreq.cmdname == "zoneregister")
            {
                return dozoneregister(ipcreq);
            }
            if (ipcreq.cmdname == "zonerename")
            {
                return dozonerename(ipcreq);
            }
            if (ipcreq.cmdname == "zonerename_admin")
            {
                return dozonerename_admin(ipcreq);
            }
            if (ipcreq.cmdname == "zoneshare")
            {
                return dozoneshare(ipcreq);
            }
            if (ipcreq.cmdname == "zoneshare_admin")
            {
                return dozoneshare_admin(ipcreq);
            }
            if (ipcreq.cmdname == "zonesharecancel")
            {
                return dozonesharecancel(ipcreq);
            }
            if (ipcreq.cmdname == "zonesharecancel_admin")
            {
                return dozonesharecancel_admin(ipcreq);
            }
            if (ipcreq.cmdname == "zonelist")
            {
                return dozonelist(ipcreq);
            }
            if (ipcreq.cmdname == "zonelist_admin")
            {
                return dozonelist_admin(ipcreq);
            }
            //auth
            if (ipcreq.cmdname == "userlogin")
            {
                return douserlogin(ipcreq);
            }
            if (ipcreq.cmdname == "usersearch")
            {
                return dousersearch(ipcreq);
            }
            if (ipcreq.cmdname == "usersignup")
            {
                return dousersignup(ipcreq);
            }
            if (ipcreq.cmdname == "usermodify")
            {
                return dousermodify(ipcreq);
            }
            if (ipcreq.cmdname == "userexit")
            {
                return douserexit(ipcreq);
            }
            if (ipcreq.cmdname == "usercancel")
            {
                return dousercancel(ipcreq);
            }
            if (ipcreq.cmdname == "authsearch")
            {
                return doauthsearch(ipcreq);
            }
            if (ipcreq.cmdname == "authmodify")
            {
                return doauthmodify(ipcreq);
            }
            if (ipcreq.cmdname == "modifycenter")
            {
                return domodifycenter(ipcreq);
            }
            if (ipcreq.cmdname == "searchcenter")
            {
                return dosearchcenter(ipcreq);
            }
            if (ipcreq.cmdname == "deletecenter")
            {
                return dodeletecenter(ipcreq);
            }
            //admin
            if (ipcreq.cmdname == "adminsignup")
            {
                return doadminsignup(ipcreq);
            }
            if (ipcreq.cmdname == "listapply")
            {
                return dolistapply(ipcreq);
            }
            if (ipcreq.cmdname == "suggestion")
            {
                return dosuggestion(ipcreq);
            }
            if (ipcreq.cmdname == "adcam")
            {
                return doadcam(ipcreq);
            }
            if (ipcreq.cmdname == "aduam")
            {
                return doaduam(ipcreq);
            }
            if (ipcreq.cmdname == "adsearcham")
            {
                return doadsearcham(ipcreq);
            }
            if (ipcreq.cmdname == "adseepool")
            {
                return doadseepool(ipcreq);
            }
            if (ipcreq.cmdname == "adauthsearch")
            {
                return doadauthsearch(ipcreq);
            }
            if (ipcreq.cmdname == "adauthmodify")
            {
                cout << "doadauthmodify here" << endl;
                return doadauthmodify(ipcreq);
            }

   

            //resource register
            if (ipcreq.cmdname == "resourceregister")
            {
                cout << "resource register..." << endl;
                return doresourceregister(ipcreq);
            }
            //resource delete
            if (ipcreq.cmdname == "resourcedelete")
            {
                cout << "resource delete..." << endl;
                return doresourcedelete(ipcreq);
            }
            //resource query
            if (ipcreq.cmdname == "resourcequery")
            {
                cout << "resource query..." << endl;
                return doresourcequery(ipcreq);
            }
            //resource update
            if (ipcreq.cmdname == "resourceupdate")
            {
                cout << "resource update..." << endl;
                return doresourceupdate(ipcreq);
            }
            //space replica
            if (ipcreq.cmdname == "spacereplica")
            {
                cout << "replicating spaces..." << endl;
                return dospacereplica(ipcreq);
            }

            else
            {
                std::cerr << "警告：出现不支持的命令的请求！" << std::endl;
                return "警告：出现不支持的命令的请求！";
            }
        });
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

std::string ClientIPC::doregisterupdate(IPCreq &ipcreq, std::string url)
{
    StorageResource newRes;
    newRes.storage_src_id = ipcreq.storage_src_id;     // 存储资源UUID
    newRes.storage_src_name = ipcreq.storage_src_name; // 存储资源名称
    newRes.host_center_id = ipcreq.host_center_id;     // 存储资源所在超算中心UUID
    newRes.host_center_name = ipcreq.host_center_name; // 存储资源所在超算中心名称
    newRes.total_capacity = ipcreq.total_capacity;     // 存储资源空间容量大小
    newRes.assign_capacity = ipcreq.assign_capacity;   // 已经分配空间容量大小
    newRes.mgs_address = ipcreq.mgs_address;           // 存储资源MGS地址
    newRes.state = (StorageResState)ipcreq.state;      // 存储资源状态

    std::string cinfor = client->optNode->getCenterInfo();
    CenterInfo mycenter;
    mycenter.deserialize(cinfor);
    bool flag = false;
    for (vector<string>::iterator iter = mycenter.centerID.begin(); iter != mycenter.centerID.end(); iter++)
    {
        string centername = mycenter.centerName[*iter];
        string centerid = *iter;
        if (newRes.host_center_name.c_str()[0] != '#' && newRes.host_center_id.c_str()[0] != '#')
        {
            if (centername == newRes.host_center_name && centerid == newRes.host_center_id)
            {
                flag = true;
                break;
            }
        }
        else if (centerid == newRes.host_center_id || centername == newRes.host_center_name)
        {
            newRes.host_center_id =centerid;
            newRes.host_center_name=centername;
            flag = true;
            break;
        }
    }

    if (!flag)
        return "input center id or center name is wrong";
    string endpoint = client->get_manager();
    string res = client->rpc->post_request(endpoint, url, newRes.serialize());
    cout<<endpoint;
    return res;
}

std::string ClientIPC::doresourceupdate(IPCreq &ipcreq)
{

    return doregisterupdate(ipcreq, "/resource/update");
}

std::string ClientIPC::doresourcequery(IPCreq &ipcreq)
{
    string url = "/resource/query/" + ipcreq.storage_src_id;
    string endpoint = client->get_manager();
    string res = client->rpc->get_request(endpoint, url);
    return res;
}

std::string ClientIPC::doresourcedelete(IPCreq &ipcreq)
{
    string url = "/resource/delete/" + ipcreq.storage_src_id;
    string endpoint = client->get_manager();
    string res = client->rpc->delete_request(endpoint, url);
    return res;
}

std::string ClientIPC::doresourceregister(IPCreq &ipcreq)
{

    return doregisterupdate(ipcreq, "/resource/register");
}

std::string ClientIPC::dospacerename(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::string ownName = client->user->getAccountName();
    std::string spacename = ipcreq.spacename;
    std::string newspacename = ipcreq.newspacename;
    std::string spaceuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        auto zoneinfo = mapping->second;
        if (zoneinfo.ownerID != ownName)
            return "权限不足";
        for (auto it : zoneinfo.spaceBicInfo)
        {
            if (it->spaceName == newspacename)
                return "已存在的空间名";
        }
        for (auto it : zoneinfo.spaceBicInfo)
        {
            if (it->spaceName == spacename)
            {
                spaceuuid = it->spaceID;
                break;
            }
        }
        if (spaceuuid.empty())
        {
            std::cerr << "空间名不存在，请确认空间名称正确！" << std::endl;
            return "空间名不存在，请确认空间名称正确！";
        }
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    SpaceRequest req;
    req.spaceID = spaceuuid;
    req.newSpaceName = newspacename;
    string response = client->rpc->post_request(client->get_manager(), "/space/rename", req.serialize());
    if(response == "success") return response;
    else return response + "空间不存在或访问数据库失败";
}

std::string ClientIPC::dospacerename_admin(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if (ipcreq.ownName == "")
    {
        return "no ownername";
    }
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::string spacename = ipcreq.spacename;
    std::string newspacename = ipcreq.newspacename;
    std::string spaceuuid;
    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        auto zoneinfo = mapping->second;
        if (zoneinfo.ownerID != ownName[0])
            return "权限不足";
        for (auto it : zoneinfo.spaceBicInfo)
        {
            if (it->spaceName == newspacename)
                return "已存在的空间名";
        }
        for (auto it : zoneinfo.spaceBicInfo)
        {
            if (it->spaceName == spacename)
            {
                spaceuuid = it->spaceID;
                break;
            }
        }
        if (spaceuuid.empty())
        {
            std::cerr << "空间名不存在，请确认空间名称正确！" << std::endl;
            return "空间名不存在，请确认空间名称正确！";
        }
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    SpaceRequest req;
    req.spaceID = spaceuuid;
    req.newSpaceName = newspacename;
    string response = client->rpc->post_request(client->get_manager(), "/space/rename", req.serialize());
    if(response == "success") return response;
    else return response + "空间不存在或访问数据库失败";
}

std::string ClientIPC::dospaceusage(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;

    //获取区域信息
    cout << ownID << endl;
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        Zone zoneinfo = mapping->second;
        for (const auto &m : spacenames)
        {
            bool found = false;
            for (const auto it : zoneinfo.spaceBicInfo)
            {
                if (it->spaceName == m)
                {
                    spaceuuids.push_back(it->spaceID);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cerr << "空间名" << m << "不存在，请确认空间名称正确！" << std::endl;
                return "空间名" + m + "不存在，请确认空间名称正确！";
            }
        }
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    SpaceRequest req;
    req.spaceIDs = spaceuuids;

    string response = client->rpc->post_request(client->get_manager(), "/space/spaceusagecheck", req.serialize()); //TODO：修改按需选择管理节点
    return "1" + response;

    // if (!result) return "success";
    // else return std::strerror(result);
}

std::string ClientIPC::dospaceusage_admin(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if (ipcreq.ownName == "")
        return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;

    //获取区域信息
    int ret = GetZoneInfo(ownID[0]);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        Zone zoneinfo = mapping->second;
        for (const auto &m : spacenames)
        {
            bool found = false;
            for (const auto it : zoneinfo.spaceBicInfo)
            {
                if (it->spaceName == m)
                {
                    spaceuuids.push_back(it->spaceID);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cerr << "空间名" << m << "不存在，请确认空间名称正确！" << std::endl;
                return "空间名" + m + "不存在，请确认空间名称正确！";
            }
        }
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    SpaceRequest req;
    req.spaceIDs = spaceuuids;

    string response = client->rpc->post_request(client->get_manager(), "/space/spaceusagecheck", req.serialize()); //TODO：修改按需选择管理节点
    return "1" + response;

    // if (!result) return "success";
    // else return std::strerror(result);
}

std::string ClientIPC::dospacesizechange(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::string spacename = ipcreq.spacename;
    int newspacesize = ipcreq.newspacesize;
    std::string spaceuuid;
    std::string centerid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        Zone zoneinfo = mapping->second;
        for (const auto it : zoneinfo.spaceBicInfo)
        {
            if (it->spaceName == spacename)
            {
                spaceuuid = it->spaceID;
                centerid = it->hostCenterID;
                break;
            }
        }
        if (spaceuuid.empty())
        {
            std::cerr << "空间名不存在，请确认空间名称正确！" << std::endl;
            return "空间名不存在，请确认空间名称正确！";
        }
    }
    else
    {
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
    if (!result)
        return "success";
    else
        return "数据库连接失败，请重试"; //std::strerror(result);
}

std::string ClientIPC::domapadd(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::string spacename = ipcreq.spacename;
    int64_t spacesize = ipcreq.spacesize;
    std::string spaceurl = ipcreq.spaceurl;
    Space space;
    space.deserialize(spaceurl);
    std::string zoneuuid;

    // TODO: 获取区域信息
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
        auto zoneinfo = mapping->second;
        for (auto it : zoneinfo.spaceBicInfo)
        {
            if (it->spaceName == spacename)
                return "已存在的空间名";
        }
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }
    if (spacename == "" || spacesize == 0)
    {
        std::cerr << "信息不全，无法创建映射" << std::endl;
        return "信息不全，无法创建映射";
    }
    else
    {
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
            if (!result)
                return "success";
            else
                return "数据库连接失败，请重试";
        }
        else
        {
            std::string centerinfo = client->optNode->getCenterInfo();
            CenterInfo mycenter;
            mycenter.deserialize(centerinfo);
            std::string centerid = client->optNode->getmapIdName(space.hostCenterName);
            if (centerid == "fail")
                return "no such center";
            else
            {
                // char url[256];
                // snprintf(url, 256, "http://%s:%s", mycenter.centerIP[centerid].c_str(), mycenter.centerPort[centerid].c_str());
                space.hostCenterID = centerid;
                req.spacePathInfo = space.serialize();
                string response = client->rpc->post_request(client->get_manager(), "/zone/mapaddapply", req.serialize());
                int result;
                json_decode(response, result);
                if (!result)
                    return "success";
                else
                    return "数据库连接失败，请重试";
            }
        }
    }
}

std::string ClientIPC::domapdeduct(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据

    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;
    std::string zoneuuid;

    //获取区域信息
    cout << ownID << endl;
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        Zone zoneinfo = mapping->second;
        zoneuuid = zoneinfo.zoneID;
        for (const auto &m : spacenames)
        {
            bool found = false;
            for (const auto it : zoneinfo.spaceBicInfo)
            {
                if (it->spaceName == m)
                {
                    spaceuuids.push_back(it->spaceID);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cerr << "空间名" << m << "不存在，请确认空间名称正确！" << std::endl;
                return "空间名" + m + "不存在，请确认空间名称正确！";
            }
        }
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID;
    req.spaceID = spaceuuids;

    string response = client->rpc->post_request(client->get_manager(), "/zone/mapdeduct", req.serialize()); //TODO：修改按需选择管理节点
    int result;
    json_decode(response, result);
    if (!result)
        return "success";
    else if( result==11 )
        return "数据库连接失败或空间权限删除失败请重试";
	else if (result == 22)
		return "空间名不存在";
	else if (result == 13)
		return "并非区域主人无权删除空间";
    else
        return "未知错误";
    
}

std::string ClientIPC::domapdeduct_admin(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据

    std::string zonename = ipcreq.zonename;
    if (ipcreq.ownName == "")
        return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> spacenames = ipcreq.spacenames;
    std::vector<std::string> spaceuuids;
    std::string zoneuuid;

    //获取区域信息

    int ret = GetZoneInfo(ownID[0]);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        Zone zoneinfo = mapping->second;
        zoneuuid = zoneinfo.zoneID;
        for (const auto &m : spacenames)
        {
            bool found = false;
            for (const auto it : zoneinfo.spaceBicInfo)
            {
                if (it->spaceName == m)
                {
                    spaceuuids.push_back(it->spaceID);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cerr << "空间名" << m << "不存在，请确认空间名称正确！" << std::endl;
                return "空间名" + m + "不存在，请确认空间名称正确！";
            }
        }
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID[0];
    req.spaceID = spaceuuids;

    string response = client->rpc->post_request(client->get_manager(), "/zone/mapdeduct", req.serialize()); //TODO：修改按需选择管理节点
    int result;
    json_decode(response, result);
    if (!result)
        return "success";
    else if( result==11 )
        return "数据库连接失败或空间权限删除失败请重试";
	else if (result == 22)
		return "空间名不存在";
	else if (result == 13)
		return "并非区域主人无权删除空间";
    else
        return "未知错误";
}

std::string ClientIPC::dozoneadd_admin(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if (ipcreq.ownName == "")
        return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> memID;
    bool tm2 = client->user->getMemberID(ipcreq.memName, memID);
    if (!tm2)
    {
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

    if (space.hostCenterName == "")
        return "no center name";
    else
    {
        if (space.storageSrcName == "")
            return "no storage name";
        else
        {
            std::string centerinfo = client->optNode->getCenterInfo();
            CenterInfo mycenter;
            mycenter.deserialize(centerinfo);
            std::string centerid = client->optNode->getmapIdName(space.hostCenterName);
            if (centerid == "fail")
                return "no such center";
            else
            {
                // char url[256];
                // snprintf(url, 256, "http://%s:%s", mycenter.centerIP[centerid].c_str(), mycenter.centerPort[centerid].c_str());
                string response = client->rpc->post_request(client->get_manager(), "/zone/add", req.serialize());
                int result;
                json_decode(response, result);
                if (!result)
                    return "success";
                else if (result == 22)
                    return "未在实际路径中找到相应空间或数据库中存在多个同命名的区域";
				else if (result == 11)
					return "数据库连接失败或权限添加失败请重试或联系管理员";
                else
                    return "未知错误";
                
            }
        }
    }
}

std::string ClientIPC::dozonecancel(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID;
    string response = client->rpc->post_request(client->get_manager(), "/zone/cancel", req.serialize()); //TODO：修改按需选择管理节点
    int result;
    json_decode(response, result);
    if (!result)
        return "success";
	else if (result == 11)
		return "数据库连接失败或权限删除失败请重试或联系管理员";
	else if (result == 13)
		return "不是管理员或区域主人，没有权限删除区域信息";
	else if (result == 2)
		return "未找到对应区域信息";
    else
        return "未知错误";
}

std::string ClientIPC::dozonecancel_admin(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if (ipcreq.ownName == "")
        return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
    }
    else
    {
        std::cerr << "区域名不存在，请确认区域名称正确！" << std::endl;
        return "区域名不存在，请确认区域名称正确！";
    }

    ZoneRequest req;
    req.zoneID = zoneuuid;
    req.ownerID = ownID[0];
    string response = client->rpc->post_request(client->get_manager(), "/zone/cancel", req.serialize()); //TODO：修改按需选择管理节点
    int result;
    json_decode(response, result);
    if (!result)
        return "success";
	else if (result == 11)
		return "数据库连接失败或权限删除失败请重试或联系管理员";
	else if (result == 13)
		return "不是管理员或区域主人，没有权限删除区域信息";
	else if (result == 2)
		return "未找到对应区域信息";
    else
        return "未知错误";
}

std::string ClientIPC::dozoneregister(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::vector<std::string> memID;
    bool tm = client->user->getMemberID(ipcreq.memName, memID);
    if (!tm)
    {
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    for(std::vector<std::string>::iterator it = memID.begin(); it != memID.end(); it++)
    {
        if(*it == ownID) return "不能见区域主人作为成员添加";
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
        if (!result)
            return "success";
        else
            return "数据库连接失败，请重试";
    }
    else
    {
        std::string centerinfo = client->optNode->getCenterInfo();
        CenterInfo mycenter;
        mycenter.deserialize(centerinfo);
        std::string centerid = client->optNode->getmapIdName(space.hostCenterName);
        if (centerid == "fail")
            return "no such center";
        else
        {
            // char url[256];
            // snprintf(url, 256, "http://%s:%s", mycenter.centerIP[centerid].c_str(), mycenter.centerPort[centerid].c_str());
            space.hostCenterID = centerid;
            req.spacePathInfo = space.serialize();
            string response = client->rpc->post_request(client->get_manager(), "/zone/registerapply", req.serialize());
            int result;
            json_decode(response, result);
            if (!result)
                return "success";
            else
                return "数据库连接失败，请重试";
        }
    }
}

std::string ClientIPC::dozonerename(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::string newzonename = ipcreq.newzonename;
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
    }
    else
    {
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
    if (!result)
        return "success";
    else if( result==11 )
        return "数据库连接失败请重试";
	else if (result == 22)
		return "已存在同名区域";
	else if (result == 13)
		return "并非区域主人无权重命名";
    else
        return "未知错误";
}

std::string ClientIPC::dozonerename_admin(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if (ipcreq.ownName == "")
        return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::string newzonename = ipcreq.newzonename;
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
    }
    else
    {
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
    if (!result)
        return "success";
    else if( result==11 )
        return "数据库连接失败请重试";
	else if (result == 22)
		return "已存在同名区域";
	else if (result == 13)
		return "并非区域主人或管理员无权重命名";
    else
        return "未知错误";
}

std::string ClientIPC::dozoneshare(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::vector<std::string> memID;
    bool tm = client->user->getMemberID(ipcreq.memName, memID);
    if (!tm)
    {
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
    }
    else
    {
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
    if (!result)
        return "success";
	else if (result == 11)
		return "数据库连接失败或区域成员权限添加失败请重试或联系管理员";
	else if (result == 13)
		return "非区域主人或管理员，无权进行区域共享";
    else
        return "未知错误";
}

std::string ClientIPC::dozoneshare_admin(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if (ipcreq.ownName == "")
        return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> memID;
    bool tm2 = client->user->getMemberID(ipcreq.memName, memID);
    if (!tm2)
    {
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
    }
    else
    {
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
    if (!result)
        return "success";
	else if (result == 11)
		return "数据库连接失败或区域成员权限添加失败请重试或联系管理员";
	else if (result == 13)
		return "非区域主人或管理员，无权进行区域共享";
    else
        return "未知错误";
}

std::string ClientIPC::dozonesharecancel(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::vector<std::string> memID;
    bool tm = client->user->getMemberID(ipcreq.memName, memID);
    if (!tm)
    {
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
    }
    else
    {
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
    if (!result)
        return "success";
	else if (result == 11)
		return "数据库连接失败或区域成员权限删除失败请重试或联系管理员";
	else if (result == 13)
		return "非区域主人或管理员，无权进行区域共享取消";
	else if (result == 22)
		return "某个要删除的成员并非区域成员";
	else
       return "未知错误";
}

std::string ClientIPC::dozonesharecancel_admin(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    std::string zonename = ipcreq.zonename;
    if (ipcreq.ownName == "")
        return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::vector<std::string> memID;
    bool tm2 = client->user->getMemberID(ipcreq.memName, memID);
    if (!tm2)
    {
        std::cerr << "未获得对应成员信息，请确认信息正确！" << std::endl;
        return "未获得对应成员信息，请确认信息正确！";
    }
    std::string zoneuuid;

    // TODO: 获取区域信息，并根据空间名获取空间UUID
    int ret = GetZoneInfo(ownID[0]);
    if (!ret)
    {
        std::cerr << "未获得对应区域信息，请确认账户信息正确！" << std::endl;
        return "未获得对应区域信息，请确认账户信息正确！";
    }

    auto mapping = zonemap.find(zonename);
    if (mapping != zonemap.end())
    {
        zoneuuid = mapping->second.zoneID;
    }
    else
    {
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
    if (!result)
        return "success";
	else if (result == 11)
		return "数据库连接失败或区域成员权限删除失败请重试或联系管理员";
	else if (result == 13)
		return "非区域主人或管理员，无权进行区域共享取消";
	else if (result == 22)
		return "某个要删除的成员并非区域成员";
	else
       return "未知错误";
}

std::string ClientIPC::dozonelist(IPCreq &ipcreq)
{
    std::string ownID = client->user->getAccountID();
    if (ownID == "")
        return "login first";
    std::string endpoint = client->get_manager();
    std::string inforesult = client->rpc->post_request(endpoint, "/zone/info", ownID);
    return inforesult;
}

std::string ClientIPC::dozonelist_admin(IPCreq &ipcreq)
{
    if (ipcreq.ownName == "")
        return " no ownername";
    std::vector<std::string> ownID;
    std::vector<std::string> ownName;
    ownName.push_back(ipcreq.ownName);
    bool tm = client->user->getMemberID(ownName, ownID);
    if (!tm)
    {
        std::cerr << "未获得对应主人信息，请确认信息正确！" << std::endl;
        return "未获得对应主人信息，请确认信息正确！";
    }
    std::string endpoint = client->get_manager();
    std::string inforesult = client->rpc->post_request(endpoint, "/zone/info", ownID[0]);
    return inforesult;
}

// 调用获取区域信息；
bool ClientIPC::GetZoneInfo(std::string clientID)
{
    vector<Zone> zoneinfores;
    string endpoint = client->get_manager();
    string inforesult = client->rpc->post_request(endpoint, "/zone/info", clientID);
    //cout << inforesult << endl;
    if (!inforesult.empty())
    {
        json_decode(inforesult, zoneinfores); //获取返回的结果
    }
    if (zoneinfores.empty())
    {
        return false;
    }
    std::unordered_map<std::string, Zone> zonemap_new;
    for (const auto &it : zoneinfores)
    {
        zonemap_new[it.zoneName] = it;
    }
    std::lock_guard<std::mutex> lock(mutex);
    zonemap.swap(zonemap_new);
    return true;
}

//user客户端函数
std::string ClientIPC::douserlogin(IPCreq &ipcreq)
{
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
            if (body == "-1")
            {
                // std::cout << "Response body = " << body << std::endl;
                return_value = "-1";
            }
            else
            { //登录成果
                // std::cout<< "Response cookie = ";
                auto cookies = res.cookies();
                for (const auto &c : cookies)
                {
                    // std::cout << c.name << " : " << c.value << std::endl;
                    mtoken = c.value;
                }
                client->user->setToken(mtoken);
                client->user->setAccountName(myaccount.accountName);

                cout << "body: " << body << endl;
                cout << "body.substr(2): " << body.substr(2) << endl;
                cout << "body.substr(0,1): " << body.substr(0, 1) << endl;

                client->user->setAccountID(body.substr(2));

                string gvdsID = client->user->getAccountID();
                client->optNode->getAuthFromServer(gvdsID);
                client->zone->GetZoneInfo(gvdsID);
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
    // cout << "aaa: "<<myauth.gvdsID << endl;
    // if(!myauth.gvdsID.empty()){
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

std::string ClientIPC::dousersearch(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    string username = ipcreq.accountName;

    if (username != client->user->getAccountName() && !client->user->getAccountName().empty())
    { //用户没有登录的时候是 false
        return "client_input_error";
    }

    //账户查询
    string return_value = "fail";
    string endpoint = client->get_manager();
    string routepath = "/users/search/" + client->user->getAccountID(); ///users/search/用戶id
    string res = client->rpc->get_request(endpoint, routepath);

    cout << "response: " << res << endl;
    return res;
}

std::string ClientIPC::dousersignup(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据

    string username = ipcreq.accountName; //账户名
    string gvdsID = ipcreq.gvdsID;          //在服务端产生
    string pass = ipcreq.Password;
    string email = ipcreq.email;
    string phone = ipcreq.phone;
    string ad = ipcreq.address;
    string de = ipcreq.department;

    //账户注册
    Account person(username, pass, gvdsID, email, phone, ad, de); //以后在服务端产生uuid，客户端这块传值不影响
    std::string value = person.serialize();
    std::cout << value << std::endl;

    string endpoint = client->get_manager();
    string routepath = "/users/bufferuserregister"; ///users/search/用戶id

    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "response: " << res << endl;
    return res;
}
//管理员账户注册
std::string ClientIPC::doadminsignup(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据

    string username = ipcreq.accountName; //账户名
    string gvdsID = ipcreq.gvdsID;          //在服务端产生
    string pass = ipcreq.Password;
    string email = ipcreq.email;
    string phone = ipcreq.phone;
    string ad = ipcreq.address;
    string de = ipcreq.department;

    //账户注册
    Account person(username, pass, gvdsID, email, phone, ad, de); //以后在服务端产生uuid，客户端这块传值不影响
    std::string value = person.serialize();
    std::cout << value << std::endl;

    string endpoint = client->get_manager();
    string routepath = "/users/adminregistration";

    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "response: " << res << endl;
    return res;
}

std::string ClientIPC::dousermodify(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据

    string username = ipcreq.accountName;        //账户名
    string gvdsID = client->user->getAccountID(); //在服务端产生     // 不能修改
    string pass = ipcreq.Password;
    string email = ipcreq.email;
    string phone = ipcreq.phone;
    string ad = ipcreq.address;
    string de = ipcreq.department;

    if (username != client->user->getAccountName() && !client->user->getAccountName().empty())
    { //用户没有登录的时候是 false
        return "client_input_error";
    }

    //账户修改
    Account person(username, pass, gvdsID, email, phone, ad, de);
    std::string person_value = person.serialize();
    std::cout << person_value << std::endl;

    string endpoint = client->get_manager();
    string routepath = "/users/modify";
    string res = client->rpc->post_request(endpoint, routepath, person_value);

    cout << "response: " << res << endl;
    return res;
}

std::string ClientIPC::douserexit(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    string username = ipcreq.accountName; //账户名

    if (username != client->user->getAccountName() && !client->user->getAccountName().empty())
    { //用户没有登录的时候是 false
        return "client_input_error";
    }

    string endpoint = client->get_manager();
    string routepath = "/users/exit/" + client->user->getAccountID(); ///users/search/用戶id
    string res = client->rpc->get_request(endpoint, routepath);
    if (res == "0"){
        client->user->setAccountName("100");
        client->user->setAccountID("200");
    }

    return res;
}

std::string ClientIPC::dousercancel(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    string username = ipcreq.accountName; //账户名
    if (username != client->user->getAccountName() && !client->user->getAccountName().empty())
    { //用户没有登录的时候是 false
        return "client_input_error";
    }

    string endpoint = client->get_manager();
    string routepath = "/users/cancel/" + client->user->getAccountID(); ///users/search/用戶id
    string res = client->rpc->get_request(endpoint, routepath);

    return res;
}

//auth
std::string ClientIPC::doauthsearch(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    string username = ipcreq.accountName; //账户名
    if (username != client->user->getAccountName() && !client->user->getAccountName().empty())
    { //用户没有登录的时候是 false
        return "client_input_error";
    }

    std::string gvdsID = client->user->getAccountID(); //在服务端产生

    //权限查询
    //std::string return_value = "authsearch success";
    string endpoint = client->get_manager();
    string routepath = "/auth/search";
    string res = client->rpc->post_request(endpoint, routepath, gvdsID);

    return res;
}

std::string ClientIPC::doauthmodify(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据

    string username = ipcreq.accountName; //账户名
    if (username != client->user->getAccountName() && !client->user->getAccountName().empty())
    { //用户没有登录的时候是 false
        return "-2";
    }

    FEAuthModifygroupinfo FEgroup;
    FEgroup.gvdsID = client->user->getAccountID(); //在服务端产生
    FEgroup.zonename = ipcreq.zonename;
    FEgroup.modify_groupauth = ipcreq.changeauth;

    string value = FEgroup.serialize();

    string endpoint = client->get_manager();
    string routepath = "/auth/modify";
    string res = client->rpc->post_request(endpoint, routepath, value);

    //++++++
    //修改完要获取最新权限
    string gvdsID = client->user->getAccountID();
    client->optNode->getAuthFromServer(gvdsID);
    //++++++
    if (res == "-1")
    {
        return "modify auth fail";
    }
    else if (res == "33")
    {
        return "Verification failed, access denied";
    }
    else
    { //返回9
        return "modify auth success";
    }
}

std::string ClientIPC::domodifycenter(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据

    FECenterInfo FEcenter;
    FEcenter.centerID = ipcreq.centerID;
    FEcenter.centerIP = ipcreq.centerIP;
    FEcenter.centerPort = ipcreq.centerPort;
    FEcenter.centerName = ipcreq.centerName;

    string value = FEcenter.serialize();

    string addr = *(HvsContext::get_context()->_config->get<std::string>("manager_addr"));
    string url = "http://" + addr; //http://localhost:9090

    //string endpoint = client->get_manager();
    string routepath = "/mconf/addCenter";
    string res = client->rpc->post_request(url, routepath, value);

    return res;
}

std::string ClientIPC::dosearchcenter(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    string endpoint = client->get_manager();
    string routepath = "/mconf/searchCenter";
    string res = client->rpc->get_request(endpoint, routepath);

    return res;
}

std::string ClientIPC::dodeletecenter(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    string value = ipcreq.centerID;

    cout << "value: " << value << endl;
    string endpoint = client->get_manager();
    string routepath = "/mconf/deleteCenter";
    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "res :" << res << endl;
    return res;
}

std::string ClientIPC::dolistapply(IPCreq &ipcreq)
{

    string gvdsID = client->user->getAccountID();
    string endpoint = client->get_manager();
    string routepath = "/users/listapply";
    string res = client->rpc->post_request(endpoint, routepath, gvdsID);

    if (res == "1")
    {
        return res;
    }
    else if (res == "33")
    {
        return res;
    }
    else
    {
        std::vector<std::string> my;
        json_decode(res, my);

        //先清空map
        client->user->clearApply();
        for (auto iter = my.begin(); iter != my.end(); iter++)
        {
            std::string con = *iter;
            struct_apply_info singel_content;
            singel_content.deserialize(con);

            client->user->setApply(singel_content.id, singel_content.data);
        }
        cout << "res: " << res << endl;
        return res;
    }
}

std::string ClientIPC::dosuggestion(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    string sug = ipcreq.sug;
    string id = ipcreq.applyid;

    string endpoint = client->get_manager();

    if (sug == "reject")
    {
        string routepath = "/users/removeapply";
        string res = client->rpc->post_request(endpoint, routepath, id);
        if (res == "0")
        {
            return "success";
        }
        else if (res == "1")
        {
            return "fail";
        }
        else
        {
            return "fail";
        }
    }
    else if (sug == "agree")
    {
        std::string tmp = id.substr(0, 5);
        string value = client->user->getApply(id);

        std::cout << "tmp here :  " << tmp << std::endl;

        std::string res;
        std::string routepath;
        std::string new_route, new_res;
        if (tmp == "usign")
        {
            routepath = "/users/registration";
            res = client->rpc->post_request(endpoint, routepath, value);

            new_route = "/users/removeapply";
            new_res = client->rpc->post_request(endpoint, new_route, id);
        }
        else if (tmp == "zregi")
        {
            routepath = "/zone/registercheck";
            std::string response = client->rpc->post_request(endpoint, routepath, value);
            int result;
            json_decode(response, result);
            if (!result)
            {
                res = "success";
            }
            else if(result==11)
            {
                res = "数据库连接失败或空间目录创建失败或权限添加失败请重试";
            }
			else if (result == 22)
			{
				res = "区域名已存在或区域名、空间名、空间容量为空";
			}
			else if (result == 28)
			{
				res = "分配容量失败，存储资源容量不足";
			}
			else if (result == 2)
			{
				res = "存储资源选择失败,未找到相关存储资源";
			}
			else if (result == 13)
			{
				res = "获取区域账户映射信息失败";
			}
            else
            {
                res = "未知错误";
            }
            
            new_route = "/users/removeapply";
            new_res = client->rpc->post_request(endpoint, new_route, id);
        }
        else if (tmp == "spadd")
        {
            routepath = "/zone/mapaddcheck";
            std::string response = client->rpc->post_request(endpoint, routepath, value);
            int result;
            json_decode(response, result);
            if (!result)
            {
                res = "success";
            }
            else if(result==11)
            {
                res = "数据库连接失败或空间目录创建失败或权限同步失败请重试";
            }
			else if (result == 22)
			{
				res = "空间已存在或同一区域在一个中心不能有两个空间或空间名、空间容量为空";
			}
			else if (result == 28)
			{
				res = "分配容量失败，存储资源容量不足";
			}
			else if (result == 2)
			{
				res = "存储资源选择失败,未找到相关存储资源";
			}
			else if (result == 13)
			{
				res = "获取区域账户映射信息失败";
			}
            else
            {
                res = "未知错误";
            }
            new_route = "/users/removeapply";
            new_res = client->rpc->post_request(endpoint, new_route, id);
        }
        else if (tmp == "spsiz")
        {
            routepath = "/space/changesize";
            std::string response = client->rpc->post_request(endpoint, routepath, value);
            int result;
            json_decode(response, result);
            if (!result)
            {
                res = "success";
            }
			else if (result == 11)
			{
				res = "数据库连接失败";
			}
			else if (result == 22)
			{
				res = "空间容量缩小过多";
			}
			else if (result == 28)
			{
				res = "分配容量失败，存储资源容量不足";
			}
            else
            {
                res = "未知错误";
            }
            new_route = "/users/removeapply";
            new_res = client->rpc->post_request(endpoint, new_route, id);
        }
        return res;
    }
    else
    {
        return "input error";
    }
}

std::string ClientIPC::doadcam(IPCreq &ipcreq)
{
    string name = ipcreq.accountName;
    string centername = ipcreq.centerName;

    //获取 accountName 对应的gvdsID
    std::vector<std::string> vec_name;
    vec_name.push_back(name);
    std::vector<std::string> memID;
    bool tmsuccess = client->user->getMemberID(vec_name, memID);
    if (!tmsuccess)
    {
        std::cerr << "未获得对应账户信息，请确认信息正确！" << std::endl;
        return "未获得对应账户信息，请确认信息正确！";
    }

    struct_AdminAccountMap new_accountmap;
    new_accountmap.adgvdsID = client->user->getAccountID();
    cout << "memID[0] :" << memID[0] << endl;
    new_accountmap.gvdsID = memID[0];
    new_accountmap.hostCenterName = centername;
    string value = new_accountmap.serialize();

    string endpoint = client->get_manager();
    string routepath = "/users/adcam";
    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "res :" << res << endl;
    return res;
}

std::string ClientIPC::doaduam(IPCreq &ipcreq)
{
    string name = ipcreq.accountName;
    string centername = ipcreq.centerName;

    //获取 accountName 对应的gvdsID
    std::vector<std::string> vec_name;
    vec_name.push_back(name);
    std::vector<std::string> memID;
    bool tmsuccess = client->user->getMemberID(vec_name, memID);
    if (!tmsuccess)
    {
        std::cerr << "未获得对应账户信息，请确认信息正确！" << std::endl;
        return "未获得对应账户信息，请确认信息正确！";
    }

    struct_AdminAccountMap new_accountmap;
    new_accountmap.adgvdsID = client->user->getAccountID();
    cout << "memID[0] :" << memID[0] << endl;
    new_accountmap.gvdsID = memID[0];
    new_accountmap.hostCenterName = centername;
    string value = new_accountmap.serialize();

    string endpoint = client->get_manager();
    string routepath = "/users/aduam";
    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "res :" << res << endl;
    return res;
}

std::string ClientIPC::doadsearcham(IPCreq &ipcreq)
{
    string name = ipcreq.accountName;

    //获取 accountName 对应的gvdsID
    std::vector<std::string> vec_name;
    vec_name.push_back(name);
    std::vector<std::string> memID;
    bool tmsuccess = client->user->getMemberID(vec_name, memID);
    if (!tmsuccess)
    {
        std::cerr << "未获得对应账户信息，请确认信息正确！" << std::endl;
        return "-1";
    }

    struct_AdminAccountMap new_accountmap;
    new_accountmap.adgvdsID = client->user->getAccountID(); //json需要adgvdsID
    cout << "memID[0] :" << memID[0] << endl;
    new_accountmap.gvdsID = memID[0];    //json需要gvdsID
    new_accountmap.hostCenterName = ""; //json这块不需要
    string value = new_accountmap.serialize();

    string endpoint = client->get_manager();
    string routepath = "/users/adsearcham";
    string res = client->rpc->post_request(endpoint, routepath, value);

    cout << "res :" << res << endl;
    return res;
}

std::string ClientIPC::doadseepool(IPCreq &ipcreq)
{
    string adgvdsID = client->user->getAccountID();

    string endpoint = client->get_manager();
    string routepath = "/users/adsearchpool";
    string res = client->rpc->post_request(endpoint, routepath, adgvdsID);

    cout << "res :" << res << endl;
    return res;
}

std::string ClientIPC::doadauthsearch(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据
    string name = ipcreq.accountName;     //管理员要查询的 用户的账户名
    
    //获取 name 对应的gvdsID
    std::vector<std::string> vec_name;
    vec_name.push_back(name);
    std::vector<std::string> memID;
    bool tmsuccess = client->user->getMemberID(vec_name, memID);
    if (!tmsuccess)
    {
        std::cerr << "未获得对应账户信息，请确认信息正确！" << std::endl;
        return "-1";
    }

    string gvdsID = memID[0];

    // if (username != client->user->getAccountName() && !client->user->getAccountName().empty())
    // { //用户没有登录的时候是 false
    //     return "client_input_error";
    // }
    // std::string gvdsID = client->user->getAccountID(); //在服务端产生

    //获取用户gvdsID成功
    //权限查询
    //std::string return_value = "authsearch success";
    string endpoint = client->get_manager();
    string routepath = "/auth/search";
    string res = client->rpc->post_request(endpoint, routepath, gvdsID);

    return res;
}


std::string ClientIPC::doadauthmodify(IPCreq &ipcreq)
{
    // TODO: 提前准备的数据

    string name = ipcreq.accountName; //账户名
    // if (username != client->user->getAccountName() && !client->user->getAccountName().empty())
    // { //用户没有登录的时候是 false
    //     return "-2";
    // }

    //获取 name 对应的gvdsID
    std::vector<std::string> vec_name;
    vec_name.push_back(name);
    std::vector<std::string> memID;
    bool tmsuccess = client->user->getMemberID(vec_name, memID);
    if (!tmsuccess)
    {
        std::cerr << "未获得对应账户信息，请确认信息正确！" << std::endl;
        return "未获得对应账户信息，请确认信息正确！";
    }

    FEAuthModifygroupinfo FEgroup;
    FEgroup.gvdsID = memID[0]; //gvdsID
    FEgroup.zonename = ipcreq.zonename;
    FEgroup.modify_groupauth = ipcreq.changeauth;

    string value = FEgroup.serialize();

    string endpoint = client->get_manager();
    string routepath = "/auth/modify";
    string res = client->rpc->post_request(endpoint, routepath, value);

    // //++++++
    // //修改完要获取最新权限
    // string gvdsID = client->user->getAccountID();
    // client->optNode->getAuthFromServer(gvdsID);
    // //++++++
    if (res == "-1")
    {
        return "modify auth fail";
    }
    else if (res == "33")
    {
        return "Verification failed, access denied";
    }
    else
    { //返回9
        return "modify auth success";
    }
}

    std::string ClientIPC::dospacereplica(IPCreq &ipcreq)
    {
        // TODO: 提前准备的数据
        string replicated_id = ipcreq.spacename;
        string single_id = ipcreq.newspacename;
        map<string, string> req;
        req["replicated"] = replicated_id;
        req["single"] = single_id;
        string response = client->rpc->post_request(client->get_manager(), "/space/replica", json_encode(req));
        int result;
        json_decode(response, result);
        if (!result)
            return "success";
        else if (result == -E2BIG)
            return "ERROR: the number of replicated space exceeds maximun limit";
        else
            return "ERROR: cannot create replica space for UNKNOWN ERROR";
    }

} // namespace gvds