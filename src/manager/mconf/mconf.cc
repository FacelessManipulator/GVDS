#include <iostream>
#include <vector>
#include <stdio.h>
#include "common/JsonSerializer.h"
#include "common/json.h"
#include "context.h"
#include "datastore/datastore.h"


#include "manager/mconf/mconf.h"



using namespace std;
namespace hvs{
using namespace Pistache::Rest;
using namespace Pistache::Http;

void Mconf::start() {}
void Mconf::stop() {}

void Mconf::router(Router& router){
    Routes::Post(router, "/mconf/addCenter", Routes::bind(&Mconf::addCenterRest, this));
    Routes::Get(router, "/mconf/searchCenter", Routes::bind(&Mconf::searchCenterRest, this));
    Routes::Post(router, "/mconf/deleteCenter", Routes::bind(&Mconf::deleteCenterRest, this));  
}

void Mconf::addCenterRest(const Rest::Request& request, Http::ResponseWriter response){
    //cout << "start: addCenterRest" << endl;
    auto info = request.body();
    cout << info << endl;

    FECenterInfo FEcenter;
    FEcenter.deserialize(info);

    int flag = addCenter(FEcenter);
    if (flag == -1){
        response.send(Http::Code::Not_Found, "fail"); 
    }
    else{
        response.send(Http::Code::Ok, "success"); 
    }
    
    // cout << "end: addCenterRest" << endl;
}


// 首次添加，没有表需要注释部分代码，先set，否则无法get
int Mconf::addCenter(FECenterInfo &FEcenter){
    auto f0_dbPtr = hvs::DatastoreFactory::create_datastore(bucket_account_info, hvs::couchbase, true);
    
    int first_access_flag = 0;
// /* 
    auto [pvalue, error_0] = f0_dbPtr->get(key);   //key 在构造函数里
    if(error_0){
        cout << "get DB center_information fail, no key" << endl;
        cout << "waiting, create key success" <<endl;
        first_access_flag = 1; //首次添加
    }
// */
    CenterInfo mycenter;
// /*  
    if (first_access_flag == 0)
        mycenter.deserialize(*pvalue);   
    

// */
    //检查是否存在相同的id，存在则认为是修改
    bool tmp = false;
    for(int i=0; i<mycenter.centerID.size(); i++){
        if (FEcenter.centerID == mycenter.centerID.at(i))
            tmp = true;
    }

    if(tmp == false){
        mycenter.centerID.push_back(FEcenter.centerID);
        std::map<std::string, std::map<std::string, std::string>> users;
        users["use_account"] = std::map<std::string, std::string>();
        users["unuse_account"] = std::map<std::string, std::string>();
        char username[32];
        for (int i = 1; i <= 100; i++) {
            snprintf(username, 32, "test%d", i);
            users["unuse_account"][username] = "";
        }
        auto users_str = hvs::json_encode(users);
        f0_dbPtr->set(FEcenter.centerName, users_str);
    }
    mycenter.centerIP[FEcenter.centerID] = FEcenter.centerIP;
    mycenter.centerPort[FEcenter.centerID] = FEcenter.centerPort;
    mycenter.centerName[FEcenter.centerID] = FEcenter.centerName;

    cout <<"FEcenter.centerName: " << FEcenter.centerName << endl;
    cout <<"key: " << key << endl;
  
    

    string value = mycenter.serialize();

    cout <<"value: " << value << endl;

    int flag = f0_dbPtr->set(key, value);
    if (flag != 0){
        cout << "modify fail;" << endl;
        return -1;
    }

    return 0;
}

void Mconf::searchCenterRest(const Rest::Request& request, Http::ResponseWriter response){
    //get
    // cout << "start: addCenterRest" << endl;
    string data = searchCenter();
    if(data == "fail"){
        response.send(Http::Code::Not_Found, "fail"); 
    }
    else{
        response.send(Http::Code::Ok, data); 
    }
    
}

string Mconf::searchCenter(){
    auto f0_dbPtr = hvs::DatastoreFactory::create_datastore(bucket_account_info, hvs::couchbase, true);

    // cout << "key: " << key <<endl;
    auto [pvalue, error_0] = f0_dbPtr->get(key);   //key 在构造函数里
    if(error_0){
        cout << "get DB center_information fail" <<endl;
        return "fail";
    }

    return *pvalue;
}

//删除
void Mconf::deleteCenterRest(const Rest::Request& request, Http::ResponseWriter response){
    //get
    // cout << "start: deleteCenter" << endl;

    auto info = request.body();
    cout << info << endl; //centerID
    string centerID = info;

    string data = deleteCenter(centerID);
    if(data == "fail"){
        response.send(Http::Code::Not_Found, "fail"); 
    }
    else{
        response.send(Http::Code::Ok, data); 
    }
    
}


string Mconf::deleteCenter(string centerID){
    cout << "start: deleteCenter" << endl;

    auto f0_dbPtr = hvs::DatastoreFactory::create_datastore(bucket_account_info, hvs::couchbase, true);

    // cout << "key: " << key <<endl;
    //1、获取数据
    auto [pvalue, error_0] = f0_dbPtr->get(key);   //key 在构造函数里
    if(error_0){
        cout << "get DB center_information fail" <<endl;
        return "fail";
    }

    CenterInfo mycenter;
    mycenter.deserialize(*pvalue);   

    cout << "start: 2、删除数据" << endl;

    //2、删除数据
    mycenter.centerIP.erase(centerID);
    mycenter.centerPort.erase(centerID);
    mycenter.centerName.erase(centerID);

    cout << "start: 2.1 mycenter.centerID" << endl;


    bool isflag = false;
    vector<string>::iterator iter = mycenter.centerID.begin();
    for(; iter!=mycenter.centerID.end(); iter++){
        if(*iter == centerID){
            cout << "iter: " << *iter << endl;
            mycenter.centerID.erase(iter);
            isflag = true;
            break;
        }
    }
    if(isflag == false){
        return "fail";
    }
    
        

    cout << "start: 3、更新数据库" << endl;

    //3、更新数据库
    string value = mycenter.serialize();

    int flag = f0_dbPtr->set(key, value);
    if (flag != 0){
        cout << "del fail;" << endl;
        return "fail";
    }


    return "success";
}


}//namespace