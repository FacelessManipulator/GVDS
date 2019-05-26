#include <iostream>
#include <vector>
#include <stdio.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"

#include "manager/authmodel/AuthModelServer.h"
//include "manager/usermodel/Account.h"
#include "manager/usermodel/UserModelServer.h"
#include <unistd.h>

using namespace std;

//f3_dbPtr     auth_info

namespace hvs{
using namespace Pistache::Rest;
using namespace Pistache::Http;

void AuthModelServer::start() {}
void AuthModelServer::stop() {}

void AuthModelServer::router(Router& router){}


//1权限增加模块
//1.1 区域初始权限记录接口  :: 被区域注册模块调用 :: 只记录数据库,不需访问存储集群设置权限,sy那边会设置
int ZonePermissionAdd(std::string zoneID, std::string ownerID){
    Auth person(zoneID);
    person.HVSAccountID = ownerID;
    person.owner_read = 1;
    person.owner_write = 1;
    person.owner_exe = 1;

    person.group_read = 1;
    person.group_write = 1;
    person.group_exe = 1;

    person.other_read = 1;
    person.other_write = 1;
    person.other_exe = 1;
    
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

//1.2 空间权限同步接口  :: 被空间创建模块调用 :: 查询空间所属区域的权限，设置空间为此权限
int SpacePermissionSyne(std::string spaceID, std::string zoneID, std::string ownerID){
    
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




    //2、根据区域权限设置空间的权限
        //2.1 获取ownerid 对应的的本地账户(空间所在地)test1;
    UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("auth").get());
    //TODO   根据空间获取hostCenterName 值
    string hostCenterName;
    string value = p_usermodel->getLocalAccountinfo(ownerID, hostCenterName);
    if (value.compare("fail") == 0){
        cout << "SpacePermissionSyne fail" << endl;
        return -1;
    }
    LocalAccountPair localpair;
    localpair.deserialize(value);
    //localpair.localaccount 这个是账户名

        //2.2 更改文件夹所属用户和组chown -R test1:test1 空间名
    //TODO   根据空间获取物理路径 spacepath值
    string spacepath = "/root/data";
    //TODO  获取账户名(localpair.localaccount)和组名（localpair.localaccount）的uid、gid
    int uid;
    int gid;

    if(chown(spacepath.c_str(), uid, gid) == -1){
        return -1;
    }
        //2.3 设置权限：chmod （au_person）(au_group)0 文件名
    if(auth_sum_person = 7){

    }
    string tmp_str = "0" + au_person + au_group + au_other;
    int tmp_int = atoi(tmp_str.c_str());
    //TODO  注意确认这块设置权限是否有问题
    if (chmod(spacepath.c_str(), tmp_int) == -1){
        return -1;
    }

 
    //不需要记录数据库

    return 0;

}


//1.3 副本权限同步接口   :: 被空间创建模块调用 ::  这个和sy商量下
int ReplacePermissionSyne(){

}

//1.4 空间定位接口      我调sy

//1.5 成员权限增加接口      ::区域共享模块调用  :: 设置新成员对区域的权限，记录数据库 
int ZoneMemberAdd(string zoneID, string ownerID, vector<string> memberID){
    
//for owner所有超算的空间获取本地账户

    //先假设一个空间
    //1、获取ownerID 本地对应的超算账户，testowner     这个要知道区域的每个空间都在哪个超算？sy
    vector<string>::iterator iter;
    for(iter = memberID.begin(); iter != memberID.end(); iter++){
        //获取*iter对应的相应超算的本地账户，test1
        //将test1加入与testowner同名的组中 usermod -a -G testowner test1
    }

    //数据库中无需记录任何东西

    return 0;
}

//1.6
//setPermission(string uuid, string local, string )

//=============================================
//2、权限删除模块
//2.1 区域注销模块     ::被区域注销模块调用::删除相应权限,删除数据库中权限记录，无需知道ownerid
int ZonePermissionDeduct(string zoneID, string OwnerID){
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

//2.2 成员权限删除接口  ::被区域共享模块调用::删除成员对区域的权限，不需要记录数据库 //区域拥有者的空间都在哪些超算，要分别加到每个超算对应的组
int ZoneMemberDel(string zoneID, string OwnerID, vector<string> memberID){

//for owner所有超算的空间获取本地账户

//到存储集群上从ownerid对应的本地账户（哪个中心）testowner对应的组testowner中删除 这些memberid
    vector<string>::iterator iter;
    for(iter = memberID.begin(); iter != memberID.end(); iter++){
        //获取*iter对应的相应超算的本地账户，test222
        //将test222从testowner同名的组中删除 gpasswd testowner -d test222
    }

//不需要改动数据库，
}

//2.3 空间定位，我调用sy 


//2.4 空间权限删除接口  ：：被空间创建模块调用：：删除存储集群上空间对应的权限  //数据库不用更新，因为无此空间记录
int SpacePermissionDelete(string spaceID){
    //获取空间物理路径  直接把拥有者和组改成root //chown -R root:root 文件名
    
}


//2.5


//3、权限修改模块，提供一个rest api，让前端调用


}//namespace hvs




//int chown(const char* path, uid_t owner, gid_t group);    fail :-1     success:0