#pragma once

#include "manager.h"
#include "hvs_struct.h"

namespace hvs {
class IOProxy_MGR : public ManagerModule, public Thread {
 private:
  virtual void start() override;
  virtual void stop() override;
  virtual void router(Pistache::Rest::Router&) override;
 private:
  std::string bucket;
 protected:
  virtual void* entry() override;

 public:
  IOProxy_MGR(const char* name) :ManagerModule(name), m_stop(true) {}
  bool add(const Rest::Request& request, Http::ResponseWriter response);
  bool list(const Rest::Request& request, Http::ResponseWriter response);
  bool del(const Rest::Request& request, Http::ResponseWriter response);
  bool update(const Rest::Request& request, Http::ResponseWriter response);

  private:
  std::shared_ptr<IOProxyNode> parse_request(const Rest::Request& request);

  private:
  bool m_stop;
};

}  // namespace hvs