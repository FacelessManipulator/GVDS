/*
Author:Liubingqi
date:2019.03.21

g++ -c UserModelServer.cc
g++ -c hvsrest.cc -lpistache  -std=c++11
g++ -o user UserModelServer.o hvsrest.o -lpistache -std=c++11

./user 5
*/
#ifndef _RESTSERVER_H_
#define _RESTSERVER_H_


#include <algorithm>

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include "usermodel/UserModelServer.h"
#include "common/Thread.h"

using namespace std;
using namespace Pistache;


void printCookies(const Http::Request& req);
void CtrlStop(int signo);

namespace Generic {
void handleReady(const Rest::Request&, Http::ResponseWriter response);
}






class RestServer : public Thread {

void *entry() override;

public:
    RestServer(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2);       //[原始函数]

    void start();      //启动rest服务 [原始函数]
    void shutdown();   //终止rest服务 [原始函数]

    void begin();  ///< 启动rest线程     [后加函数]
    void end();  /// < 终止rest线程，会join直到entry函数完成最后一个flush的循环   [后加函数]


private:
    void setupRoutes();       //[原始函数]

    //void doRecordMetric(const Rest::Request& request, Http::ResponseWriter response)
    //void doGetMetric(const Rest::Request& request, Http::ResponseWriter response) 

    void doAuth(const Rest::Request& request, Http::ResponseWriter response);

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};
extern RestServer* init_rest();     //[后加函数]
extern void stop_rest(RestServer* rest);     //[后加函数]


//int main(int argc, char *argv[]) 

#endif