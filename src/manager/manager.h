#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <map>
#include "context.h"

#include <pistache/http.h>
#include <pistache/router.h>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/thread/thread.hpp>
#include "common/RestServer.h"
#include "common/Thread.h"
#include "msg/node.h"
#include "msg/op.h"

class ManagerTest;
namespace hvs {
class ManagerModule;
class Manager : public Thread, public Node, public JsonSerializer {
 public:
  Manager() : m_stop(false), Node(MANAGER_NODE) {
    // TODO: should read from config file
  }
  bool start();
  void stop();
  void registe_module(std::shared_ptr<ManagerModule> mod);
  std::shared_ptr<ManagerModule> get_module(const std::string & mod_name);
  void route(Pistache::Rest::Router& router);
  int rest_port() {
    if (restserver) return restserver->getPort();
  }
  void manager_info(const Rest::Request& request,
                    Http::ResponseWriter response);
  void serialize_impl() override;
  ~Manager() override { stop(); };

 private:
  void* entry() override;

 private:
  // thread saft variables
  bool m_stop;
  std::unique_ptr<RestServer> restserver;
  std::map<std::string, std::shared_ptr<ManagerModule>> modules;
};

class ManagerModule {
 public:
  std::string module_name;
  Manager* mgr;

 protected:
  ManagerModule(const char* name) : module_name(name), mgr(nullptr) {}
  // implement router register functions
  virtual void router(Pistache::Rest::Router& router) {}
  // could involk in module starting stage
  virtual void start() {}
  // could involk in module destroying stage
  virtual void stop() {}
  // module doesn't have its own thread

  friend class Manager;
};

extern hvs::Manager* init_manager();
extern void destroy_manager(hvs::Manager* mgr);
}  // namespace hvs