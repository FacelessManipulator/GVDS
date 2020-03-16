/*
Author:Liubingqi
date:2019.03.21

g++ -c UserModelServer.cc
g++ -c gvdsrest.cc -lpistache  -std=c++11
g++ -o user UserModelServer.o gvdsrest.o -lpistache -std=c++11

./user 5
*/


#include <iostream>
#include <vector>
#include <stdio.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/centerinfo.h"

#include "manager/usermodel/UserModelServer.h"
#include "manager/usermodel/MD5.h"
#include "gvds_struct.h"

using namespace std;


//using namespace gvds;

//f0_dbPtr     account_info
        //f2_dbPtr     token_info       key:mtoken
        //f1_dbPtr     account_map_id   key: account
        //f0_dbPtr     account_info     key: uuid

//f1_dbPtr   sc_account_info
        //f3_dbPtr     sc_account_pool    key:Beijing  Shanghai Guangzhou Changsha Jinan
        //f4_dbPtr     sc_account_info    key:uuid

//f5_dbPtr  apply_info

namespace gvds{
using namespace Pistache::Rest;
using namespace Pistache::Http;

void UserModelServer::start() {}
void UserModelServer::stop() {}

void UserModelServer::router(Router& router){
    Routes::Post(router, "/users/modify", Routes::bind(&UserModelServer::modifyUserinfoRest, this));
    Routes::Get(router, "/users/search/:name", Routes::bind(&UserModelServer::getUserinfoRest, this));
    Routes::Post(router, "/users/registration", Routes::bind(&UserModelServer::UserRegisterRest, this));
    Routes::Post(router, "/users/login", Routes::bind(&UserModelServer::UserLoginRest,this));
    Routes::Get(router, "/users/exit/:id", Routes::bind(&UserModelServer::exitUserAccountRest, this));
    Routes::Get(router, "/users/cancel/:id", Routes::bind(&UserModelServer::cancellationUserAccountRest, this));
    Routes::Post(router, "/users/memberID", Routes::bind(&UserModelServer::getMemberIDRest, this));

    //管理员接口
    Routes::Post(router, "/users/adminregistration", Routes::bind(&UserModelServer::AdminUserRegisterRest, this)); //管理员注册
    Routes::Post(router, "/users/bufferuserregister", Routes::bind(&UserModelServer::bufferUserRegisterRest, this)); //新的用户账户注册接口，写如暂存区
    Routes::Post(router, "/users/listapply", Routes::bind(&UserModelServer::viewbufferListRest, this)); //展示暂存区内容
    Routes::Post(router, "/users/removeapply", Routes::bind(&UserModelServer::removeoneofApplyInfoRest, this)); //删除apply_info 内容   从暂存区删除
    Routes::Post(router, "/users/adcam", Routes::bind(&UserModelServer::adminCreateAccountMapping, this)); //管理员 手动建立、删除 账户映射接口
    Routes::Post(router, "/users/aduam", Routes::bind(&UserModelServer::adminDelAccountMapping, this));
    Routes::Post(router, "/users/adsearcham", Routes::bind(&UserModelServer::adminSearchAccountMapping, this)); // 管理员 查询账户映射信息
    //管理员查询 各个 超算账户池使用了
    Routes::Post(router, "/users/adsearchpool", Routes::bind(&UserModelServer::adminSearchAccountPoolRest, this)); 
    
    

}

void UserModelServer::getMemberIDRest(const Rest::Request& request, Http::ResponseWriter response){
     dout(-1) << "====== start UserModelServer function: getMemberIDRest ======"<< dendl;
    auto info = request.body();

    //反序列化
    vector<string> memberName;
    json_decode(info, memberName);


    vector<string> memberID;
    bool flag = getMemberID(memberName, memberID);
    if(flag){
        dout(-1) << "get getMemberIDRest success" << dendl;
        string json_str = json_encode(memberID); //序列号 返回
        response.send(Http::Code::Ok, json_str);
    }
    else{
        dout(-1) << "get getMemberIDRest fail" << dendl;
        response.send(Http::Code::Ok, "fail");
    }
    dout(-1) << "====== end UserModelServer function: getMemberIDRest ======"<< dendl;
}

bool UserModelServer::getMemberID(vector<string> &memberName, vector<string> &memberID){
    dout(-1) << " enter getMemberID "<< dendl;
    //获取每一个membername 对应的 gvdsid
    auto f0_dbPtr = DatastoreFactory::create_datastore(bucket_account_info, couchbase, true);
    for(int i=0; i<memberName.size(); i++){
        auto [pvalue, error_0] = f0_dbPtr->get(".USER_UUID_MAP", memberName[i]);
        if(error_0 != 0){  
            return false;
        }
        string accountID;
        json_decode(*pvalue, accountID);
        memberID.push_back(accountID);
    }
    return true;
}

//账户注册
void UserModelServer::UserRegisterRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: UserRegisterRest ======"<< dendl;

    auto info = request.body();
    //Account person("lbq", "123456", "78910", "XXXXXX@163.com", "15012349876", "xueyuanlu",  "Beihang");
/*
    Account person("lbq", "123456", "456", "XXXXXX@163.com", "15012349876", "xueyuanlu",  "Beihang", "has");
    person.sc.location_scacc["beijing"] = "local_lbq";
    person.sc.scacc_password["local_lbq"] = "654321";
    person.sc.location_scacc["shanghai"] = "local_lbq1";
    person.sc.scacc_password["local_lbq1"] = "654321";
*/


    Account person;
    person.deserialize(info);  

    string result = UserRegister(person);

    dout(-1)<<"result:"<<result<<dendl;
    response.send(Http::Code::Ok, result); //point
    dout(-1) << "====== end UserModelServer function: UserRegisterRest ======"<< dendl;
}


string UserModelServer::UserRegister(Account &person){
    dout(-1) << "enter UserRegister ======"<< dendl;

    //std::string person_key = person.accountID;
    //检查是否存在key，不存在，则进行下面代码开始注册，存在则返回注册失败
    
    auto f0_dbPtr = DatastoreFactory::create_datastore(bucket_account_info, couchbase, true);
    auto [pvalue, error_0] = f0_dbPtr->get(".USER_UUID_MAP", person.accountName);
    if(error_0 == 0){  
        return "Account name already exists!";
    }

    //不存在此key，开始注册
    //1、生成uuid
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    const std::string tmp_uuid = boost::uuids::to_string(a_uuid);

    std::string person_key = tmp_uuid;

    //赋值
    person.accountID = tmp_uuid;


    //2、写入account_map_info表
    
    int f1_flag = f0_dbPtr->set(".USER_UUID_MAP", person.accountName, json_encode(person.accountID));
    if (f1_flag == 13) {
        f0_dbPtr->set(".USER_UUID_MAP", "{}");
        f1_flag = f0_dbPtr->set(".USER_UUID_MAP", person.accountName, json_encode(person.accountID));
    }
    if (f1_flag != 0){
        dout(-1) << "Registration fail: DB[account_map_id] write fail;" << dendl;
        return "Registration fail";
    }

    //写入account_info表
    std::string person_value = person.serialize();  //json
   
    // std::shared_ptr<gvds::CouchbaseDatastore> f0_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
    //     gvds::CouchbaseDatastore(bucket_account_info));
    // f0_dbPtr->init();
    
    int flag = f0_dbPtr->set(user_prefix + person_key, person_value);
    if (flag != 0){
        dout(-1) << "Registration fail: DB[account_info] write fail;" << dendl;
        return "Registration fail";
    }
    else{
        string result_1 = "Dear:" + person.accountName + ", registration success";
        return result_1;
    }    
    //string result = "Dear user: " + person.GVDSAccountName + ", registration success. Please log in with your username and password";
    //return result;
}



//账户登录
//登录成功，返回：token   失败：-1 ，成功：uuid；
void UserModelServer::UserLoginRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: UserLoginRest ======"<< dendl;
    auto info = request.body(); 
    
    //解析账户名密码
    AccountPass acc_pass;
    acc_pass.deserialize(info);

    string userID;
    string identity; 
    bool is_success = UserLogin(acc_pass.accountName, acc_pass.Password, userID, identity);

    if (is_success){
        //md5
        string origin_str = acc_pass.accountName + acc_pass.Password; //再加上url以及时间等等信息
        string mtoken = md5(origin_str);
        std::shared_ptr<gvds::CouchbaseDatastore> f0_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
            gvds::CouchbaseDatastore(bucket_account_info));
        f0_dbPtr->init();
        string value ="1";
        int login_flag = f0_dbPtr->set(mtoken, value);
        if (login_flag != 0){
            // response.send(Http::Code::Ok, "login fail!,token set fail");
            response.send(Http::Code::Ok, "-1");
        }
        else{
            response.cookies().add(Http::Cookie("token", mtoken));
            string tmp_id = identity + "_" + userID;
            response.send(Http::Code::Ok, tmp_id); //point
        }
    }
    else{
        // response.send(Http::Code::Unauthorized, "login fail!");
         response.send(Http::Code::Ok, "-1");

    }

    //auto pmtoken = response.headers().get("Token");
    //dout(-1) << "pmtoken: " << pmtoken <<dendl;
    //dout(-1) << "*pmtoken: " << *pmtoken <<dendl;
   

    dout(-1)<<"====== end UserModelServer function: UserLoginRest ======"<<dendl;    
}

bool UserModelServer::UserLogin(std::string account, std::string pass, std::string &userID, std::string &identity){
    dout(-1) << "enter UserLogin"<< dendl;
    //AccountPair中实现新类，只存账户名，和id，这两个信息
    //查询账户名对应的id，作为数据库查询的key
    auto f0_dbPtr = DatastoreFactory::create_datastore(bucket_account_info, couchbase, true);
    //获取account对应的id    [若数据库没有此key，则返回登录失败的代码]
    auto [pvalue, error_0] = f0_dbPtr->get(".USER_UUID_MAP", account);
    if(error_0) {
        dout(-1) << "DB[account_map_id]: No such account" <<dendl;
        return false;
    }
    string key;
    json_decode(*pvalue, key);
    //判断登录者身份：普通用户 0 ；  管理员 1
    if(validadminidentity(key)){
        //是管理员
        identity = "1";
    }
    else{
        //普通用户
        identity = "0";
    }

    //[若数据库没有此key，则返回登录失败的代码]
    //auto [pPass, error_1] = f0_dbPtr->get(key, path);   *pPass输出带引号
    //tmp
    //使用获取的id查询数据库中密码，并比较
    auto [pvalue_2, error_2] = f0_dbPtr->get(user_prefix + key);
    if(error_2){
        dout(-1) << "DB[account_info]: No such account" <<dendl;
        return false;
    }

    Account tmp;
    tmp.deserialize(*pvalue_2);

    //pass == *pPass 密码一致
    if (!pass.compare(tmp.Password)) {
        string result = "login success";
        userID = key; //返回给客户端uuid
        return true;
    } 

    else{
        dout(-1)<<"pass = " << pass <<dendl;
        dout(-1) <<"tmp.Password = " << tmp.Password << dendl;
        string result = "login fail";
        return false;
    }

}


//账户信息查询
//返回33权限，-1失败，正常值；
void UserModelServer::getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: getUserinfoRest ======"<< dendl;

    // bool valid = auth_token(request);
    // if (!valid){
    //     response.send(Http::Code::Unauthorized, "33");
    //     return;
    // }
    
    
    auto uuid = request.param(":name").as<std::string>();
    
    bool is_get_success = true;
    // include your functin
    string data = getUserinfo(uuid, is_get_success);
    if(is_get_success){
        response.send(Http::Code::Ok, data);
    }
    else{
        response.send(Http::Code::Ok, "-1"); //point
    }
    
    dout(-1) << "====== end UserModelServer function: getUserinfoRest ======"<< dendl;
}

string UserModelServer::getUserinfo(string uuid, bool &is_get_success){
    dout(-1) << "enter getUserinfo"<< dendl;
    /*
    map<string, string> usermap;

    usermap["username"] = uuid;
    usermap.insert(pair<string, string>("email", "liubingqi112@163.com"));

    string nameValue = usermap["username"];
    string emailValue = usermap["email"];
    //for (map<string, string>::const_iterator iter = usermap.begin( ); iter != usermap.end( ); ++iter)


    string data = "{\"name\":\"" + nameValue + "\", \"email\":\"" + emailValue + "\"}" ;
    return data;

    */
    string key = uuid;

    auto f0_dbPtr = DatastoreFactory::create_datastore(bucket_account_info, couchbase, true);

    //判断key是否存在
    auto [pvalue, error] = f0_dbPtr->get(user_prefix + key);
    if (error){
        is_get_success = false;
        return "SearchFail";
    }

    dout(-1)<<"pvalue:"<< *pvalue <<dendl;
    return *pvalue;

}


// 返回33权限，应有结果；
void UserModelServer::modifyUserinfoRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: modifyUserinfoRest ======"<< dendl;

    // bool valid = auth_token(request);
    // if (!valid){
    //     response.send(Http::Code::Unauthorized, "Verification failed, access denied");
    //     return;
    // }

    auto info = request.body();

    Account person;
    person.deserialize(info);  

    string result = modifyUserinfo(person);

    dout(-1)<<"result:"<<result<<dendl;
    response.send(Http::Code::Ok, result); //point

    dout(-1) << "====== end UserModelServer function: modifyUserinfoRest ======"<< dendl;
}

string UserModelServer::modifyUserinfo(Account &person){
    dout(-1) << "enter modifyUserinfo"<< dendl;

    //更新account_info表
    string person_key = person.accountID;
    string person_value = person.serialize();  //json

    dout(-1) << person_key << dendl;
    dout(-1) << person_value << dendl;
   
    std::shared_ptr<gvds::CouchbaseDatastore> f0_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
        gvds::CouchbaseDatastore(bucket_account_info)); 
    f0_dbPtr->init();
    
    int flag = f0_dbPtr->set(user_prefix + person_key, person_value);
    if (flag != 0){
        dout(-1) << "Modify fail: DB[account_info] update fail" << dendl;
        return "Modify fail";
    }
    else{
        return "Modify success";
    }    

}

//返回：33权限，字符串-1失败, 正常值；
void UserModelServer::exitUserAccountRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: exitUserAccountRest ======"<< dendl;

    // bool valid = auth_token(request);
    // if (!valid){
    //     response.send(Http::Code::Unauthorized, "33");
    //     return;
    // }
    
    //auto uuid = request.param(":id").as<std::string>();
    
    std::string name;
    std::string mtoken;
    auto cookies = request.cookies();
    for (const auto& c: cookies) {
        //dout(-1) << c.name << " = " << c.value << dendl;
        name = c.name;
        mtoken = c.value;
    }

    bool is_exit_success = true;
    
    string data = exitUserAccount(mtoken, is_exit_success);
    if(is_exit_success){
        response.send(Http::Code::Ok, "0");
    }
    else{
        response.send(Http::Code::Ok, "-1"); //point
    }
    
    dout(-1) << "====== end UserModelServer function: exitUserAccountRest ======"<< dendl;
}


string UserModelServer::exitUserAccount(std::string mtoken , bool &is_exit_success){
    dout(-1) << "enter exitUserAccount"<< dendl;

    auto f0_dbPtr = DatastoreFactory::create_datastore(bucket_account_info, couchbase, true);

    int flag = f0_dbPtr->remove(mtoken);
    if(flag == 0){
        is_exit_success = true;
        return "Eixt success";
    }
    else{
        is_exit_success = true;
        return "-1";
    }
    
//删除token，返回登录页
}


//彻底注销虚拟数据空间用户
//返回33权限，失败：-1，-2，成功：正常结果
void UserModelServer::cancellationUserAccountRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: cancellationUserAccountRest ======"<< dendl;

    // bool valid = auth_token(request);
    // if (!valid){
    //     response.send(Http::Code::Unauthorized, "33");
    //     return;
    // }

    auto uuid = request.param(":id").as<std::string>();
    bool is_cancel_success = true;

    string data = cancellationUserAccount(uuid, is_cancel_success);
    if(is_cancel_success){
        response.send(Http::Code::Ok, data);
    }
    else{
        response.send(Http::Code::Ok, data); //point
    }
    
    dout(-1) << "====== end UserModelServer function: cancellationUserAccountRest ======"<< dendl;
}

string UserModelServer::cancellationUserAccount(string uuid, bool is_cancel_success){
    dout(-1) << "enter cancellationUserAccount"<< dendl;

    //获取用户信息
    auto f0_dbPtr = DatastoreFactory::create_datastore(bucket_account_info, couchbase, true);

    auto [pvalue, error] = f0_dbPtr->get(user_prefix + uuid);
    if(error){
        is_cancel_success = false;
        dout(-1) << "access to db[account_info] fail" << dendl;
        return "-1";
    }
    Account gvdsperson;
    gvdsperson.deserialize(*pvalue);

    //TODO判断区域是否注销完毕sy
    // std::string query = "select * from `"+zonebucket+"` where owner = \"ownerID\";";
    // int pos = query.find("ownerID");
    // query.erase(pos, 7);
    // query.insert(pos, uuid);
    // std::shared_ptr<gvds::Datastore> dbPtr = gvds::DatastoreFactory::create_datastore(zonebucket, gvds::DatastoreType::couchbase);
    // auto zonePtr = static_cast<CouchbaseDatastore*>(dbPtr.get());
    // auto [vp, err] = zonePtr->n1ql(query);
    // if(vp->size() != 0){
    //     is_cancel_success = false;
    //     dout(-1) << "User cancellation fail, your Zone is not cancal success" << dendl;
    //     return "-2";
    // }

    //TODO判断区域是否注销完毕sy
    string ownerID = uuid;
    char query[256];
    snprintf(query, 256, 
    "select * from `%s` where owner = \"%s\"", 
    zonebucket.c_str(), ownerID.c_str());
    std::shared_ptr<gvds::Datastore> dbPtr = gvds::DatastoreFactory::create_datastore(zonebucket, gvds::DatastoreType::couchbase);
    auto zonePtr = static_cast<CouchbaseDatastore*>(dbPtr.get());
    auto [vp, err] = zonePtr->n1ql(string(query));
    if(vp->size() != 0){
        is_cancel_success = false;
        dout(-1) << "User cancellation fail, your Zone is not cancal success" << dendl;
        return "-2";
    }

    std::shared_ptr<gvds::CouchbaseDatastore> f1_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
        gvds::CouchbaseDatastore(bucket_sc_account_info));  
    f1_dbPtr->init();
     
    int flag = f1_dbPtr->remove(sc_user_prefix + uuid);
    if(flag){
        is_cancel_success = false;
        dout(-1) << "Remove account fail" << dendl;
        return "-1";
    }

    //调用账户退出子函数,删除token，但是不删也没事

    
    //删除account_map_id
    int acc_map_id_flag = f0_dbPtr->remove(gvdsperson.accountName);
    if(acc_map_id_flag){
        is_cancel_success = false;
        dout(-1) << "when db[account_map_id] remove, fail" << dendl;
        return "-1";
    }

    //删除account_info
    int acc_info_flag = f0_dbPtr->remove(user_prefix + uuid);
    if(acc_info_flag){
        is_cancel_success = false;
        dout(-1) << "when db[account_map_id] remove, fail" << dendl;
        return "-1";
    }

    is_cancel_success = true;
    return "User cancellation success.";
}
 
 /* 
bool UserModelServer::RemoveAccountMapping_old(string accountID){

    std::shared_ptr<gvds::CouchbaseDatastore> f1_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
        gvds::CouchbaseDatastore(bucket_sc_account_info));  
    f1_dbPtr->init();

    auto [pvalue_scuser, error] = f1_dbPtr->get(accountID);  //sc_account_info  key:uuid
    if(error){
        dout(-1) << "get sc_account_info fail."<< dendl;
        return false;
    }

    SCAccount person;
    person.deserialize(*pvalue_scuser);
    
    //删除受hostCenterName影响，因为一下删除5个地点，因此这块不用改
    bool beijing = SubRemoveAccountMapping_old(person, "Beijing", f1_dbPtr);  //sc_account_pool  key:Beijing,Shanghai...
    bool shanghai = SubRemoveAccountMapping_old(person, "Shanghai", f1_dbPtr);
    bool guangzhou = SubRemoveAccountMapping_old(person, "Guangzhou", f1_dbPtr);
    bool changsha = SubRemoveAccountMapping_old(person, "Changsha", f1_dbPtr);
    bool jinan = SubRemoveAccountMapping_old(person, "Jinan", f1_dbPtr);
    
    if(!(beijing && shanghai && guangzhou && changsha && jinan)){
       return false;
    }
    else{
       return true;
    }
}

//删除一个用户指定地区(location)的［所有］账户映射
bool UserModelServer::SubRemoveAccountMapping_old(SCAccount &person, string location, shared_ptr<gvds::CouchbaseDatastore> f1_dbPtr){
    dout(-1) << "location " << location << dendl;
    auto [pvalue_location, error] = f1_dbPtr->get(location);    //sc_account_pool  key:Beijing,Shanghai...
    if (error){
        dout(-1) << "get "<< location << " err"<<dendl;
        return false;
    }
    //dout(-1) << *pvalue_location <<dendl;

    AccountSCPool somewhere;
    somewhere.deserialize(*pvalue_location);
    map<string, string>::iterator iter;

    //每次for循环做的事
    //修改相应地区账户池，key1添加到unuse中，然后在use中删除该key1
    //删除该用户的映射
    if(!location.compare("Beijing")){    
        for(iter = person.Beijing_account.begin(); iter != person.Beijing_account.end(); iter++){  //删除一个用户指定地区(location)的所有账户映射，//如果为空则不执行for循环，没有任何影响
            somewhere.unuse_account[iter->first] = iter->second;   //account  password
            somewhere.use_account.erase(iter->first);

            person.Beijing_account.erase(iter->first);
        }
    }
    else if(!location.compare("Shanghai")){
        for(iter = person.Shanghai_account.begin(); iter != person.Shanghai_account.end(); iter++){
            somewhere.unuse_account[iter->first] = iter->second;   //account  password
            somewhere.use_account.erase(iter->first);

            person.Shanghai_account.erase(iter->first);
        }
    }
    else if(!location.compare("Guangzhou")){
        for(iter = person.Guangzhou_account.begin(); iter != person.Guangzhou_account.end(); iter++){
            somewhere.unuse_account[iter->first] = iter->second;   //account  password
            somewhere.use_account.erase(iter->first);

            person.Guangzhou_account.erase(iter->first);
        }
    }
    else if(!location.compare("Changsha")){
        for(iter = person.Changsha_account.begin(); iter != person.Changsha_account.end(); iter++){
            somewhere.unuse_account[iter->first] = iter->second;   //account  password
            somewhere.use_account.erase(iter->first);

            person.Changsha_account.erase(iter->first);
        }
    }
    else if(!location.compare("Jinan")){
        for(iter = person.Jinan_account.begin(); iter != person.Jinan_account.end(); iter++){
            somewhere.unuse_account[iter->first] = iter->second;   //account  password
            somewhere.use_account.erase(iter->first);

            person.Jinan_account.erase(iter->first);
        }
    }

    //更新数据库的sc_account_pool  和  sc_account_info 
    string value1 = somewhere.serialize();
    int flag1 = f1_dbPtr->set(location, value1);  //sc_account_pool
    if (flag1 != 0){
        dout(-1)<< "remove map fail: DB[sc_account_pool] update fail"<< dendl;
        return false;
    }
    else{
        dout(-1)<< "remove map success: DB[sc_account_pool] update success" << dendl;
    }    
    
    string value2 = person.serialize();
    int flag2 = f1_dbPtr->set(person.accountID, value2);   //sc_account_info
    if (flag2 != 0){
        dout(-1)<< "remove map fail: DB[sc_account_info] update fail"<<dendl;
        return false;
    }
    else{
        dout(-1)<< "removeAccountMapping success: DB[sc_account_info] update success" << dendl;
        return true;
    }    

}

//register，first, build account mapping 
bool UserModelServer::BuildAccountMapping_old(string accountID){
    
    //get sc_account_pool
    
    SCAccount person(accountID);

    std::shared_ptr<gvds::CouchbaseDatastore> f1_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
        gvds::CouchbaseDatastore(bucket_sc_account_info));  
    f1_dbPtr->init();
    
    //账户映射算法，选一个地点进行映射，目前是默认Beijing，如果不做算法，这里就直接映射5个就完事了（直接把下面四个//取消）
    //参数是数据库中的key，因此不能换，这里要做转换， if ... key = "Beijing"
    bool beijing = SubBuildAccountMapping(person, "Beijing", f1_dbPtr);   //sc_account_pool
    //bool shanghai = SubBuildAccountMapping(person, "Shanghai", f1_dbPtr);
    //bool guangzhou = SubBuildAccountMapping(person, "Guangzhou", f1_dbPtr);
    //bool changsha = SubBuildAccountMapping(person, "Changsha", f1_dbPtr);
    //bool jinan = SubBuildAccountMapping(person, "Jinan", f1_dbPtr);

    
   if(beijing){
       return true;
   }
   else{
       return false;
   }

}

//20190621 不用修改，这个是活的
//建立给定地区(location)的用户的［1个］账户映射，即调用Beijing两次，则在Beijing建立2个账户映射; 删除是直接删除给定地区的所有账户映射，若想只删除给定地区的其中一个账户，则需再自定函数实现
bool UserModelServer::SubBuildAccountMapping_old(SCAccount &person, string location, shared_ptr<gvds::CouchbaseDatastore> f1_dbPtr){
    
    dout(-1) << "location " << location << dendl;
    auto [pvalue_location, error] = f1_dbPtr->get(location);
    if (error){
        dout(-1) << "get "<< location << " err"<<dendl;
        return false;
    }
    
    dout(-1) << *pvalue_location <<dendl;

    AccountSCPool somewhere;
    somewhere.deserialize(*pvalue_location);
    if(somewhere.unuse_account.empty()){
        dout(-1) << location <<" .unuse_account is empty , no local account can be used to build accountmap. Fail. " << dendl;
        return false;
    }
    map<string, string>::iterator iter;
    iter = somewhere.unuse_account.begin();  //从未用账户池中取出一个账户
    //add to person

    if(!location.compare("Beijing")){
        person.Beijing_account[iter->first] = iter->second; //account   password
        dout(-1) << "confirm Beijing" <<dendl;
    }
    else if(!location.compare("Shanghai")){
        person.Shanghai_account[iter->first] = iter->second;   
        dout(-1) << "confirm Shanghai" <<dendl;
    }
    else if(!location.compare("Guangzhou")){
        person.Guangzhou_account[iter->first] = iter->second;  
        dout(-1) << "confirm Guangzhou" <<dendl;
    }
    else if(!location.compare("Changsha")){
        person.Changsha_account[iter->first] = iter->second; 
        dout(-1) << "confirm Changsha" <<dendl;
    }
    else if(!location.compare("Jinan")){
        person.Jinan_account[iter->first] = iter->second;   
        dout(-1) << "confirm Jinan" <<dendl;
    }
    //add to use
    somewhere.use_account[iter->first] = iter->second;
    //del from unuse
    somewhere.unuse_account.erase(iter->first);

    //更新两个数据库表sc_account_pool  sc_account_info
    string value = somewhere.serialize();   //sc_account_pool
    int flag = f1_dbPtr->set(location, value);
    if (flag != 0){
        dout(-1)<< "map fail: DB[sc_account_pool] update fail"<< dendl;
        return false;
    }
    else{
        dout(-1)<< "map success: DB[sc_account_pool] update success" << dendl;
    }    

    //dout(-1) << person.Changsha_account.size() <<dendl;
    string person_value = person.serialize();

    int flag2 = f1_dbPtr->set(person.accountID, person_value); //sc_account_info
    if (flag2 != 0){
        dout(-1)<< "map fail: DB[sc_account_info] update fail"<<dendl;
        return false;
    }
    else{
        dout(-1)<< "BuildAccountMapping success: DB[sc_account_info] update success" << dendl;
        return true;
    }    
    
}
*/

//--------------------------------------------------------------------
//管理员接口
//管理员账户注册：和原账户注册逻辑基本一致，只是多了记录 adminwhitelist 这个操作
//返回值 与 原先注册接口相比 无变化 不要改vue客户端
void UserModelServer::AdminUserRegisterRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: AdminUserRegisterRest ======"<< dendl;

    auto info = request.body();

    Account person;
    person.deserialize(info);  

    string result = AdminUserRegister(person);

    dout(-1)<<"result:"<<result<<dendl;
    response.send(Http::Code::Ok, result); //point
    dout(-1) << "====== end UserModelServer function: AdminUserRegisterRest ======"<< dendl;
}


string UserModelServer::AdminUserRegister(Account &person){
    dout(-1) << "enter UserRegister ======"<< dendl;

    //std::string person_key = person.accountID;
    //检查是否存在key，不存在，则进行下面代码开始注册，存在则返回注册失败
    
    auto f0_dbPtr = DatastoreFactory::create_datastore(bucket_account_info, couchbase, true);
    auto [pvalue, error_0] = f0_dbPtr->get(".USER_UUID_MAP", person.accountName);
    if(error_0 == 0){  
        return "Account name already exists!";
    }

    //不存在此key，开始注册
    //1、生成uuid
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    const std::string tmp_uuid = boost::uuids::to_string(a_uuid);

    std::string person_key = tmp_uuid;

    //赋值
    person.accountID = tmp_uuid;


    //2、写入account_map_info表
    int f1_flag = f0_dbPtr->set(".USER_UUID_MAP", person.accountName, json_encode(person.accountID));
    if (f1_flag == 13) {
        f0_dbPtr->set(".USER_UUID_MAP", "{}");
        f1_flag = f0_dbPtr->set(".USER_UUID_MAP", person.accountName, json_encode(person.accountID));
    }
    if (f1_flag != 0){
        dout(-1) << "Registration fail: DB[account_map_id] write fail;" << dendl;
        return "Registration fail";
    }

    //sc_accounut_pool取出并调整,   sc_account_info写入
    // 管理员账户不用建立本地账户映射
    // bool buildmap = BuildAccountMapping_v2(person.accountID);
    // if(!buildmap){
    //     dout(-1) << "buildmap fail" << dendl;
    //     return "Registration fail";
    // }

    //管理员特有，将此uuid记录进 adminwhiteuser
    auto raw_dbc = static_cast<gvds::CouchbaseDatastore*>(f0_dbPtr.get())->get_raw_client();
    int admin_flag = raw_dbc->arr_insert_uniq(adminlist, "namelist", json_encode(person.accountID)).status().errcode();
    if(admin_flag!=0){
        dout(-1) << "Registration fail: cannot insert admin into admin white list;" << dendl;
        return "Registration fail";
    }
   

    //写入account_info表
    std::string person_value = person.serialize();  //json
    int flag = f0_dbPtr->set(user_prefix + person_key, person_value);
    if (flag != 0){
        dout(-1) << "Registration fail: DB[account_info] write fail;" << dendl;
        return "Registration fail";
    }
    else{
        string result_1 = "Dear:" + person.accountName + ", registration success";
        return result_1;
    }    
    //string result = "Dear user: " + person.GVDSAccountName + ", registration success. Please log in with your username and password";
    //return result;
}

//新用户注册，结合老的一起用； 
//用户注册请求都发到这个接口里来
//用户注册请求先写入apply_info 数据库，管理员同意后，再调原来的账户注册接口。
// "0" 是请求成功   "11"是请求失败
void UserModelServer::bufferUserRegisterRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: bufferUserRegister ======"<< dendl;
    auto info = request.body();

    dout(-1) << "info: " << info << dendl;

    //dout(-1) << "viewbufferList start ============" << dendl;
    //auto a = viewbufferList("15cdc484-5097-49ad-a02a-33ef359d8bea");
    //dout(-1) << "a: " << a;
    //dout(-1) << "viewbufferList end ============" << dendl;

    int result = bufferUserRegister(info);
    response.send(Http::Code::Ok, json_encode(result));
    dout(-1) << "result: " <<result << dendl;
    dout(-1) << "====== end UserModelServer function: bufferUserRegister ======"<< dendl;
}

int UserModelServer::bufferUserRegister(std::string apply){
    std::shared_ptr<gvds::Datastore> f5_dbPtr =gvds::DatastoreFactory::create_datastore(applybucket, gvds::DatastoreType::couchbase);
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    const std::string uuid = boost::uuids::to_string(a_uuid);
    std::string key = "usign-" + uuid;

    struct_apply_info singel_content;
    singel_content.id = key;
    singel_content.data = apply;
    string value = singel_content.serialize();
    dout(-1) << "see_value: " << dendl;
    dout(-1) << value << dendl;

    int flag = f5_dbPtr->set(key, value);
    if(flag != 0)
    {
      return errno = EAGAIN;
    }
    else return 0;
}



//管理员查看apply_info中的请求
//返回值：  "33"不是管理员 ，"1" 是失败    其他是成功(json_encode(my_apply);)
void UserModelServer::viewbufferListRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: viewbufferListRest ======"<< dendl;
    auto info = request.body();

    dout(-1) << "info: " << info << dendl;  // 管理员id
    string data = viewbufferList(info);
    dout(-1) << "data: " << data << dendl;
    response.send(Http::Code::Ok, data);

    dout(-1) << "====== end UserModelServer function: viewbufferListRest ======"<< dendl;
}
string UserModelServer::viewbufferList(std::string gvdsID){
    //验证 gvdsID是否管理员id
    if(!validadminidentity(gvdsID)){
        return "33";// 不是管理员
    }
    std::shared_ptr<gvds::Datastore> f5_dbPtr =gvds::DatastoreFactory::create_datastore(applybucket, gvds::DatastoreType::couchbase);
    auto applyPtr = static_cast<CouchbaseDatastore*>(f5_dbPtr.get());
    
    //把bucket的数据库 的 所用 或者前5条 返回给客户端
    std::string query = "select id,data from " + applybucket +" where meta().id like \"usign-%\" or meta().id like \"zregi-%\" or meta().id like \"spadd-%\" or meta().id like \"spsiz-%\" limit 5";
    dout(10) << "query: "<< query << dendl;
    auto [vp, err] = applyPtr->n1ql(query);
    if(err!=0){
        return "1";
    }
    //dout(-1) << "*vp: " <<dendl;
    //dout(-1) << *vp << dendl;
    //vector<vector<struct_apply_info> > my;
    //json_decode(*vp, my);
    // vector<string> my_apply;
    // for(auto iter =vp->begin(); iter!=vp->end(); iter++){
    //     string con = *iter;
    //     int len = con.size();
    //     string tmp = con.substr(sizeof("{\"test\":")-1, len-1);
    //     dout(10) << "tmp: " << tmp.substr(0, tmp.size()-1) << dendl;

    //     my_apply.push_back(tmp.substr(0, tmp.size()-1));
    //     //vector<struct_apply_info> inner_vec;
    //     //json_decode(tmp, inner_vec);
    // }
    // string json_str = json_encode(my_apply);
    //查询出了多条记录，如何一条一条的 添加进vector ？
    // dout(10) << "json_str: " << json_str << dendl;
    return json_encode(*vp);
}

//管理原 accept  客户端 调此接口，删除记录， 并发送到对应功能接口
//管理员 refuse 客户端 调此接口，删除记录
void UserModelServer::removeoneofApplyInfoRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: removeoneofApplyInfoRest ======"<< dendl;
    auto info = request.body();
    dout(-1) << "info: " << info << dendl;

    std::shared_ptr<gvds::Datastore> f5_dbPtr =gvds::DatastoreFactory::create_datastore(applybucket, gvds::DatastoreType::couchbase);
    int flag = f5_dbPtr->remove(info);
    if(flag == 0){
        response.send(Http::Code::Ok, "0");  //删除成功
    }
    else{
        response.send(Http::Code::Ok, "1");  //删除失败
    }
}


//管理新建 和 删除账户映射
//客户端需先获取对应用户的gvdsID，若获取不到，则有问题；获取到后  发生gvdsID 和hostCenterName
// 返回 0 是成功，1是失败  33是没有权限
void UserModelServer::adminCreateAccountMapping(const Rest::Request& request, Http::ResponseWriter response){
    response.send(Http::Code::Ok, "0");
}

//返回 33 权限不组 1 失败  0成功
void UserModelServer::adminDelAccountMapping(const Rest::Request& request, Http::ResponseWriter response){
    response.send(Http::Code::Ok, "0");// 成功
}

//管理员账户映射查询
//返回 33 权限不足;     1失败; 其他(SCAccount) 成功;
void UserModelServer::adminSearchAccountMapping(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: adminSearchAccountMapping ======"<< dendl;
    auto info = request.body();
    dout(-1) << "info: " << info << dendl;   //

    struct_AdminAccountMap new_accountmap;
    new_accountmap.deserialize(info);     //new_accountmap.hostCenterName 并没有用到，只是借用struct_AdminAccountMap 里的 adgvdsID 和gvdsID字段
    // 若不是管理员，直接返回
    if(!validadminidentity(new_accountmap.adgvdsID)){
        response.send(Http::Code::Ok, "33");// 不是管理员
        return;
    }

    std::shared_ptr<gvds::CouchbaseDatastore> f1_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
        gvds::CouchbaseDatastore(bucket_sc_account_info));  
    f1_dbPtr->init();

    auto [pvalue_scuser, error] = f1_dbPtr->get(sc_user_prefix + new_accountmap.gvdsID);  //sc_account_info  key:uuid
    if(error){
        dout(-1) << "get sc_account_info fail."<< dendl;
        response.send(Http::Code::Ok, "1");// 失败
        return;
    }

    // SCAccount person;
    // person.deserialize(*pvalue_scuser);
    response.send(Http::Code::Ok, *pvalue_scuser);// 成功

}

//管理员查看账户池的情况  
void UserModelServer::adminSearchAccountPoolRest(const Rest::Request& request, Http::ResponseWriter response){
    dout(-1) << "====== start UserModelServer function: adminSearchAccountPoolRest ======"<< dendl;
    auto info = request.body();
    dout(-1) << "info: " << info << dendl;  //info 是adgvdsID

    std::string data = adminSearchAccountPool(info);
    response.send(Http::Code::Ok, data);
}

//管理员 各个 账户池 使用量查询
//33 权限问题    1 失败  其他(struct_infoAccountPool)成功
std::string UserModelServer::adminSearchAccountPool(std::string adgvdsID){
    if(!validadminidentity(adgvdsID)){
        return "33";
    }

    //获取account_info center_information中的超算中心名字，并以该名字作为key去查询 sc_account_info 中的超算池
    std::shared_ptr<gvds::CouchbaseDatastore> f0_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
        gvds::CouchbaseDatastore(bucket_account_info));
    f0_dbPtr->init();

    auto [pvalue, error_0] = f0_dbPtr->get(c_key);   //c_key 在构造函数里
    if(error_0){
        dout(-1) << "get DB center_information fail" <<dendl;
        return "1";
    }
    CenterInfo mycenter;   //超算中心的信息 , 主要使用其中的名字信息
    mycenter.deserialize(*pvalue);


    std::shared_ptr<gvds::CouchbaseDatastore> f1_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
        gvds::CouchbaseDatastore(bucket_sc_account_info));  
    f1_dbPtr->init();

    struct_infoAccountPool acpool;   //返回给客户端的查询信息

    bool tmp = false;// 防止不进入下面的循环
    for(auto iter = mycenter.centerID.begin(); iter!= mycenter.centerID.end(); iter++){
        std::string name = mycenter.centerName[*iter];
        dout(-1) << "-------- name :" << name << dendl;

        auto [pv, pv_err] = f1_dbPtr->get(name);
        if(pv_err){ //!=0
            dout(-1) << "get DB centerName[sc_account_pool] fail" <<dendl;
            return "1"; //失败
        }

        AccountSCPool center_pool;
        center_pool.deserialize(*pv);
        int usecount = 0, unusecount = 0;
        for(auto in_iter_ua = center_pool.unuse_account.begin(); in_iter_ua != center_pool.unuse_account.end(); in_iter_ua++){
            unusecount++;
        }
        for(auto in_iter_a = center_pool.use_account.begin(); in_iter_a != center_pool.use_account.end(); in_iter_a++){
            usecount++;
        }

        acpool.hostCenterName.push_back(name);
        acpool.usecount[name] = usecount;
        acpool.unusecount[name] = unusecount;
        tmp = true;
    }

    if(!tmp){
        dout(-1) << "no entry the circle" << dendl;
        return "1";// 查询失败
    }
    return acpool.serialize();
}

//验证是否是管理员
bool UserModelServer::validadminidentity(string gvdsID){
    std::shared_ptr<gvds::Datastore> f0_dbPtr =gvds::DatastoreFactory::create_datastore(bucket_account_info, gvds::DatastoreType::couchbase);

    auto [admin_vp, admin_err] = f0_dbPtr->get(adminlist);
    if(admin_err){ //!=0
        return false;// 身份不是管理员
    }

    struct_AdminList tmp_list;
    tmp_list.deserialize(*admin_vp);
    for (int i=0; i<tmp_list.namelist.size(); i++){
        if(gvdsID == tmp_list.namelist[i]){
            return true;
        }
    }
    return false;
}

//MD5

string md5(string strPlain){
		MD5_CTX mdContext;
		int bytes;
		unsigned char data[1024];
 
		MD5Init(&mdContext);
		MD5Update(&mdContext, (unsigned char*)const_cast<char*>(strPlain.c_str()), strPlain.size());
		MD5Final(&mdContext);
 
		string md5;
		char buf[3];
		for (int i = 0; i < 16; i++)
		{
			sprintf(buf, "%02x", mdContext.digest[i]);
			md5.append(buf);
		}
		return md5;
}


void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    dout(-1) << "Cookies: [" << dendl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        dout(-1) << indent << c.name << " = " << c.value << dendl;
    }
    dout(-1) << "]" << dendl;
}


bool auth_token(const Rest::Request& request){
    dout(-1) << "function: auth_token"<< dendl;
    std::string name;
    std::string mtoken;
    auto cookies = request.cookies();
    for (const auto& c: cookies) {
        //dout(-1) << c.name << " = " << c.value << dendl;
        name = c.name;
        mtoken = c.value;
    }
    std::shared_ptr<gvds::CouchbaseDatastore> f0_dbPtr = std::make_shared<gvds::CouchbaseDatastore>(
        gvds::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();

   
    auto [vp, err] = f0_dbPtr->get(mtoken);
    if(err){ //!=0
        return false;//验证失败
    }
    return true; //验证成功
}


}// namespace gvds