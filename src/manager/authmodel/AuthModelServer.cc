#include <iostream>
#include <vector>
#include <stdio.h>
#include "common/JsonSerializer.h"
#include "common/centerinfo.h"
#include "context.h"
#include "datastore/datastore.h"

#include "manager/authmodel/AuthModelServer.h"
#include "manager/usermodel/UserModelServer.h"
#include "manager/space/SpaceServer.h"

#include <unistd.h> //
#include <cstdlib> //system()


using namespace std;

//f3_dbPtr     auth_info


//mgr 和 manager->rest_port()的问题

//联合调试可能的bug
//1、hostCenterName 无法和空间模块匹配
//2、请求ip和端口是否正确

//待完成工作 （这个需要联合调试的时候）
//有了hostCenterName，从配置文件获取对应的ip，不用if判断了
//获取路径的时候，从配置文件读取对应超算的觉得路径，如/root/data/，拼接spacePath(这个是以区域uuid做为文件名的文件夹)，获取完整物理路径，设置相应权限

// 0770 has some problem


namespace hvs{
using namespace Pistache::Rest;
using namespace Pistache::Http;

void AuthModelServer::start() {}
void AuthModelServer::stop() {}

void AuthModelServer::router(Router& router){
    Routes::Post(router, "/auth/selfmemberadd", Routes::bind(&AuthModelServer::self_AuthmemberaddRest, this));
    Routes::Post(router, "/auth/selfmemberdel", Routes::bind(&AuthModelServer::self_AuthmemberdelRest, this));

    Routes::Post(router, "/auth/selfgroupauthmodify", Routes::bind(&AuthModelServer::self_AuthgroupmodifyRest, this));

    Routes::Post(router, "/auth/modify", Routes::bind(&AuthModelServer::AuthModifyRest, this));
    Routes::Post(router, "/auth/search", Routes::bind(&AuthModelServer::AuthSearchModelRest, this));
    
    Routes::Post(router, "/auth/selfSPD", Routes::bind(&AuthModelServer::self_AuthSPDRest, this));
}

void AuthModelServer::self_AuthmemberaddRest(const Rest::Request& request, Http::ResponseWriter response){
    //这里不调用token验证    //因为这里接受的是服务端的rest请求

    //包含的信息，*space_iter（该空间的信息），ownerID vector<string> memberID
    //rest要做的事情：（1）获取ownerID对应的本地账户名字（2）获取每个成员对应的本地账户名
    //              （3）设置相应权限
    auto info = request.body();
    cout << info << endl;

    SelfAuthSpaceInfo auth_space;
    auth_space.deserialize(info);

    int flag = self_Authmemberadd(auth_space);
    if(flag == 0){
        dout(-1) << "success" << dendl;
        response.send(Http::Code::Ok, "0"); 
    }
    else{
        dout(-1) << "fail" << dendl;
        response.send(Http::Code::Ok, "-1"); //point
    }

    cout << "====== end AuthModelServer function: self_AuthmemberaddRest ======"<< endl;
}
int AuthModelServer::self_Authmemberadd(SelfAuthSpaceInfo &auth_space){

    string ownerID = auth_space.ownerID_zone;

    Space spacemeta;
    spacemeta.deserialize(auth_space.spaceinformation);

    //  确认是否匹配
    string hostCenterName  = spacemeta.hostCenterName;

    // 1、获取ownerID 本地对应的超算账户，testowner     这个要知道区域的每个空间都在哪个超算
    UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());
    string value = p_usermodel->getLocalAccountinfo(ownerID, hostCenterName); // 确认是否hostCenterName匹配
    if (value.compare("fail") == 0){
        cout << "SpacePermissionSyne fail" << endl;
        return -1;
    }
    LocalAccountPair localpair;
    localpair.deserialize(value);  //localpair.localaccount 这个是账户名
    string gp = auth_space.zoneID.substr(0, 9);

    vector<string>::iterator m_iter;
    for(m_iter = auth_space.memberID.begin(); m_iter != auth_space.memberID.end(); m_iter++){
        //1.1获取*iter对应的相应超算的本地账户，test1
        // 确认是否hostCenterName匹配
        string m_value = p_usermodel->getLocalAccountinfo(*m_iter, hostCenterName); 
        if (m_value.compare("fail") == 0){
            cout << "SpacePermissionSyne fail" << endl;
            return -1;
        }
        LocalAccountPair member_localpair;
        member_localpair.deserialize(m_value);  //member_localpair.localaccount 这个是账户名

        //1.2将test1加入与testowner同名的组中 usermod -a -G testowner test1
        //   usermod -a -G localpair.localaccount member_localpair.localaccount
        string cmd = "usermod -a -G " + gp + " " + member_localpair.localaccount;
        cout << "cmd :" << cmd << endl;
        system(cmd.c_str()); 
    }// for

    return 0;
}

void AuthModelServer::self_AuthmemberdelRest(const Rest::Request& request, Http::ResponseWriter response){
    //这里不调用token验证    //因为这里接受的是服务端的rest请求

    //包含的信息，*space_iter（该空间的信息），ownerID vector<string> memberID
    //rest要做的事情：（1）获取ownerID对应的本地账户名字（2）获取每个成员对应的本地账户名
    //              （3）设置相应权限
    auto info = request.body();
    cout << info << endl;

    SelfAuthSpaceInfo auth_space;
    auth_space.deserialize(info);

    int flag = self_Authmemberdel(auth_space);
    if(flag == 0){
        dout(-1) << "success" << dendl;
        response.send(Http::Code::Ok, "0");;
    }
    else{
        dout(-1) << "fail" << dendl;
        response.send(Http::Code::Ok, "-1"); //point
    }

    cout << "====== end AuthModelServer function: self_AuthmemberdelRest ======"<< endl;
}
int AuthModelServer::self_Authmemberdel(SelfAuthSpaceInfo &auth_space){
    
    string ownerID = auth_space.ownerID_zone;

    Space spacemeta;
    spacemeta.deserialize(auth_space.spaceinformation);

    //  确认是否匹配
    string hostCenterName  = spacemeta.hostCenterName;

    // 1、获取ownerID 本地对应的超算账户，testowner     这个要知道区域的每个空间都在哪个超算
    UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());
    string value = p_usermodel->getLocalAccountinfo(ownerID, hostCenterName); // 确认是否hostCenterName匹配
    if (value.compare("fail") == 0){
        cout << "SpacePermissionSyne fail" << endl;
        return -1;
    }
    LocalAccountPair localpair;
    localpair.deserialize(value);  //localpair.localaccount 这个是账户名
    string gp = auth_space.zoneID.substr(0, 9);

    vector<string>::iterator m_iter;
    for(m_iter = auth_space.memberID.begin(); m_iter != auth_space.memberID.end(); m_iter++){
        //1.1获取*iter对应的相应超算的本地账户，test1
        // 确认是否hostCenterName匹配
        string m_value = p_usermodel->getLocalAccountinfo(*m_iter, hostCenterName); 
        if (m_value.compare("fail") == 0){
            cout << "member delete fail" << endl;
            return -1;
        }
        LocalAccountPair member_localpair;
        member_localpair.deserialize(m_value);  //member_localpair.localaccount 这个是账户名

        //1.2将test1从testowner同名的组中删除    gpasswd testowner -d test222
        //      gpasswd localpair.localaccount -d member_localpair.localaccount
        string cmd = "gpasswd " + gp + " -d " + member_localpair.localaccount;
        cout << "cmd:" << cmd << endl;
        system(cmd.c_str()); 
    }// for

    return 0;
}

void AuthModelServer::self_AuthgroupmodifyRest(const Rest::Request& request, Http::ResponseWriter response){

    //rest中：hvsID对应的本地ID
            //chmod 7修改值0 spaceID
    auto info = request.body();
    cout << info << endl;

    AuthModifygroupinfo groupinfo;
    groupinfo.deserialize(info);

    int flag = self_Authgroupmodify(groupinfo);
    if(flag == 0){
        dout(-1) << "success" << dendl;
        response.send(Http::Code::Ok, "0");;
    }
    else{
        dout(-1) << "fail" << dendl;
        response.send(Http::Code::Ok, "-1"); //point
    }

    cout << "====== end AuthModelServer function: self_AuthgroupmodifyRest ======"<< endl;

}
int AuthModelServer::self_Authgroupmodify(AuthModifygroupinfo &groupinfo){
    //hvsID对应的本地ID
    //chmod 7修改值0 spaceID
    string hvsID = groupinfo.hvsID;

    Space spacemeta;
    spacemeta.deserialize(groupinfo.spaceinformation);

    //  确认是否匹配
    string hostCenterName  = spacemeta.hostCenterName;

    // //没用到hvsID
    // // 1、获取ownerID 本地对应的超算账户，testowner     
    // UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());
    // string value = p_usermodel->getLocalAccountinfo(ownerID, hostCenterName); //TODO 确认是否hostCenterName匹配
    // if (value.compare("fail") == 0){
    //     cout << "self_Authgroupmodify fail" << endl;
    //     return -1;
    // }
    // LocalAccountPair localpair;
    // localpair.deserialize(value);  //localpair.localaccount 这个是账户名

    string localstoragepath = *(HvsContext::get_context()->_config->get<std::string>("storage"));
        cout <<"localstoragepath: " << localstoragepath << endl;
    string spacepath = localstoragepath + "/" + spacemeta.spacePath; //这个可能不是最终的路径，需确认
        cout << "final path :" << spacepath << endl;

    //示例chmod 777 filename
    string cmd = "chmod " + groupinfo.au_person + groupinfo.au_group + groupinfo.au_other + " " + spacepath;
        cout << "cmd:" << cmd << endl;
    system(cmd.c_str());

    return 0;
}

void AuthModelServer::self_AuthSPDRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "=========enter：self_AuthSPDRest============" << endl;
    auto info = request.body();
    cout << info << endl;

    SelfSPD mySPD;
    mySPD.deserialize(info);

    int flag = self_AuthSPD(mySPD);
    cout << "flag: " << flag <<endl;
     if(flag == 0){
        dout(-1) << "success" << dendl;
        response.send(Http::Code::Ok, "0");;
    }
    else{
        dout(-1) << "fail" << dendl;
        response.send(Http::Code::Ok, "-1"); //point
    }
    cout << "=========end：self_AuthSPDRest============"<< endl;
}

int AuthModelServer::self_AuthSPD(SelfSPD &mySPD){
    
    cout << "=========enter：self_AuthSPD============" << endl;
    Space spacemeta;
    spacemeta.deserialize(mySPD.spaceinformation);

    //设置root 删除组
    string localstoragepath = *(HvsContext::get_context()->_config->get<std::string>("storage"));
        cout <<"localstoragepath: " << localstoragepath << endl;

    string spacepath = localstoragepath + "/" + spacemeta.spacePath;
    cout << "final path: " << spacepath << endl;

    //root的gid、uid为0
    if(chown(spacepath.c_str(), 0, 0) == -1){
        return -1;
    }

    string group_cmd = "groupdel " + mySPD.gp;
    cout << "group_cmd: " << group_cmd << endl;
    system(group_cmd.c_str());

    cout << "=========end：self_AuthSPD============" << endl;
    return 0;

}

//1权限增加模块
//1.1 区域初始权限记录接口  :: 被区域注册模块调用 :: 只记录数据库,不需访问存储集群设置权限,sy那边会设置
int AuthModelServer::ZonePermissionAdd(std::string zoneID, std::string ownerID){
    Auth person(zoneID);
    person.HVSAccountID = ownerID;
    person.owner_read = 1;
    person.owner_write = 1;
    person.owner_exe = 1;

    person.group_read = 1;
    person.group_write = 1;
    person.group_exe = 1;

    person.other_read = 0;
    person.other_write = 0;
    person.other_exe = 0;
    
    string value = person.serialize();

    std::shared_ptr<hvs::CouchbaseDatastore> f3_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("auth_info"));
    f3_dbPtr->init();

    int f1_flag = f3_dbPtr->set(zoneID, value);
    if (f1_flag != 0){
        cout << "fail" << endl;
        return -1;
    }

    return 0;
}

//1.2 空间权限同步接口  :: 被空间创建模块调用 :: 查询空间所属区域的权限，设置空间为此权限,并且查询组员，同步设置组员权限
//一次只设置一个空间，不涉及跨超算调用【因此此接口已完成，不用再次修改】
int AuthModelServer::SpacePermissionSyne(std::string spaceID, std::string zoneID, std::string ownerID, std::vector<std::string> memberID){
    cout << "=======start: SpacePermissionSyne=======" << endl;

    string localstoragepath = *(HvsContext::get_context()->_config->get<std::string>("storage"));
    cout <<"localstoragepath: " << localstoragepath << endl;

    // string ZoneID;
    // //调用sy的接口,获取空间对应的区域；
    // //bool getspace_zone(spaceID, & zoneID); 返回bool，  
    // bool space_api = true;
    // if (!space_api){
    //     return -1;
    // }
    // ZoneID = "123";

    //1、根据zoneid 获取区域的权限 
    std::shared_ptr<hvs::CouchbaseDatastore> f3_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("auth_info"));
    f3_dbPtr->init();

    auto [pvalue, error] = f3_dbPtr->get(zoneID);
    if(error){
        cout << "fail" << endl;
        return -1;
    }

    Auth person;
    person.deserialize(*pvalue);

    int auth_sum_person = 0;
    if (person.owner_read == 1)
        auth_sum_person += 4;
    if(person.owner_write == 1)
        auth_sum_person += 2;
    if(person.owner_exe == 1)
        auth_sum_person += 1;
    string au_person = to_string(auth_sum_person);

    int auth_sum_group = 0;
    if(person.group_read == 1)
        auth_sum_group += 4;
    if(person.group_write == 1)
        auth_sum_group += 2;
    if(person.group_exe == 1)
        auth_sum_group += 1;
    string au_group = to_string(auth_sum_group);

    int auth_sum_other = 0;
    if(person.other_read == 1)
        auth_sum_other += 4;
    if(person.other_write == 1)
        auth_sum_other += 2;
    if(person.other_exe == 1)
        auth_sum_other += 1;
    string au_other = to_string(auth_sum_other);

    cout << "111" << endl;
    //2、根据区域权限设置空间的权限
        //2.1 获取ownerid 对应的的本地账户(空间所在地)test1;
    UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());
    // 根据空间获取hostCenterName 值
        //2.1.1查询区域信息,获取区域对应空间信息
    cout << "112.1" << endl;
    std::shared_ptr<hvs::CouchbaseDatastore> zone_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
    zone_dbPtr->init();
    auto [pzone_value, zone_error] = zone_dbPtr->get(zoneID);
    if (zone_error){
        cout << "authmodelserver: zone search fail" << endl;
        return -1;
    }
    cout << "112.2" << endl;
    Zone zone_content;
    zone_content.deserialize(*pzone_value);  //为了获取zone_content.spaceID 一个vector
    cout << "112" << endl;
        //2.1.2获取每个空间对应的超算
    vector<Space> result; //里面存储每个空间的string信息 需要饭序列化
    SpaceServer *p_space = static_cast<SpaceServer*>(mgr->get_module("space").get());
        dout(-1) << "start: recall GetSpacePosition" << dendl;
    p_space->GetSpacePosition(result, zone_content.spaceID);
        dout(-1) << "end: recall GetSpacePosition" << dendl;
    
    cout << "113" << endl;
    for (auto& space_iter : result) {
        Space spacemeta = space_iter;

        if (spacemeta.spaceID.compare(spaceID) != 0){
            continue;
        }

        //测试时假设是Shanghai
        string hostCenterName = spacemeta.hostCenterName;
        cout <<"spacemeta.hostCenterName: " << spacemeta.hostCenterName << endl;
        //string hostCenterName ="Shanghai";
        cout <<"test hostCenterName: " << hostCenterName << endl;

        string value = p_usermodel->getLocalAccountinfo(ownerID, hostCenterName);
        if (value.compare("fail") == 0){
            cout << "SpacePermissionSyne fail" << endl;
            return -1;
        }
        LocalAccountPair localpair;
        localpair.deserialize(value);  //localpair.localaccount 这个是账户名 ownerID对应的

            //2.2 更改文件夹所属用户和组chown -R test1:test1 空间名
        //获取物理路径 spacepath值 
        string spacepath = localstoragepath + "/" + spacemeta.spacePath; //这个可能不是最终的路径，需确认TODO
        cout << "localstoragepath : " << localstoragepath <<endl; 
        cout << "final path :" << spacepath << endl;
        //TODO  获取账户名(localpair.localaccount)和组名（localpair.localaccount）的uid、gid
        // int uid;
        // int gid;

        // if(chown(spacepath.c_str(), uid, gid) == -1){
        //     return -1;
        // }
        //（1）创建组    //重复创建不影响
        string gp = zoneID.substr(0, 9);
        string g_cmd = "groupadd " + gp;
        cout << "g_cmd: " << g_cmd <<endl;
        system(g_cmd.c_str());

        //（2）设置全 （拥有者:组）权限
        string chown_cmd = "chown -R " + localpair.localaccount + ":" + gp + " " + spacepath;
        cout << chown_cmd << endl;
        system(chown_cmd.c_str());
            //（3）2.3 设置权限：chmod （au_person）(au_group)(au_other) 文件名      chmod 777 filename
        string tmp_str = au_person + au_group + au_other;
        //  注意确认这块设置权限是否有问题
        // if (chmod(spacepath.c_str(), 0770) == -1){ //TODO    tmp_int=770 这块参数类型有问题，要0770才正常
        //     cout << "chmod fail";
        //     return -1;
        // }
        string chmod_cmd = "chmod " + tmp_str + " " + spacepath;
        cout << chmod_cmd << endl;
        system(chmod_cmd.c_str());
        //TODO 设置组员权限
            //获取组成员hvsID 对应的本地账户    【localpair.localaccount】
            //加入区域ownerID 对应的本地账户中  【member_localpair.localaccount】
        vector<string>::iterator m_iter;   //TODO如何获取memberID！！！！！！！！！！！！！！！！！！！！！！！
        for(m_iter = memberID.begin(); m_iter != memberID.end(); m_iter++){
                string m_value = p_usermodel->getLocalAccountinfo(*m_iter, hostCenterName); 
                if (m_value.compare("fail") == 0){
                    cout << "SpacePermissionSyne fail" << endl;
                    return -1;
                }
                LocalAccountPair member_localpair;
                member_localpair.deserialize(m_value);  //member_localpair.localaccount 这个是账户名
                //1.2将test1加入与testowner同名的组中 usermod -a -G testowner test1
                //   usermod -a -G localpair.localaccount member_localpair.localaccount
                string cmd = "usermod -a -G " + gp + " " + member_localpair.localaccount;
                cout << "cmd :" << cmd << endl;
                system(cmd.c_str()); 
        }

    } //外层for 

    //不需要记录数据库

    return 0;

}

//1、根据zoneid 获取区域的权限 
//2  shezhi ci spaceID yu zoneid xiangtong quanxian
        //huoqu space suozaidi de owner duiying de local zhanghu





//1.3 副本权限同步接口   :: 被空间创建模块调用 ::  这个和sy商量下
int AuthModelServer::ReplacePermissionSyne(){}

//1.4 空间定位接口      我调sy

//1.5 成员权限增加接口      ::区域共享模块调用  :: 设置新成员对区域的权限，记录数据库 
int AuthModelServer::ZoneMemberAdd(string zoneID, string ownerID, vector<string> memberID){
    
    std::shared_ptr<hvs::CouchbaseDatastore> zone_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
    zone_dbPtr->init();
    auto [pzone_value, zone_error] = zone_dbPtr->get(zoneID);
    if (zone_error){
        cout << "authmodelserver: zone search fail" << endl;
        return -1;
    }

    Zone zone_content;
    zone_content.deserialize(*pzone_value);  //为了获取zone_content.spaceID 一个vector

        //获取每个空间对应的超算
            //获取ownerid 对应的的本地账户(空间所在地)test1;
    UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());

    vector<Space> result; //里面存储每个空间的string信息 需要饭序列化
    SpaceServer *p_space = static_cast<SpaceServer*>(mgr->get_module("space").get());
        dout(-1) << "start: recall GetSpacePosition" << dendl;
    p_space->GetSpacePosition(result, zone_content.spaceID);
        dout(-1) << "end: recall GetSpacePosition" << dendl;
    
    //+++++++
    //---提前读取center_information信息----
     std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();
    string c_key = "center_information";
    auto [pcenter_value, c_error] = f0_dbPtr->get(c_key);
    if (c_error){
        cout << "authmodelserver: get center_information fail" << endl;
        return -1;
    }
    CenterInfo mycenter;
    mycenter.deserialize(*pcenter_value);
    vector<string>::iterator c_iter;
    //--------------------
    


    Http::Client client;
    char url[256];

    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);
    int return_flag = 0;
    //+++++++

    //for owner对应的区域的所有超算空间获取本地账户
    for (auto space_iter : result){
        Space spacemeta = space_iter;

        string hostCenterName = spacemeta.hostCenterName;
        cout << "hostCenterName: " << hostCenterName << endl;

        //++++++++++++++++++++++++++++++++++++
        //获取hostCenterName，并进行判断，向对应超算中心发出请求,包含又发送到本超算的restful请求
        //包含的信息，*space_iter（该空间的信息），ownerID vector<string> memberID
        //rest要做的事情：（1）获取ownerID对应的本地账户名字（2）获取每个成员对应的本地账户名
        //              （3）设置相应权限

        //+++++
        //TODO获取空间对应ip地址   根据   spacemeta.hostCenterName 或者 spacemeta.hostCenterID 获取地址
        SelfAuthSpaceInfo auth_space;
        auth_space.spaceinformation = space_iter.serialize();
        auth_space.ownerID_zone = ownerID;
        auth_space.memberID = memberID;//赋值
        auth_space.zoneID = zoneID;
        string tmp_value = auth_space.serialize();

        if(ManagerID == spacemeta.hostCenterID){ //本地
            if(self_Authmemberadd(auth_space) != 0){
                return_flag = -1;
            } 
        }
        else //远端
        { 
            cout << "发送到远端"<<endl;
            string tmp_ip = mycenter.centerIP[spacemeta.hostCenterID];
            string tmp_port = mycenter.centerPort[spacemeta.hostCenterID];
            cout << tmp_ip << endl;
            cout << tmp_port << endl;

        
            //TODO 注意端口是否正确
            snprintf(url, 256, "http://%s:%s/auth/selfmemberadd", tmp_ip.c_str() ,tmp_port.c_str());

            auto response = client.post(url).body(tmp_value).send();
            dout(-1) << "Client Info: post request " << url << dendl;

            std::promise<bool> prom;
            auto fu = prom.get_future();
            response.then(
            [&](Http::Response res) {
                std::cout << "Response code = " << res.code() << std::endl;
                auto body = res.body();
                if (!body.empty()){
                    std::cout << "Response body = " << body << std::endl;
                    //your code write here
                    if (body != "0"){
                        cout << "body != 0, fail " << endl;
                        return_flag = -1;
                    }

                }
                prom.set_value(true);
            },
            Async::IgnoreException);
            //阻塞
            fu.get();
        } //else
        //++++++++++++++++++++++++++++++++++++
    } // 外层for

    //数据库中无需记录任何东西
    //++++++++
    client.shutdown();
    //++++++++
    return return_flag; //0是成功
}

//1.6
//setPermission(string uuid, string local, string )

//=============================================
//2、权限删除模块
//2.1 区域权限删除接口     ::被区域注销模块调用::删除相应权限,删除数据库中权限记录，无需知道ownerid
int AuthModelServer::ZonePermissionDeduct(string zoneID, string OwnerID){
    std::shared_ptr<hvs::CouchbaseDatastore> f3_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("auth_info"));
    f3_dbPtr->init();

    int flag = f3_dbPtr->remove(zoneID);
    if(flag){
        cout << "remove auth fail" <<endl;
        return -1;
    }
    else{
        cout << "remove auth success" << endl;
        return 0;
    }
}

//2.2 成员权限删除接口  ::被区域共享模块调用::删除成员对区域的权限，不需要记录数据库 //区域拥有者的空间都在哪些超算，要分别从到每个超算对应的组删除
int AuthModelServer::ZoneMemberDel(string zoneID, string ownerID, vector<string> memberID){
    std::shared_ptr<hvs::CouchbaseDatastore> zone_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
    zone_dbPtr->init();
    auto [pzone_value, zone_error] = zone_dbPtr->get(zoneID);
    if (zone_error){
        cout << "authmodelserver: zone search fail" << endl;
        return -1;
    }

    Zone zone_content;
    zone_content.deserialize(*pzone_value);  //为了获取zone_content.spaceID 一个vector

    //获取每个空间对应的超算
            //获取ownerid 对应的的本地账户(空间所在地)test1;
    UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());

    vector<Space> result; //里面存储每个空间的string信息 需要饭序列化
    SpaceServer *p_space = static_cast<SpaceServer*>(mgr->get_module("space").get());
        dout(-1) << "start: recall GetSpacePosition" << dendl;
    p_space->GetSpacePosition(result, zone_content.spaceID);
        dout(-1) << "end: recall GetSpacePosition" << dendl;
    
    //+++++++
    //---提前读取center_information信息----
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();
    string c_key = "center_information";
    auto [pcenter_value, c_error] = f0_dbPtr->get(c_key);
    if (c_error){
        cout << "authmodelserver: get center_information fail" << endl;
        return -1;
    }
    CenterInfo mycenter;
    mycenter.deserialize(*pcenter_value);
    vector<string>::iterator c_iter;
    //--------------------

    Http::Client client;
    char url[256];

    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);
    int return_flag = 0;
    //+++++++

    //for owner对应的区域的所有超算空间获取本地账户
    vector<string>::iterator space_iter;
    for (auto space_iter : result){
        Space spacemeta = space_iter;

        string hostCenterName = spacemeta.hostCenterName;

        //++++++++++++++++++++++++++++++++++++
        //获取hostCenterName，并进行判断，向对应超算中心发出请求,包含又发送到本超算的restful请求
        //包含的信息，*space_iter（该空间的信息），ownerID vector<string> memberID
        //rest要做的事情：（1）获取ownerID对应的本地账户名字（2）获取每个成员对应的本地账户名
        //              （3）设置相应权限

        //+++++
        //获取空间对应ip地址   根据   spacemeta.hostCenterName 或者 spacemeta.hostCenterID 获取地址
        //hostCenterName可能需要做转换
        //TODO获取空间对应ip地址   根据   spacemeta.hostCenterName 或者 spacemeta.hostCenterID 获取地址
        SelfAuthSpaceInfo auth_space;
        auth_space.spaceinformation = space_iter.serialize();
        auth_space.ownerID_zone = ownerID;
        auth_space.memberID = memberID;//赋值
        auth_space.zoneID = zoneID;
        string tmp_value = auth_space.serialize();

        if(ManagerID == spacemeta.hostCenterID){ //本地
            if(self_Authmemberdel(auth_space) != 0){
                return_flag = -1;
            } 
        }
        else //远端
        { 
            cout <<"发送到远端" << endl;
            string tmp_ip = mycenter.centerIP[spacemeta.hostCenterID];
            string tmp_port = mycenter.centerPort[spacemeta.hostCenterID];
            cout << tmp_ip << endl;
            cout << tmp_port << endl;


            //TODO 注意端口是否正确  

            snprintf(url, 256, "http://%s:%s/auth/selfmemberdel", tmp_ip.c_str() ,tmp_port.c_str());

            auto response = client.post(url).body(tmp_value).send();
            dout(-1) << "Client Info: post request " << url << dendl;

            std::promise<bool> prom;
            auto fu = prom.get_future();
            response.then(
                [&](Http::Response res) {
                //dout(-1) << "Manager Info: " << res.body() << dendl;
                std::cout << "Response code = " << res.code() << std::endl;
                auto body = res.body();
                if (!body.empty()){
                    std::cout << "Response body = " << body << std::endl;
                    //your code write here
                    if (body != "0"){
                        cout << "body != 0, fail " << endl;
                        return_flag = -1;
                    }

                }
                prom.set_value(true);
            },
            Async::IgnoreException);

            //阻塞
            fu.get();
            //++++++++++++++++++++++++++++++++++++
        }//else
    }//最外层for
    
    //不需要改动数据库，
    //++++++++
    client.shutdown();
    //++++++++
    return return_flag; //0是成功
}

//2.3 空间定位，我调用sy 


//2.4 空间权限删除接口  ：：被空间创建模块调用：：删除存储集群上空间对应的权限  //数据库不用更新，因为无此空间记录
//这个也不跨超算，因此不用rest，【不用更改】 //这是因为在客户端就判断好了在哪个管理节点--宋尧
int AuthModelServer::SpacePermissionDelete(string spaceID, string zoneID){
    string localstoragepath = *(HvsContext::get_context()->_config->get<std::string>("storage"));
    cout <<"localstoragepath: " << localstoragepath << endl;

    vector<string> tmp_vec_spaceID;
    tmp_vec_spaceID.push_back(spaceID);

    vector<Space> result; //里面存储每个空间的string信息 需要饭序列化
    SpaceServer *p_space = static_cast<SpaceServer*>(mgr->get_module("space").get());
        dout(-1) << "start: recall GetSpacePosition" << dendl;
    p_space->GetSpacePosition(result, tmp_vec_spaceID);
        dout(-1) << "end: recall GetSpacePosition" << dendl;

    
    //+++++++
    //---提前读取center_information信息----
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();
    string c_key = "center_information";
    auto [pcenter_value, c_error] = f0_dbPtr->get(c_key);
    if (c_error){
        cout << "authmodelserver: get center_information fail" << endl;
        return -1;
    }
    CenterInfo mycenter;
    mycenter.deserialize(*pcenter_value);
    vector<string>::iterator c_iter;
    //--------------------
     Http::Client client;
     char url[256];

    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);
    //----------------------
    int return_flag = 0;

    string gp = zoneID.substr(0, 9);
    for (auto space_iter : result){ //for其实没有用，因为vector只有一个spaceID，即每次只删除一个spaceID
        Space spacemeta = space_iter;

        string hostCenterName = spacemeta.hostCenterName;

        SelfSPD mySPD;
        mySPD.spaceinformation = space_iter.serialize();
        mySPD.gp = gp;
        mySPD.zoneID = zoneID;
        mySPD.spaceID = spaceID;
        string tmp_value = mySPD.serialize();


        //TODO (这个是否是最终路径，要确认) 获取空间物理路径  直接把拥有者和组改成root //chown -R root:root 文件名
        if(ManagerID == spacemeta.hostCenterID){ //本地
            if(self_AuthSPD(mySPD) != 0){
                cout<< "删除空间权限失败" << endl;
                return_flag = -1;
            }
            else{
                cout<< "删除空间权限成功" << endl;
            }
        }
        else{ //远端
            cout <<"发送到远端" << endl;
            string tmp_ip = mycenter.centerIP[spacemeta.hostCenterID];
            string tmp_port = mycenter.centerPort[spacemeta.hostCenterID];
            
            snprintf(url, 256, "http://%s:%s/auth/selfSPD", tmp_ip.c_str() ,tmp_port.c_str());

            auto response = client.post(url).body(tmp_value).send();
            dout(-1) << "Client Info: post request " << url << dendl;

            std::promise<bool> prom;
            auto fu = prom.get_future();
            response.then(
                [&](Http::Response res) {
                //dout(-1) << "Manager Info: " << res.body() << dendl;
                std::cout << "Response code = " << res.code() << std::endl;
                auto body = res.body();
                if (!body.empty()){
                    std::cout << "Response body = " << body << std::endl;
                    //your code write here
                    if (body != "0"){
                        cout << "body != 0, fail " << endl;
                        return_flag = -1;
                    }

                }
                prom.set_value(true);
            },
            Async::IgnoreException);

            //阻塞
            fu.get();
            cout << "fu.get()"<< endl;
        }
    }

    //++++++++
    client.shutdown();
    //++++++++
    //不需要记录数据库
    return return_flag;
}


//2.5


//3、权限修改模块，提供一个rest api，让前端调用  //  这个等其他的测试都通过再写吧，否则没意义【流程参考A4纸上的】
// 返回33权限， 0成功，-1失败
void AuthModelServer::AuthModifyRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start AuthModelServer function: AuthModifyRest ======"<< endl;
    // bool valid = auth_token(request);
    // if (!valid){
    //     response.send(Http::Code::Unauthorized, "33");
    //     return;
    // }

    auto info = request.body();
    cout << info << endl;

    FEAuthModifygroupinfo FEgroup;
    FEgroup.deserialize(info);

    int flag = AuthModify(FEgroup.hvsID, FEgroup.zonename, FEgroup.modify_groupauth);
    if(flag == 0){
        dout(-1) << "success" << dendl;
        response.send(Http::Code::Ok, "0");;
    }
    else{
        dout(-1) << "fail" << dendl;
        response.send(Http::Code::Ok, "-1"); //point
    }

    cout << "====== end AuthModelServer function: AuthModifyRest ======"<< endl;

}

int AuthModelServer::AuthModify(string hvsID, string zonename, string modify_groupauth){
    cout << "enter AuthModify"<< endl;
    //1、  获取hvsID 对应的 zoneID  以及权限   (根据hvsID 和 zonename 如何获取到一个zoneID？)
    std::string zonebucket = "zone_info";
    std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore(zonebucket));
    zonePtr->init();

    string ownerID = hvsID;
    std::string query = "select * from `"+zonebucket+"` where owner = \"ownerID\" and name = \"zonename\";";
   
    int pos = query.find("ownerID");
    query.erase(pos, 7);
    query.insert(pos, ownerID);
    int pos2 = query.find("zonename");
    query.erase(pos2, 8);
    query.insert(pos2, zonename);
    cout << "******" <<query << "--"<< endl;
    auto [vp, err] = zonePtr->n1ql(query);
    if(vp->size() == 0) {
        cout << "find zoneID fail" << endl;
        return -1;
    }

    Zone tmp;
    std::vector<std::string>::iterator it = vp->begin();   //查询结果只有一个，就是对应owner 的 zoneID
    std::string n1ql_result = *it;
    std::string tmp_value = n1ql_result.substr(13, n1ql_result.length() - 14);
    tmp.deserialize(tmp_value);
        
    std::string zoneID = tmp.zoneID;
    std::cout << "****zoneID: " << zoneID << std::endl;


    //1.5 获取这个区域对应的组权限
    std::shared_ptr<hvs::CouchbaseDatastore> f3_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("auth_info"));
    f3_dbPtr->init();

    auto [pvalue, error] = f3_dbPtr->get(zoneID);
    if(error){
        cout << "fail" << endl;
        return -1;
    }

    Auth person;
    person.deserialize(*pvalue);

    int auth_sum_person = 0;
    if (person.owner_read == 1)
        auth_sum_person += 4;
    if(person.owner_write == 1)
        auth_sum_person += 2;
    if(person.owner_exe == 1)
        auth_sum_person += 1;
    string au_person = to_string(auth_sum_person);

    int auth_sum_group = 0;
    if(person.group_read == 1)
        auth_sum_group += 4;
    if(person.group_write == 1)
        auth_sum_group += 2;
    if(person.group_exe == 1)
        auth_sum_group += 1;
    string au_group = to_string(auth_sum_group);

    int auth_sum_other = 0;
    if(person.other_read == 1)
        auth_sum_other += 4;
    if(person.other_write == 1)
        auth_sum_other += 2;
    if(person.other_exe == 1)
        auth_sum_other += 1;
    string au_other = to_string(auth_sum_other);


    //2、zoneID 对应的所有spaceID
    std::shared_ptr<hvs::CouchbaseDatastore> zone_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
    zone_dbPtr->init();
    auto [pzone_value, zone_error] = zone_dbPtr->get(zoneID);
    if (zone_error){
        cout << "authmodelserver: auth modify fail" << endl;
        return -1;
    }

    Zone zone_content;
    zone_content.deserialize(*pzone_value);  //为了获取zone_content.spaceID 一个vector

    //获取每个空间对应的超算
            //获取ownerid 对应的的本地账户(空间所在地)test1;
    UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());

    vector<Space> result; //里面存储每个空间的string信息 需要饭序列化
    SpaceServer *p_space = static_cast<SpaceServer*>(mgr->get_module("space").get());
        dout(-1) << "start: recall GetSpacePosition" << dendl;
    p_space->GetSpacePosition(result, zone_content.spaceID);
        dout(-1) << "end: recall GetSpacePosition" << dendl;
    
    //+++++++
    //---提前读取center_information信息----
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();
    //string c_key = "center_information";
    cout << "c_key : " << c_key << endl;
    auto [pcenter_value, c_error] = f0_dbPtr->get(c_key);
    if (c_error){
        cout << "authmodelserver: get center_information fail" << endl;
        return -1;
    }
    CenterInfo mycenter;
    mycenter.deserialize(*pcenter_value);
    vector<string>::iterator c_iter;
    //--------------------


    Http::Client client;
    char url[256];

    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);
    int return_flag = 0;
    //+++++++

    //for owner对应的区域的所有超算空间获取本地账户
    vector<string>::iterator space_iter;
    for (auto space_iter : result){
        Space spacemeta = space_iter;

        //spaceID 的hostCenterName 发rest
        string hostCenterName = spacemeta.hostCenterName;

        //TODO 根据hostCenterName获取相应超算的ip和port
        AuthModifygroupinfo groupinfo;
        groupinfo.spaceinformation = space_iter.serialize();
        groupinfo.hvsID = hvsID;
        groupinfo.au_person = au_person;
        groupinfo.au_group = modify_groupauth;
        groupinfo.au_other = au_other;
        string tmp_value = groupinfo.serialize();

        if(ManagerID == spacemeta.hostCenterID){ //本地
            if(self_Authgroupmodify(groupinfo) != 0){
                cout<< "修改权限失败" << endl;
                return_flag = -1;
            }
        }
        else
        {
            cout << "发送到远端"<<endl;
            string tmp_ip = mycenter.centerIP[spacemeta.hostCenterID];
            string tmp_port = mycenter.centerPort[spacemeta.hostCenterID];
            cout << tmp_ip << endl;
            cout << tmp_port << endl;


            //TODO 注意端口是否正确  
            // snprintf(url, 256, "http://%s:9090/auth/selfgroupauthmodify", ip_space.c_str());
            snprintf(url, 256, "http://%s:%s/auth/selfgroupauthmodify", tmp_ip.c_str() ,tmp_port.c_str());

            
                //rest中：hvsID对应的本地ID
                        //chmod 7修改值0 spaceID
            
            auto response = client.post(url).body(tmp_value).send();
            dout(-1) << "Client Info: post request " << url << dendl;

            std::promise<bool> prom;
            auto fu = prom.get_future();
            response.then(
            [&](Http::Response res) {
                //dout(-1) << "Manager Info: " << res.body() << dendl;
                std::cout << "Response code = " << res.code() << std::endl;
                auto body = res.body();
                if (!body.empty()){
                    std::cout << "Response body = " << body << std::endl;
                    //your code write here
                    if (body != "0"){
                        cout << "body != 0, fail " << endl;
                        return_flag = -1;
                    }
            }
            prom.set_value(true);
            },
            Async::IgnoreException);

            //阻塞
            fu.get();
        }//else
        //++++++++++++++++++++++++++++++++++++
    }
    
    //不需要改动数据库，
    //++++++++
    client.shutdown();
    //++++++++

    //若实际修改成功，要更新数据库
    if (return_flag == 0)
    {   
        int pr,pw,pe;
        transform_auth(modify_groupauth, pr, pw, pe);
        person.group_read = pr;
        person.group_write = pw;
        person.group_exe = pe;
        string m_value = person.serialize();
        int f3_f = f3_dbPtr->set(zoneID, m_value);
        if (f3_f != 0){
            cout << "fail" << endl;
            return_flag = -1;
        }
    }
    

    return return_flag; //0是成功
}

void AuthModelServer::transform_auth(std::string &modify_groupauth, int &pr, int &pw, int &pe){
    if(modify_groupauth == "0"){
        pr = 0; pw = 0; pe = 0;
    }
    else if(modify_groupauth == "1"){
        pr = 0; pw = 0; pe = 1;
    }
    else if(modify_groupauth == "2"){
        pr = 0; pw = 1; pe = 0;
    }
    else if(modify_groupauth == "3"){
        pr = 0; pw = 1; pe = 1;
    }
    else if(modify_groupauth == "4"){
        pr = 1; pw = 0; pe = 0;
    }
    else if(modify_groupauth == "5"){
        pr = 1; pw = 0; pe = 1;
    }
    else if(modify_groupauth == "6"){
        pr = 1; pw = 1; pe = 0;
    }
    else if(modify_groupauth == "7"){
        pr = 1; pw = 1; pe = 1;
    }
    else {
        pr = 0; pw = 0; pe = 0;
    }
}



//4、权限查询模块  
//失败返回 33, fail 正常值；
void AuthModelServer::AuthSearchModelRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start AuthModelServer function: AuthSearchModelRest ======"<< endl;
    // bool valid = auth_token(request);
    // if (!valid){
    //     cout << "aaa here?" << endl;
    //     response.send(Http::Code::Unauthorized, "33");
    //     return;
    // }

    auto info = request.body();
    cout << "info: " <<info << endl;  //hvsid

    string m_info = info;
    string data = AuthSearchModel(m_info);
    if (data.compare("fail") == 0){
        response.send(Http::Code::Ok, "fail"); 
    }
    else{
        response.send(Http::Code::Ok, data); // data 在C++客户端使用AuthSearch 返序列化
    }
   
    cout << "====== end AuthModelServer function: AuthSearchModelRest ======"<< endl;
}


string AuthModelServer::AuthSearchModel(string &hvsID){
     cout << "enter AuthSearchModel"<< endl;
    //1、查询hvsid对应的所有区域ID
    vector<Zone> result_z;
    ZoneServer *p_zone = static_cast<ZoneServer*>(mgr->get_module("zone").get());
        cout << "start: p_zone->GetZoneInfo" << endl;
    bool result_b = p_zone->GetZoneInfo(result_z, hvsID);//sy函数
        cout << "end: p_zone->GetZoneInfo" << endl;
    //string result;
    if( !result_b ){
        cout << "get zoneID info fail" <<endl;
        return "fail";
    }

    //result = res.serialize();

    //2、然后for区域
    AuthSearch myauth;
    myauth.hvsID = hvsID;

    string hvsname;
    std::shared_ptr<hvs::Datastore> accountPtr = hvs::DatastoreFactory::create_datastore("account_info", hvs::DatastoreType::couchbase);
    auto [own, oerr] = accountPtr->get(hvsID);
    if (!oerr)
    {
          Account owner;
          owner.deserialize(*own);
          hvsname = owner.accountName;
    }
    else return "fail";

    for(auto iter : result_z){
        Zone myzone = iter;
        
        string r,w,x,identity;
        string ownergroupR, ownergroupW, ownergroupE;
        int tmp = subAuthSearchModel(myzone, hvsname, r, w, x , identity, ownergroupR, ownergroupW, ownergroupE);
        if (tmp==-1){
            continue;// 直接接续下一个区域
        }
        myauth.vec_ZoneID.push_back(myzone.zoneID);
        myauth.read[myzone.zoneID] = r;
        myauth.write[myzone.zoneID] = w;
        myauth.exe[myzone.zoneID] = x;
        myauth.isowner[myzone.zoneID] = identity;
        myauth.zoneName[myzone.zoneID] = myzone.zoneName;
        if(identity == "1"){ //主人
            myauth.ownergroupR[myzone.zoneID] = ownergroupR;
            myauth.ownergroupW[myzone.zoneID] = ownergroupW;
            myauth.ownergroupE[myzone.zoneID] = ownergroupE;
        }
        //2.1调用子函数
            //查询在每个区域中是主人 还是 成员
            //然后获取相应身份的权限数据
            //并返回
        //添加到（建议用map）    
    }//for

    return myauth.serialize();
}
 //2.1调用子函数
int AuthModelServer::subAuthSearchModel(Zone &myzone, string hvsID, string &r, string &w, string &x , string &identity,
                                        string &ownergroupR, string &ownergroupW, string &ownergroupE){
    
    //2.1.1查询在每个区域中是主人 还是 成员
    if(myzone.ownerID.compare(hvsID)==0){
        identity = "1";
    }
    else{
        //不用查询了，因为这是这个用户所拥有的看见之一，因此不是owner，必然是member
        identity = "0";
    }

    //2.1.2然后获取相应身份的权限数据
    std::shared_ptr<hvs::CouchbaseDatastore> f3_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("auth_info"));
    f3_dbPtr->init();

    auto [pvalue, error] = f3_dbPtr->get(myzone.zoneID);
    if(error){
        cout << "fail" << endl;
        return -1;
    }

    Auth person;
    person.deserialize(*pvalue);

    //2.1.3判读是主人还是成员
    if(identity == "1"){ //主人
        r = to_string(person.owner_read);
        w = to_string(person.owner_write);
        x = to_string(person.owner_exe);
        ownergroupR = to_string(person.group_read);
        ownergroupW = to_string(person.group_write);
        ownergroupE = to_string(person.group_exe);
        return 0;
    }
    if(identity == "0"){
        r = to_string(person.group_read);
        w = to_string(person.group_write);
        x = to_string(person.group_exe);
        return 0;
    }

}

}//namespace hvs




//int chown(const char* path, uid_t owner, gid_t group);    fail :-1     success:0