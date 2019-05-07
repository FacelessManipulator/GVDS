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

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

#include "common/Thread.h"
#include "usermodel/UserModelServer.h"

using namespace std;
using namespace Pistache;

namespace hvs {
class Manager;

class RestServer : public Thread {
  void* entry() override;

 public:
  RestServer(Address addr)
      : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

  void init(size_t thr = 2);  //[原始函数]
  void start();               //启动rest服务 [原始函数]
  void shutdown();            //终止rest服务 [原始函数]
  int getPort() {
    if (httpEndpoint) httpEndpoint->getPort();
  }

 private:

  std::shared_ptr<Http::Endpoint> httpEndpoint;
  Rest::Router router;
  friend class Manager;
};
extern RestServer* init_rest();           //[后加函数]
extern void stop_rest(RestServer* rest);  //[后加函数]
}  // namespace hvs

#endif