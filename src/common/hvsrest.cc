/*
Author:Liubingqi
date:2019.03.21

g++ -c UserModelServer.cc
g++ -c hvsrest.cc -lpistache  -std=c++11
g++ -o user UserModelServer.o hvsrest.o -lpistache -std=c++11

./user 5
*/


#include <algorithm>

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include "UserModelServer.h"


using namespace std;
using namespace Pistache;



void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic {

void handleReady(const Rest::Request&, Http::ResponseWriter response) {
    response.send(Http::Code::Ok, "1");
}

}

class RestServer {
public:
    RestServer(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
            .threads(thr)
            .flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() {
        httpEndpoint->shutdown();
    }

private:
    void setupRoutes() {
        using namespace Rest;

        //Routes::Post(router, "/record/:name/:value?", Routes::bind(&RestServer::doRecordMetric, this));
        //Routes::Get(router, "/value/:name", Routes::bind(&RestServer::doGetMetric, this));
        Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
        Routes::Get(router, "/auth", Routes::bind(&RestServer::doAuth, this));

        //add your router here
        Routes::Get(router, "/users/:name", Routes::bind(&UserModelServer::getUserinfoRest, UserModelServer::getInstance()));
    }

    //void doRecordMetric(const Rest::Request& request, Http::ResponseWriter response)
    //void doGetMetric(const Rest::Request& request, Http::ResponseWriter response) 

    void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        printCookies(request);
        response.cookies()
            .add(Http::Cookie("lang", "en-US"));
        response.send(Http::Code::Ok);
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};



int main(int argc, char *argv[]) {
    Port port(9080);

    int thr = 2;

    if (argc >= 2) {
        port = std::stol(argv[1]);

        if (argc == 3)
            thr = std::stol(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    RestServer stats(addr);

    stats.init(thr);
    stats.start();

    stats.shutdown();
}
