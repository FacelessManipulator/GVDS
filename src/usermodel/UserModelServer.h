/*
Author:Liubingqi
date:2019.03.21
*/

#ifndef USERMODELSERVER_H
#define USERMODELSERVER_H

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <iostream>
#include <map>

using namespace Pistache;
using namespace std;

class UserModelServer {
public:
    static UserModelServer* getInstance(){
        if (instance == nullptr)
            instance = new UserModelServer();
	    return instance;
    };
 //--------------------------------------------
    //define your function here
    
    void getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response);

    string getUserinfo(string name);
 //--------------------------------------------
private:
    UserModelServer() = default;
    //UserModelServer(const UserModelServer&) = default;
    //UserModelServer& operator=(const UserModelServer&) {};
    ~UserModelServer();

    static UserModelServer* instance;  //single object
};



#endif



