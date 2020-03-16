#pragma once

#include <map>
#include <string>
#include "gvds_struct.h"
#include "manager.h"
#include "monitor_counter.hpp"

namespace gvds {
class IOProxy_MGR : public ManagerModule, public Thread {
 private:
  virtual void start() override;
  virtual void stop() override;
  virtual void router(Pistache::Rest::Router&) override;

 private:
  std::string bucket;
  std::map<std::string, std::shared_ptr<IOProxyNode>> live_ioproxy;
  std::map<std::string, MonitorSpeed> iop_stat;

 protected:
  virtual void* entry() override;

 public:
  IOProxy_MGR(const char* name)
      : ManagerModule(name), m_stop(true) {
    isThread = true;
  }
  bool add(const Rest::Request& request, Http::ResponseWriter response);
  bool list(const Rest::Request& request, Http::ResponseWriter response);
  bool del(const Rest::Request& request, Http::ResponseWriter response);
  bool update(const Rest::Request& request, Http::ResponseWriter response);
  bool detail(const Rest::Request& request, Http::ResponseWriter response);


private:
  std::shared_ptr<IOProxyNode> parse_request(const Rest::Request& request);
  void init_ioproxy_list();

 private:
  bool m_stop;
};

}  // namespace gvds