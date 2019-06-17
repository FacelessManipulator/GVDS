#include "client/cache_mod.h"
#include "client/msg_mod.h"

using namespace hvs;
using namespace std;

void ClientCache::start() {}

void ClientCache::stop() {}

void* ClientCache::entry() {}

ioproxy_rpc_statbuffer* ClientCache::get_stat(std::shared_ptr<IOProxyNode> iop,
                                              const std::string& path) {
  string global_path(iop->uuid);
  global_path.append(path);


  auto res = client->rpc->call(iop, "ioproxy_stat", path.c_str());
  if (!res) {
    // rpc call failed
    return nullptr;
  }
  // singleton memory pool is thread safe
  auto retbuf = res->as<ioproxy_rpc_statbuffer>();
  if (retbuf.error_code) {
    // stat failed on remote server
    missing.touch(path);
    return nullptr;
  }
  auto st_buf = static_cast<ioproxy_rpc_statbuffer*>(stat_pool_sig::malloc());
  memcpy(st_buf, &retbuf, sizeof(ioproxy_rpc_statbuffer));
  return st_buf;
}

ioproxy_rpc_statbuffer* ClientCache::lookup(const std::string& key) {
  if(missing.hit(key)) {
    // found in missing list, means not exists.
    return nullptr;
  } else {
    auto buf_obj = cached.find((key);
    if(!buf_obj.has_value()) {
      return nullptr;
    } else {
      cached.hit(key);
      return buf_obj->data;
    }
  }
}