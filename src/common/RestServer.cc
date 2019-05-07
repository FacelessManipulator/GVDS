#include <signal.h>

#include "aggregation/AggregationServer.h"
#include "common/RestServer.h"
namespace hvs {

void RestServer::init(size_t thr) {
  auto opts = Http::Endpoint::options().threads(thr).flags(
      Tcp::Options::InstallSignalHandler);
  httpEndpoint->init(opts);
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
  httpEndpoint->setHandler(router.handler());
  httpEndpoint->serve();

  return NULL;
}

}  // namespace hvs