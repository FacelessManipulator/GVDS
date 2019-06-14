#include <iostream>
#include <vector>
#include <stdio.h>
#include "common/JsonSerializer.h"
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
    
}

void Mconf::addCenterRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "start: addCenterRest" << endl;
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
    
    cout << "end: addCenterRest" << endl;
}


// 首次添加，没有表需要注释部分代码，先set，否则无法get
int Mconf::addCenter(FECenterInfo &FEcenter){
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();
    
    int first_access_flag = 0;
// /* 
    auto [pvalue, error_0] = f0_dbPtr->get(key);   //key 在构造函数里
    if(error_0){
        cout << "get DB center_information fail, no key" <<endl;
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
    }
    mycenter.centerIP[FEcenter.centerID] = FEcenter.centerIP;
    mycenter.centerPort[FEcenter.centerID] = FEcenter.centerPort;
    mycenter.centerName[FEcenter.centerID] = FEcenter.centerName;

    string value = mycenter.serialize();

    int flag = f0_dbPtr->set(key, value);
    if (flag != 0){
        cout << "modify fail;" << endl;
        return -1;
    }

    return 0;
}

void Mconf::searchCenterRest(const Rest::Request& request, Http::ResponseWriter response){
    //get
    cout << "start: addCenterRest" << endl;
    string data = searchCenter();
    if(data == "fail"){
        response.send(Http::Code::Not_Found, "fail"); 
    }
    response.send(Http::Code::Ok, data); 
}

string Mconf::searchCenter(){
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();

    cout << "key: " << key <<endl;
    auto [pvalue, error_0] = f0_dbPtr->get(key);   //key 在构造函数里
    if(error_0){
        cout << "get DB center_information fail" <<endl;
        return "fail";
    }

    return *pvalue;
}





}//namespace