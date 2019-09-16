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

class Buffer {
  
};

class ClientCache : public ClientModule, public Thread {
 private:
  virtual void start() override;
  virtual void stop() override;

 protected:
  void* entry() override;
  ioproxy_rpc_statbuffer* lookup(const std::string& key);

 private:
  ioproxy_rpc_buffer* not_exists;
  std::shared_mutex cache_mu;
  LRU<std::string, ioproxy_rpc_statbuffer*> missing;
  LRU<std::string, ioproxy_rpc_statbuffer*> cached;

 public:
  ioproxy_rpc_statbuffer* get_stat(std::shared_ptr<IOProxyNode> node, const std::string& path);

  

 public:
  ClientCache(const char* name, Client* cli) : ClientModule(name, cli) {
    isThread = true;
  }
  struct stat_pool {};
  struct data_cache_unit {};
  typedef boost::singleton_pool<stat_pool, sizeof(ioproxy_rpc_statbuffer)>
      stat_pool_sig;
  typedef boost::singleton_pool<data_cache_unit, sizeof(char)*CACHE_UNIT_SIZE> data_cache_pool;
};
}  // namespace hvs