/*
Author:Liubingqi
date:2019.03.21

g++ -c UserModelServer.cc
g++ -c hvsrest.cc -lpistache  -std=c++11
g++ -o user UserModelServer.o hvsrest.o -lpistache -std=c++11

./user 5
*/

#include "UserModelServer.h"


//object 
UserModelServer* UserModelServer::instance = nullptr;

void UserModelServer::getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response){
    auto name = request.param(":name").as<std::string>();
    
    // include your functin
    string data = getUserinfo(name);

    response.send(Http::Code::Ok, data); //point
}

string UserModelServer::getUserinfo(string name){
    cout << "server function: getUserinfo"<< endl;

    map<string, string> usermap;

    usermap["username"] = name;
    usermap.insert(pair<string, string>("email", "liubingqi112@163.com"));


    string nameValue = usermap["username"];
    string emailValue = usermap["email"];
    //for (map<string, string>::const_iterator iter = usermap.begin( ); iter != usermap.end( ); ++iter)

    string data = "{\"name\":\"" + nameValue + "\", \"email\":\"" + emailValue + "\"}" ;


    return data;
}

