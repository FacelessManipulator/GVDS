/*
Author:Liubingqi
date:2019.03.21

g++ -c UserModelServer.cc
g++ -c hvsrest.cc -lpistache  -std=c++11
g++ -o user UserModelServer.o hvsrest.o -lpistache -std=c++11

./user 5
*/


#include <iostream>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"

#include "usermodel/UserModelServer.h"

using namespace std;
//using namespace hvs;
//object 
UserModelServer* UserModelServer::instance = nullptr;

void UserModelServer::getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response){
    auto uuid = request.param(":name").as<std::string>();
    
    // include your functin
    string data = getUserinfo(uuid);

    response.send(Http::Code::Ok, data); //point
}

string UserModelServer::getUserinfo(string uuid){
    cout << "UserModelServer function: getUserinfo"<< endl;
/*
    map<string, string> usermap;

    usermap["username"] = name;
    usermap.insert(pair<string, string>("email", "liubingqi112@163.com"));

    string nameValue = usermap["username"];
    string emailValue = usermap["email"];
    //for (map<string, string>::const_iterator iter = usermap.begin( ); iter != usermap.end( ); ++iter)


    //string data = "{\"name\":\"" + nameValue + "\", \"email\":\"" + emailValue + "\"}" ;
*/
    string key = uuid;

    auto dbPtr = hvs::DatastoreFactory::create_datastore(
      "AccountInfo", hvs::DatastoreType::couchbase);
    dbPtr->init();

    //需要判断key是否存在，不存在或者其他情况，则查询失败
    auto [pvalue, error] = dbPtr->get(key);


    return *pvalue;
}



void UserModelServer::UserRegisterRest(const Rest::Request& request, Http::ResponseWriter response){
    auto info = request.body();
    cout << info << endl;

    Account person;

    person.accountName = "lbq";
    person.accountID = "123";
    person.Password = "123456";
    person.accountEmail = "XXXXXX@163.com";
    person.Department = "Beihang";
    person.accountAddress = "xueyuanlu";
    person.accountPhone = "15012349876";
    
    string result = UserRegister(person);
    
    response.send(Http::Code::Ok, result); //point

}


string UserModelServer::UserRegister(Account person){
    cout << "UserModelServer function: UserRegister"<< endl;

    std::string person_key = person.accountID;
    std::string person_value = person.serialize();  //json

    auto dbPtr = hvs::DatastoreFactory::create_datastore(
      "AccountInfo", hvs::DatastoreType::couchbase);

    dbPtr->init();
    
    int flag = dbPtr->set(person_key, person_value);
    if (flag != 0){
        string result = "Registration fail";
        return result;
    }
    else{
        string result = "Dear:" + person.accountName + ", registration success";
        return result;
    }


  

   
    
    //string result = "Dear user: " + person.HVSAccountName + ", registration success. Please log in with your username and password";

    //return result;
}