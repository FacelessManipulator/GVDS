#pragma once

#include <boost/pool/singleton_pool.hpp>
#include <cerrno>
#include <shared_mutex>
#include "common/LRU.h"
#include "client.h"
#include "common/Thread.h"
#include "io_proxy/rpc_types.h"

#define CACHE_UNIT_SIZE 512

namespace hvs {

class ClientCache : public ClientModule {
 private:
  virtual void start() override;
  virtual void stop() override;


 private:
  std::shared_mutex cache_mu;
  int stat_cache_ct;
  LRU<std::string, struct stat*> missing;
  LRU<std::string, struct stat*> cached;
  // just use map to store stat first

 public:
  int max_stat_cache_ct;
  enum Status {
      FOUND,
      NOT_FOUND,
      FOUND_MISSING
  };
 public:
  Status get_stat(const std::string& path, struct stat* dest);
  bool set_stat(const std::string& path, struct stat* src);
  void expire_stat(const std::string& path);

 protected:
  Status lookup(const std::string& key, struct stat*& res);

 public:
  ClientCache(const char* name, Client* cli) : ClientModule(name, cli) {
    isThread = true;
  }
  struct stat_pool {};
  struct data_cache_unit {};
  typedef boost::singleton_pool<stat_pool, sizeof(struct stat)>
      stat_pool_sig;
  typedef boost::singleton_pool<data_cache_unit, sizeof(char)*CACHE_UNIT_SIZE> data_cache_pool;
};
}  // namespace hvs