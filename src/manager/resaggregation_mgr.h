#pragma once

#include "manager.h"
#include "aggregation_struct.h"

namespace gvds
{
class ResAggregation_MGR : public ManagerModule
{
private:
  virtual void start() override;
  virtual void stop() override;
  virtual void router(Pistache::Rest::Router &) override;

private:
  std::string bucket;

public:
  ResAggregation_MGR(const char *name) : ManagerModule(name) {}
  bool add(const Rest::Request &request, Http::ResponseWriter response);
  bool list(const Rest::Request &request, Http::ResponseWriter response);
  bool del(const Rest::Request &request, Http::ResponseWriter response);
  bool update(const Rest::Request &request, Http::ResponseWriter response);

private:
  std::shared_ptr<StorageResource> parse_request(const Rest::Request &request);
};

} // namespace gvds