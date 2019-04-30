#include <signal.h>

#include "aggregation/AggregationServer.h"
#include "common/RestServer.h"

void printCookies(const Http::Request& req) {
  auto cookies = req.cookies();
  std::cout << "Cookies: [" << std::endl;
  const std::string indent(4, ' ');
  for (const auto& c : cookies) {
    std::cout << indent << c.name << " = " << c.value << std::endl;
  }
  std::cout << "]" << std::endl;
}

void CtrlStop(int signo) { exit(0); }

bool auth_token(const Rest::Request& request) {
  std::cout << "function: auth_token" << std::endl;
  printCookies(request);

  std::string name;
  std::string mtoken;
  auto cookies = request.cookies();
  for (const auto& c : cookies) {
    // std::cout << c.name << " = " << c.value << std::endl;
    name = c.name;
    mtoken = c.value;
  }
  std::shared_ptr<hvs::CouchbaseDatastore> f2_dbPtr =
      std::make_shared<hvs::CouchbaseDatastore>(
          hvs::CouchbaseDatastore("token_info"));
  f2_dbPtr->init();

  auto [vp, err] = f2_dbPtr->get(mtoken);
  if (err) {       //!=0
    return false;  //验证失败
  }
  return true;  //验证成功
}

void Generic::handleReady(const Rest::Request&, Http::ResponseWriter response) {
  response.send(Http::Code::Ok, "1");
}

namespace hvs {

void RestServer::init(size_t thr) {
  auto opts = Http::Endpoint::options().threads(thr).flags(
      Tcp::Options::InstallSignalHandler);
  httpEndpoint->init(opts);
  setupRoutes();
}

void RestServer::start() {
  assert(!is_started());
  create("rest");
}

void RestServer::shutdown() {
  if (is_started()) {
    httpEndpoint->shutdown();
    join();
  }
}

void RestServer::setupRoutes() {
  using namespace Rest;

  // Routes::Post(router, "/record/:name/:value?",
  // Routes::bind(&RestServer::doRecordMetric, this)); Routes::Get(router,
  // "/value/:name", Routes::bind(&RestServer::doGetMetric, this));
  Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
  Routes::Get(router, "/auth", Routes::bind(&RestServer::doAuth, this));

  // add your router here
  Routes::Get(router, "/users/search/:name",
              Routes::bind(&UserModelServer::getUserinfoRest,
                           UserModelServer::getInstance()));
  Routes::Post(router, "/users/registration",
               Routes::bind(&UserModelServer::UserRegisterRest,
                            UserModelServer::getInstance()));
  Routes::Post(router, "/users/login",
               Routes::bind(&UserModelServer::UserLoginRest,
                            UserModelServer::getInstance()));

  // resource aggregation
  Routes::Post(router, "/resource/register",
               Routes::bind(&AggregationServer::StorageResRegisterRest,
                            AggregationServer::getInstance()));
  Routes::Post(router, "/resource/logout",
               Routes::bind(&AggregationServer::StorageResLogoutRest,
                            AggregationServer::getInstance()));
}

void RestServer::doAuth(const Rest::Request& request,
                        Http::ResponseWriter response) {
  printCookies(request);
  response.cookies().add(Http::Cookie("lang", "en-US"));
  response.send(Http::Code::Ok);
}

RestServer* init_rest() {
  auto _config = HvsContext::get_context()->_config;
  auto rest_port = _config->get<int>("rest.port");
  auto rest_thread = _config->get<int>("rest.thread_num");
  auto ip = _config->get<std::string>("ip");
  if (!rest_port) {
    std::cerr << "restserver error: invalid port." << std::endl;
    return nullptr;
  } else if (!rest_thread) {
    std::cerr << "restserver error: invalid thread num." << std::endl;
    return nullptr;
  } else if (!ip) {
    std::cerr << "restserver warning: invalid ip, turning to use 0.0.0.0"
              << std::endl;
    ip = "0.0.0.0";
  }

  Port port(*rest_port);
  Address addr(*ip, port);
  RestServer* stats = new RestServer(addr);
  stats->init(*rest_thread);  //[原始函数]
  stats->start();  //调用Thread类的create函数, creat函数调用entry函数, 开启线程
                   //[后加函数]
  if (*rest_port == 0) {
    // if rest port in configuration is 0, try using random port in ephemeral
    // port range.
    usleep(100000); // wait 100 ms. rest server may started.
    std::cerr << "restserver info: port 0. Using random port " << stats->getPort()
              << std::endl;
  }
  return stats;
}

void stop_rest(RestServer* rest) {
  rest->shutdown();
  delete rest;
}

void* RestServer::entry() {
  signal(SIGINT, CtrlStop);

  httpEndpoint->setHandler(router.handler());
  httpEndpoint->serve();

  return NULL;
}

}  // namespace hvs