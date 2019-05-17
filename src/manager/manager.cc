#include "manager/manager.h"
#include "manager/ioproxy_mgr.h"
#include "usermodel/UserModelServer.h"

using namespace hvs;
using namespace std;
using namespace Pistache::Rest;

void Manager::start() {
  // init rest server
  auto _config = HvsContext::get_context()->_config;
  auto rest_port = _config->get<int>("rest.port");
  auto rest_thread = _config->get<int>("rest.thread_num");
  auto ip = _config->get<std::string>("ip");
  if (!rest_port) {
    std::cerr << "restserver error: invalid port." << std::endl;
  } else if (!rest_thread) {
    std::cerr << "restserver error: invalid thread num." << std::endl;
  } else if (!ip) {
    std::cerr << "restserver warning: invalid ip, turning to use 0.0.0.0"
              << std::endl;
    ip = "0.0.0.0";
  }

  Port port(*rest_port);
  Address addr(*ip, port);

  restserver = make_unique<RestServer>(addr);
  restserver->init(*rest_thread);  //[原始函数]
  route(restserver->router);
  // init modules
  for (auto mod : modules) {
    mod->start();
    // set router in modules
    mod->router(restserver->router);
  }
  m_stop = false;
  restserver->start();
  create("Manager");
}

void Manager::stop() {
  // stop rest server
  if (restserver) stop_rest(restserver.release());
  // stop modules
  for (auto mod : modules) {
    mod->stop();
  }
  m_stop = true;
}

void* Manager::entry() {
  // we may listen on unix socket after midterm, but currently manager thread do
  // nothing
  if (restserver) {
    restserver->join();
  }
}

void Manager::route(Router& router) {
  Routes::Get(router, "/manager", Routes::bind(&Manager::manager_info, this));
}

void Manager::serialize_impl() {
  std::vector<std::string> mod_name;
  for (auto mod : modules) {
    mod_name.push_back(mod->module_name);
  }
  std::string ip = addr.to_string();
  int port = restserver->getPort();
  put("ip", ip);
  put("rest_port", port);
  put("name", Node::name);
  put("modules", mod_name);
}

void Manager::manager_info(const Rest::Request& req, Http::ResponseWriter res) {
  res.send(Pistache::Http::Code::Ok, serialize());
}

namespace hvs {
hvs::Manager* init_manager() {
  auto _config = HvsContext::get_context()->_config;
  auto ip = _config->get<std::string>("ip");
  if (!ip) {
    std::cerr << "restserver warning: invalid ip, turning to use 0.0.0.0"
              << std::endl;
    ip = "0.0.0.0";
  }
  auto mgr = new Manager();
  mgr->addr.from_string(*ip);
  // registe modlues in manager node
  mgr->registe_module(std::make_shared<IOProxy_MGR>("ioproxy manager"));
  mgr->registe_module(std::make_shared<UserModelServer>());
  hvs::HvsContext::get_context()->node = mgr;
  mgr->start();
  return mgr;
}

void destroy_manager(hvs::Manager* mgr) {
  mgr->stop();
  delete mgr;
}
}  // namespace hvs