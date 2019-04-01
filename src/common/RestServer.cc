#include <signal.h>

#include "common/RestServer.h"


void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

void CtrlStop(int signo){
    exit(0);
}



void Generic::handleReady(const Rest::Request&, Http::ResponseWriter response) {
    response.send(Http::Code::Ok, "1");
}


void RestServer::init(size_t thr) {
        auto opts = Http::Endpoint::options()
            .threads(thr)
            .flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
        setupRoutes();
}


 void RestServer::start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
}

 void RestServer::shutdown() {
        httpEndpoint->shutdown();
}

void RestServer::setupRoutes() {
    using namespace Rest;

    //Routes::Post(router, "/record/:name/:value?", Routes::bind(&RestServer::doRecordMetric, this));
    //Routes::Get(router, "/value/:name", Routes::bind(&RestServer::doGetMetric, this));
    Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
    Routes::Get(router, "/auth", Routes::bind(&RestServer::doAuth, this));

    //add your router here
    Routes::Get(router, "/users/:name", Routes::bind(&UserModelServer::getUserinfoRest, UserModelServer::getInstance()));
}


void RestServer::doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        printCookies(request);
        response.cookies()
            .add(Http::Cookie("lang", "en-US"));
        response.send(Http::Code::Ok);
}

void RestServer::begin(){
    assert(!is_started());
    create("rest");
}

void RestServer::end(){
    if (is_started()) {
        shutdown();     //RestServer类函数 [原始函数]
        join();
    }
}

RestServer* init_rest(){
    Port port(9080);

    int thr = 5;

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    RestServer* stats = new RestServer(addr);    

    stats->init(thr);     //[原始函数]

    stats->begin();       //调用Thread类的create函数, creat函数调用entry函数, 开启线程  [后加函数]   

    return stats;
   
}

void stop_rest(RestServer* rest){
    rest->end();
    delete rest;
}

void *RestServer::entry() {

    signal(SIGINT, CtrlStop); 

    start();        //RestServer类函数, 启动rest服务 [原始函数]      //start 对应 shutdown, begin 对应 end

    return NULL;
}