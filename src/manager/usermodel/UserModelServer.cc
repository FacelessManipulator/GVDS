/*
Author:Liubingqi
date:2019.03.21

g++ -c UserModelServer.cc
g++ -c hvsrest.cc -lpistache  -std=c++11
g++ -o user UserModelServer.o hvsrest.o -lpistache -std=c++11

./user 5
*/


#include <iostream>
#include <vector>
#include <stdio.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"


#include "manager/usermodel/UserModelServer.h"
#include "manager/usermodel/MD5.h"


using namespace std;


//using namespace hvs;

//f0_dbPtr     account_info
        //f2_dbPtr     token_info       key:mtoken
        //f1_dbPtr     account_map_id   key: account
        //f0_dbPtr     account_info     key: uuid

//f1_dbPtr   sc_account_info
        //f3_dbPtr     sc_account_pool    key:Beijing  Shanghai Guangzhou Changsha Jinan
        //f4_dbPtr     sc_account_info    key:uuid

namespace hvs{
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
}

void UserModelServer::getMemberIDRest(const Rest::Request& request, Http::ResponseWriter response){
     cout << "====== start UserModelServer function: getMemberIDRest ======"<< endl;
    auto info = request.body();
    cout << info << endl;

    //反序列化
    vector<string> memberName;
    json_decode(info, memberName);


    vector<string> memberID;
    bool flag = getMemberID(memberName, memberID);
    if(flag){
        cout << "get getMemberIDRest success" << endl;
        string json_str = json_encode(memberID); //序列号 返回
        response.send(Http::Code::Ok, json_str);
    }
    else{
        cout << "get getMemberIDRest fail" << endl;
        response.send(Http::Code::Ok, "fail");
    }
    cout << "====== end UserModelServer function: getMemberIDRest ======"<< endl;
}

bool UserModelServer::getMemberID(vector<string> &memberName, vector<string> &memberID){
    cout << " enter getMemberID "<< endl;
    
    //获取每一个membername 对应的 hvsid
     std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();

    
    for(int i=0; i<memberName.size(); i++){
        cout << "memberName[i]: " << memberName[i] << endl;
        auto [pvalue, error_0] = f0_dbPtr->get(memberName[i]);
        if(error_0 != 0){  
            return false;
        }

        AccountPair acc_pair;
        acc_pair.deserialize(*pvalue);

        memberID.push_back(acc_pair.accountID);
        sleep(1);
    }
    return true;
}

//账户注册
void UserModelServer::UserRegisterRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: UserRegisterRest ======"<< endl;

    auto info = request.body();
    cout << info << endl;
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

    cout<<"result:"<<result<<endl;
    response.send(Http::Code::Ok, result); //point
    cout << "====== end UserModelServer function: UserRegisterRest ======"<< endl;
}


string UserModelServer::UserRegister(Account &person){
    cout << "enter UserRegister ======"<< endl;

    //std::string person_key = person.accountID;
    //检查是否存在key，不存在，则进行下面代码开始注册，存在则返回注册失败
    
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();
    
    auto [pvalue, error_0] = f0_dbPtr->get(person.accountName);
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
    AccountPair acc_pair(person.accountName, person.accountID);
    string pari_key = person.accountName;
    string pair_value = acc_pair.serialize();
    
    int f1_flag = f0_dbPtr->set(pari_key, pair_value);
    if (f1_flag != 0){
        cout << "Registration fail: DB[account_map_id] write fail;" << endl;
        return "Registration fail";
    }

    //sc_accounut_pool取出并调整,   sc_account_info写入
    bool buildmap = BuildAccountMapping(person.accountID);
    if(!buildmap){
        cout << "buildmap fail" << endl;
        return "Registration fail";
    }

    //写入account_info表
    std::string person_value = person.serialize();  //json

    std::cout<<person_key<<endl;
    std::cout<<person_value<<endl;
   
    // std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
    //     hvs::CouchbaseDatastore("account_info"));
    // f0_dbPtr->init();
    
    int flag = f0_dbPtr->set(person_key, person_value);
    if (flag != 0){
        cout << "Registration fail: DB[account_info] write fail;" << endl;
        return "Registration fail";
    }
    else{
        string result_1 = "Dear:" + person.accountName + ", registration success";
        return result_1;
    }    
    //string result = "Dear user: " + person.HVSAccountName + ", registration success. Please log in with your username and password";
    //return result;
}



//账户登录
//登录成功，返回：token   失败：-1 ，成功：uuid；
void UserModelServer::UserLoginRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: UserLoginRest ======"<< endl;
    //=====
    printCookies(request);
    //=====
    auto info = request.body(); 
    cout << info << endl;   //账户名，密码
    
    //解析账户名密码
    AccountPass acc_pass;
    acc_pass.deserialize(info);

    string userID;
    bool is_success = UserLogin(acc_pass.accountName, acc_pass.Password, userID);

    if (is_success){
        //md5
        string origin_str = acc_pass.accountName + acc_pass.Password; //再加上url以及时间等等信息
        string mtoken = md5(origin_str);
        std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
            hvs::CouchbaseDatastore("account_info"));
        f0_dbPtr->init();
        string value ="1";
        int login_flag = f0_dbPtr->set(mtoken, value);
        if (login_flag != 0){
            // response.send(Http::Code::Ok, "login fail!,token set fail");
            response.send(Http::Code::Ok, "-1");
            
        }
        else{
            response.cookies().add(Http::Cookie("token", mtoken));
            response.send(Http::Code::Ok, userID); //point
        }
    }
    else{
        // response.send(Http::Code::Unauthorized, "login fail!");
         response.send(Http::Code::Ok, "-1");

    }

    //auto pmtoken = response.headers().get("Token");
    //cout << "pmtoken: " << pmtoken <<endl;
    //cout << "*pmtoken: " << *pmtoken <<endl;
   

    cout<<"====== end UserModelServer function: UserLoginRest ======"<<endl;    
}

bool UserModelServer::UserLogin(std::string account, std::string pass, std::string &userID){
    cout << "enter UserLogin"<< endl;
    //AccountPair中实现新类，只存账户名，和id，这两个信息
    //查询账户名对应的id，作为数据库查询的key

    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));

    f0_dbPtr->init();

    //获取account对应的id    [若数据库没有此key，则返回登录失败的代码]
    auto [pvalue, error_0] = f0_dbPtr->get(account);
    if(error_0){
        cout << "DB[account_map_id]: No such account" <<endl;
        return false;
    }
    AccountPair login_acc_pair;
    login_acc_pair.deserialize(*pvalue);

    //使用获取的id查询数据库中密码，并比较
    string key = login_acc_pair.accountID;
    //string path = "HVSPassword";

    // std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
    //     hvs::CouchbaseDatastore("account_info"));
    
    // f0_dbPtr->init();

    //[若数据库没有此key，则返回登录失败的代码]
    //auto [pPass, error_1] = f0_dbPtr->get(key, path);   *pPass输出带引号
//tmp
    auto [pvalue_2, error_2] = f0_dbPtr->get(key);
    if(error_2){
        cout << "DB[account_info]: No such account" <<endl;
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
        cout<<"pass = " << pass <<endl;
        cout <<"tmp.Password = " << tmp.Password << endl;
        string result = "login fail";
        return false;
    }

}


//账户信息查询
//返回33权限，-1失败，正常值；
void UserModelServer::getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: getUserinfoRest ======"<< endl;

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
    
    cout << "====== end UserModelServer function: getUserinfoRest ======"<< endl;
}

string UserModelServer::getUserinfo(string uuid, bool &is_get_success){
    cout << "enter getUserinfo"<< endl;
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

    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();

    //判断key是否存在
    auto [pvalue, error] = f0_dbPtr->get(key);
    if (error){
        is_get_success = false;
        return "SearchFail";
    }

    cout<<"pvalue:"<< *pvalue <<endl;
    return *pvalue;

}


// 返回33权限，应有结果；
void UserModelServer::modifyUserinfoRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: modifyUserinfoRest ======"<< endl;

    // bool valid = auth_token(request);
    // if (!valid){
    //     response.send(Http::Code::Unauthorized, "Verification failed, access denied");
    //     return;
    // }

    auto info = request.body();
    cout << info << endl;

    Account person;
    person.deserialize(info);  

    string result = modifyUserinfo(person);

    cout<<"result:"<<result<<endl;
    response.send(Http::Code::Ok, result); //point

    cout << "====== end UserModelServer function: modifyUserinfoRest ======"<< endl;
}

string UserModelServer::modifyUserinfo(Account &person){
    cout << "enter modifyUserinfo"<< endl;

    //更新account_info表
    string person_key = person.accountID;
    string person_value = person.serialize();  //json

    cout << person_key << endl;
    cout << person_value << endl;
   
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info")); 
    f0_dbPtr->init();
    
    int flag = f0_dbPtr->set(person_key, person_value);
    if (flag != 0){
        cout << "Modify fail: DB[account_info] update fail" << endl;
        return "Modify fail";
    }
    else{
        return "Modify success";
    }    

}

//返回：33权限，字符串-1失败, 正常值；
void UserModelServer::exitUserAccountRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: exitUserAccountRest ======"<< endl;

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
        //std::cout << c.name << " = " << c.value << std::endl;
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
    
    cout << "====== end UserModelServer function: exitUserAccountRest ======"<< endl;
}


string UserModelServer::exitUserAccount(std::string mtoken , bool &is_exit_success){
    cout << "enter exitUserAccount"<< endl;
    
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));//token
    f0_dbPtr->init();

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
    cout << "====== start UserModelServer function: cancellationUserAccountRest ======"<< endl;

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
    
    cout << "====== end UserModelServer function: cancellationUserAccountRest ======"<< endl;
}

string UserModelServer::cancellationUserAccount(string uuid, bool is_cancel_success){
    cout << "enter cancellationUserAccount"<< endl;

    //获取用户信息
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();

    auto [pvalue, error] = f0_dbPtr->get(uuid);
    if(error){
        is_cancel_success = false;
        cout << "access to db[account_info] fail" << endl;
        return "-1";
    }
    Account hvsperson;
    hvsperson.deserialize(*pvalue);

    //TODO判断区域是否注销完毕sy
    bool is_district_cancel = true;
    if(!is_district_cancel){
        is_cancel_success = false;
        cout << "User cancellation fail, your Zone is not cancal success" << endl;
        return "-2";
    }

    //释放映射的账户，删除sc_account_info中以uuid为key的内容
    bool is_remove_acc = RemoveAccountMapping(uuid);
    if(!is_remove_acc){
        is_cancel_success = false;
        cout << "Account map remove fail" << endl;
        return "-1";
    }

    std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("sc_account_info"));  
    f1_dbPtr->init();
     
    int flag = f1_dbPtr->remove(uuid);
    if(flag){
        is_cancel_success = false;
        cout << "Remove account fail" << endl;
        return "-1";
    }

    //调用账户退出子函数,删除token，但是不删也没事

    
    //删除account_map_id
    int acc_map_id_flag = f0_dbPtr->remove(hvsperson.accountName);
    if(acc_map_id_flag){
        is_cancel_success = false;
        cout << "when db[account_map_id] remove, fail" << endl;
        return "-1";
    }

    //删除account_info
    int acc_info_flag = f0_dbPtr->remove(uuid);
    if(acc_info_flag){
        is_cancel_success = false;
        cout << "when db[account_map_id] remove, fail" << endl;
        return "-1";
    }

    is_cancel_success = true;
    return "User cancellation success.";
}

bool UserModelServer::RemoveAccountMapping(string accountID){

    std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("sc_account_info"));  
    f1_dbPtr->init();

    auto [pvalue_scuser, error] = f1_dbPtr->get(accountID);  //sc_account_info  key:uuid
    if(error){
        cout << "get sc_account_info fail."<< endl;
        return false;
    }

    SCAccount person;
    person.deserialize(*pvalue_scuser);
    
    //删除受hostCenterName影响，因为一下删除5个地点，因此这块不用改
    bool beijing = SubRemoveAccountMapping(person, "Beijing", f1_dbPtr);  //sc_account_pool  key:Beijing,Shanghai...
    bool shanghai = SubRemoveAccountMapping(person, "Shanghai", f1_dbPtr);
    bool guangzhou = SubRemoveAccountMapping(person, "Guangzhou", f1_dbPtr);
    bool changsha = SubRemoveAccountMapping(person, "Changsha", f1_dbPtr);
    bool jinan = SubRemoveAccountMapping(person, "Jinan", f1_dbPtr);
    
    if(!(beijing && shanghai && guangzhou && changsha && jinan)){
       return false;
    }
    else{
       return true;
    }
}

//删除一个用户指定地区(location)的［所有］账户映射
bool UserModelServer::SubRemoveAccountMapping(SCAccount &person, string location, shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr){
    cout << "location " << location << endl;
    auto [pvalue_location, error] = f1_dbPtr->get(location);    //sc_account_pool  key:Beijing,Shanghai...
    if (error){
        cout << "get "<< location << " err"<<endl;
        return false;
    }
    //cout << *pvalue_location <<endl;

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
        cout<< "remove map fail: DB[sc_account_pool] update fail"<< endl;
        return false;
    }
    else{
        cout<< "remove map success: DB[sc_account_pool] update success" << endl;
    }    
    
    string value2 = person.serialize();
    int flag2 = f1_dbPtr->set(person.accountID, value2);   //sc_account_info
    if (flag2 != 0){
        cout<< "remove map fail: DB[sc_account_info] update fail"<<endl;
        return false;
    }
    else{
        cout<< "removeAccountMapping success: DB[sc_account_info] update success" << endl;
        return true;
    }    

}

//register，first, build account mapping 
bool UserModelServer::BuildAccountMapping(string accountID){
    
    //get sc_account_pool
    
    SCAccount person(accountID);

    std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("sc_account_info"));  
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
bool UserModelServer::SubBuildAccountMapping(SCAccount &person, string location, shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr){
    
    cout << "location " << location << endl;
    auto [pvalue_location, error] = f1_dbPtr->get(location);
    if (error){
        cout << "get "<< location << " err"<<endl;
        return false;
    }
    
    cout << *pvalue_location <<endl;

    AccountSCPool somewhere;
    somewhere.deserialize(*pvalue_location);
    if(somewhere.unuse_account.empty()){
        cout << location <<" .unuse_account is empty , no local account can be used to build accountmap. Fail. " << endl;
        return false;
    }
    map<string, string>::iterator iter;
    iter = somewhere.unuse_account.begin();  //从未用账户池中取出一个账户
    //add to person

    if(!location.compare("Beijing")){
        person.Beijing_account[iter->first] = iter->second; //account   password
        cout << "confirm Beijing" <<endl;
    }
    else if(!location.compare("Shanghai")){
        person.Shanghai_account[iter->first] = iter->second;   
        cout << "confirm Shanghai" <<endl;
    }
    else if(!location.compare("Guangzhou")){
        person.Guangzhou_account[iter->first] = iter->second;  
        cout << "confirm Guangzhou" <<endl;
    }
    else if(!location.compare("Changsha")){
        person.Changsha_account[iter->first] = iter->second; 
        cout << "confirm Changsha" <<endl;
    }
    else if(!location.compare("Jinan")){
        person.Jinan_account[iter->first] = iter->second;   
        cout << "confirm Jinan" <<endl;
    }
    //add to use
    somewhere.use_account[iter->first] = iter->second;
    //del from unuse
    somewhere.unuse_account.erase(iter->first);

    //更新两个数据库表sc_account_pool  sc_account_info
    string value = somewhere.serialize();   //sc_account_pool
    int flag = f1_dbPtr->set(location, value);
    if (flag != 0){
        cout<< "map fail: DB[sc_account_pool] update fail"<< endl;
        return false;
    }
    else{
        cout<< "map success: DB[sc_account_pool] update success" << endl;
    }    

    //cout << person.Changsha_account.size() <<endl;
    string person_value = person.serialize();

    int flag2 = f1_dbPtr->set(person.accountID, person_value); //sc_account_info
    if (flag2 != 0){
        cout<< "map fail: DB[sc_account_info] update fail"<<endl;
        return false;
    }
    else{
        cout<< "BuildAccountMapping success: DB[sc_account_info] update success" << endl;
        return true;
    }    
    
}


//账户映射接口，返回指定超算本地账户的账户名，密码
string UserModelServer::getLocalAccountinfo(string ownerID, string hostCenterName){
    std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("sc_account_info"));
    f1_dbPtr->init();

    //TODO:这块要try一下 否则不存在此ownerid，查寻不到此用户
    auto [pvalue, error] = f1_dbPtr->get(ownerID);
    if(error){
        cout << "fail" << endl;
        return "fail";
    }

    
    SCAccount person;
    person.deserialize(*pvalue);
    
    map<string, string>::iterator iter;

    if(hostCenterName.compare(supercomputing_A) == 0){ //Beijing
        if(person.Beijing_account.empty()){
            //调用接口，建立到北京的账户映射；
                //第二个参数是账户池的key
            bool tmp_flag = SubBuildAccountMapping(person, "Beijing", f1_dbPtr); //这块"Beijing"是做key，不能随意改
            if (!tmp_flag){
                return "fail";
            }

            auto [pvalue_1, error_1] = f1_dbPtr->get(ownerID);
            if(error_1){
                cout << "fail" << endl;
                return "fail";//并返回结果，如果建立失败
            }
            SCAccount person_1;
            person_1.deserialize(*pvalue_1);
            iter = person_1.Beijing_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }
        else{
            iter = person.Beijing_account.begin();   //这块person是在最前面定义；
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }
    }
    else if(hostCenterName.compare(supercomputing_B) == 0){ //Shanghai
        //类似北京
        if(person.Shanghai_account.empty()){
            //调用接口，建立到上海的账户映射；
                //第二个参数是账户池的key
            bool tmp_flag = SubBuildAccountMapping(person, "Shanghai", f1_dbPtr); //这块"Shanghai"是做key，不能随意改
            if (!tmp_flag){
                return "fail";
            }

            auto [pvalue_1, error_1] = f1_dbPtr->get(ownerID);
            if(error_1){
                cout << "fail" << endl;
                return "fail";//并返回结果，如果建立失败
            }
            SCAccount person_1;
            person_1.deserialize(*pvalue_1);
            iter = person_1.Shanghai_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }
        else{
            iter = person.Shanghai_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }// if
    }
    else if(hostCenterName.compare(supercomputing_C) == 0){//Guangzhou
        //类似北京
        if(person.Guangzhou_account.empty()){
            //调用接口，建立到上海的账户映射；
                //第二个参数是账户池的key
            bool tmp_flag = SubBuildAccountMapping(person, "Guangzhou", f1_dbPtr); //这块"Guangzhou"是做key，不能随意改
            if (!tmp_flag){
                return "fail";
            }

            auto [pvalue_1, error_1] = f1_dbPtr->get(ownerID);
            if(error_1){
                cout << "fail" << endl;
                return "fail";//并返回结果，如果建立失败
            }
            SCAccount person_1;
            person_1.deserialize(*pvalue_1);
            iter = person_1.Guangzhou_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }
        else{
            iter = person.Guangzhou_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }// if
    }
    else if(hostCenterName.compare(supercomputing_D) == 0){//Changsha
        //类似北京
        if(person.Changsha_account.empty()){
            //调用接口，建立到上海的账户映射；
                //第二个参数是账户池的key
            bool tmp_flag = SubBuildAccountMapping(person, "Changsha", f1_dbPtr); //这块"Changsha"是做key，不能随意改
            if (!tmp_flag){
                return "fail";
            }

            auto [pvalue_1, error_1] = f1_dbPtr->get(ownerID);
            if(error_1){
                cout << "fail" << endl;
                return "fail";//并返回结果，如果建立失败
            }
            SCAccount person_1;
            person_1.deserialize(*pvalue_1);
            iter = person_1.Changsha_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }
        else{
            iter = person.Changsha_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }// if
    }
    else if(hostCenterName.compare(supercomputing_E) == 0){//Jinan
        //类似北京
        if(person.Jinan_account.empty()){
            //调用接口，建立到上海的账户映射；
                //第二个参数是账户池的key
            bool tmp_flag = SubBuildAccountMapping(person, "Jinan", f1_dbPtr); //这块"Jinan"是做key，不能随意改
            if (!tmp_flag){
                return "fail";
            }
         
            auto [pvalue_1, error_1] = f1_dbPtr->get(ownerID);
            if(error_1){
                cout << "fail" << endl;
                return "fail";//并返回结果，如果建立失败
            }
            
            SCAccount person_1;
            person_1.deserialize(*pvalue_1); cout << "333" << endl;
            iter = person_1.Jinan_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }
        else{
            iter = person.Jinan_account.begin();
            if(!existlocalaccount(iter->first)) return "fail";
            LocalAccountPair localpair(iter->first, iter->second); //对应的本地账户名，密码
            return localpair.serialize();
        }// if
    }

}


/* 在某个地方先调用写，完成sc_account_pool表
    bool tmp = addSCaccount();
    if(tmp){
        cout << "**** addSCaccount finish******" <<endl;
    }
*/
//这个里面确定了账户池的key，如果要改，可以在这改
bool UserModelServer::addSCaccount(){
    std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("sc_account_info")); 
    f1_dbPtr->init();

    //Beijing
    AccountSCPool Beijing;
    Beijing.unuse_account["test1"] = "123456";
    Beijing.unuse_account["test2"] = "123456";
    Beijing.unuse_account["test3"] = "123456";
    Beijing.unuse_account["test4"] = "123456";
    Beijing.unuse_account["test5"] = "123456";
    Beijing.unuse_account["test6"] = "123456";
    Beijing.unuse_account["test7"] = "123456";
    Beijing.unuse_account["test8"] = "123456";
    Beijing.unuse_account["test9"] = "123456";
    Beijing.unuse_account["test10"] = "123456";
    Beijing.unuse_account["test11"] = "123456";
    Beijing.unuse_account["test12"] = "123456";
    Beijing.unuse_account["test13"] = "123456";


    Beijing.use_account["user1"] = "555555";
    Beijing.use_account["user2"] = "555555";

    string key1 = "Beijing";
    string value1 = Beijing.serialize();

    int flag1 = f1_dbPtr->set(key1, value1);
    if (flag1 != 0){
        cout<< "set fail: DB[sc_account_pool] update fail"<<endl;
        return false;
    }
    else{
        cout<<"add Beijing success"<<endl;
    }  


    //Shanghai
    AccountSCPool Shanghai;
    Shanghai.unuse_account["test1"] = "123456";
    Shanghai.unuse_account["test2"] = "123456";
    Shanghai.unuse_account["test3"] = "123456";
    Shanghai.unuse_account["test4"] = "123456";
    Shanghai.unuse_account["test5"] = "123456";
    Shanghai.unuse_account["test6"] = "123456";
    Shanghai.unuse_account["test7"] = "123456";
    Shanghai.unuse_account["test8"] = "123456";
    Shanghai.unuse_account["test9"] = "123456";
    Shanghai.unuse_account["test10"] = "123456";
    Shanghai.unuse_account["test11"] = "123456";
    Shanghai.unuse_account["test12"] = "123456";
    Shanghai.unuse_account["test13"] = "123456";


    Shanghai.use_account["user1"] = "555555";
    Shanghai.use_account["user2"] = "555555";

    string key2 = "Shanghai";
    string value2 = Shanghai.serialize();

    int flag2 = f1_dbPtr->set(key2, value2);
    if (flag2 != 0){
        cout<< "set fail: DB[sc_account_pool] update fail"<<endl;
        return false;
    }
    else{
        cout<<"add Shanghai success"<<endl;
    }  

    //Guangzhou   
    AccountSCPool Guangzhou;
    Guangzhou.unuse_account["test1"] = "123456";
    Guangzhou.unuse_account["test2"] = "123456";
    Guangzhou.unuse_account["test3"] = "123456";
    Guangzhou.unuse_account["test4"] = "123456";
    Guangzhou.unuse_account["test5"] = "123456";
    Guangzhou.unuse_account["test6"] = "123456";
    Guangzhou.unuse_account["test7"] = "123456";
    Guangzhou.unuse_account["test8"] = "123456";
    Guangzhou.unuse_account["test9"] = "123456";
    Guangzhou.unuse_account["test10"] = "123456";
    Guangzhou.unuse_account["test11"] = "123456";
    Guangzhou.unuse_account["test12"] = "123456";
    Guangzhou.unuse_account["test13"] = "123456";


    Guangzhou.use_account["user1"] = "555555";
    Guangzhou.use_account["user2"] = "555555";

    string key3 = "Guangzhou";
    string value3 = Guangzhou.serialize();

    int flag3 = f1_dbPtr->set(key3, value3);
    if (flag3 != 0){
        cout<< "set fail: DB[sc_account_pool] update fail"<<endl;
        return false;
    }
    else{
        cout<<"add Guangzhou success"<<endl;
    }  


    //Changsha 
    AccountSCPool Changsha;
    Changsha.unuse_account["test1"] = "123456";
    Changsha.unuse_account["test2"] = "123456";
    Changsha.unuse_account["test3"] = "123456";
    Changsha.unuse_account["test4"] = "123456";
    Changsha.unuse_account["test5"] = "123456";
    Changsha.unuse_account["test6"] = "123456";
    Changsha.unuse_account["test7"] = "123456";
    Changsha.unuse_account["test8"] = "123456";
    Changsha.unuse_account["test9"] = "123456";
    Changsha.unuse_account["test10"] = "123456";
    Changsha.unuse_account["test11"] = "123456";
    Changsha.unuse_account["test12"] = "123456";
    Changsha.unuse_account["test13"] = "123456";


    Changsha.use_account["user1"] = "555555";
    Changsha.use_account["user2"] = "555555";

    string key4 = "Changsha";
    string value4 = Changsha.serialize();

    int flag4 = f1_dbPtr->set(key4, value4);
    if (flag4 != 0){
        cout<< "set fail: DB[sc_account_pool] update fail"<<endl;
        return false;
    }
    else{
        cout<<"add Changsha success"<<endl;
    }  

    //Jinan
    AccountSCPool Jinan;
    Jinan.unuse_account["test1"] = "123456";
    Jinan.unuse_account["test2"] = "123456";
    Jinan.unuse_account["test3"] = "123456";
    Jinan.unuse_account["test4"] = "123456";
    Jinan.unuse_account["test5"] = "123456";
    Jinan.unuse_account["test6"] = "123456";
    Jinan.unuse_account["test7"] = "123456";
    Jinan.unuse_account["test8"] = "123456";
    Jinan.unuse_account["test9"] = "123456";
    Jinan.unuse_account["test10"] = "123456";
    Jinan.unuse_account["test11"] = "123456";
    Jinan.unuse_account["test12"] = "123456";
    Jinan.unuse_account["test13"] = "123456";


    Jinan.use_account["user1"] = "555555";
    Jinan.use_account["user2"] = "555555";

    string key5 = "Jinan";
    string value5 = Jinan.serialize();

    int flag5 = f1_dbPtr->set(key5, value5);
    if (flag5 != 0){
        cout<< "set fail: DB[sc_account_pool] update fail"<<endl;
        return false;
    }
    else{
        cout<<"add Jinan success"<<endl;
    }  

    return true;
}



//existlocalaccount
//输入一个账户，检验本地是否存在
bool UserModelServer::existlocalaccount(string valid){
    int bufsize = 80000;
    FILE *read_fp;
    char buffer[bufsize + 1];
    int chars_read;
    memset(buffer, '\0', sizeof(buffer));
    read_fp = popen ("cat /etc/passwd|grep -v nologin|grep -v halt|grep -v shutdown|awk -F\":\" '{ print $1}'|more", "r");
    if (read_fp != NULL)
    {
        chars_read = fread(buffer, sizeof(char), bufsize, read_fp);
        if (chars_read > 0)
        {
            // printf("Output was: \n%s", buffer);
            std::vector<std::string> my;
            int i=0;
            int front = 0;
            for(i=0; i < strlen(buffer); i++){
                if (buffer[i] == '\n'){
                    string tmp_str;
                    while(front != i){
                        tmp_str += buffer[front];
                        front++;
                    }
                    front++;
                    // cout << "tmp_str: " << tmp_str << endl;
                    my.push_back(tmp_str);
                }
                //printf("aaaaaa %c ",buffer[i]);
            }
            
            for(int j=0; j<my.size(); j++){
                if(valid==my[j]){
                    cout << "my:" << my[j] << endl;
                    cout << "返回成功" << endl;
                    return true;
                }
            }
            cout << "返回失败" << endl;
            return false;
        }
        else
        {
            cout << "返回失败" << endl;
            return false;

        }
        pclose(read_fp);
        
    }
    else{
        cout << "返回失败" <<endl;
        return false;
    }
}

//MD5

string md5(string strPlain)
{
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
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}


bool auth_token(const Rest::Request& request){
    std::cout << "function: auth_token"<< std::endl;
    printCookies(request);
    
    std::string name;
    std::string mtoken;
    auto cookies = request.cookies();
    for (const auto& c: cookies) {
        //std::cout << c.name << " = " << c.value << std::endl;
        name = c.name;
        mtoken = c.value;
    }
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();

   
    auto [vp, err] = f0_dbPtr->get(mtoken);
    if(err){ //!=0
        return false;//验证失败
    }
    return true; //验证成功
}


}// namespace hvs