#include "client/cache_mod.h"
#include "client/msg_mod.h"

using namespace hvs;
using namespace std;

void ClientCache::start() {
    auto _config = HvsContext::get_context()->_config;
    max_stat_cache_ct = _config->get<int>("client.stat_cache_size").value_or(0);
    stat_cache_ct = 0;
}

void ClientCache::stop() {}

ClientCache::Status ClientCache::get_stat(const std::string& path, struct stat* dest) {
    cache_mu.lock_shared();
    struct stat* lk_res;
    auto fd_res = lookup(path, lk_res);
    if (fd_res == FOUND_MISSING || fd_res == NOT_FOUND) {
        cache_mu.unlock_shared();
        return fd_res;
    } else {
        memcpy(dest, lk_res, sizeof(struct stat));
        cache_mu.unlock_shared();
        return FOUND;
    }
}

bool ClientCache::set_stat(const std::string& path, struct stat* st_buf) {
    lock_guard<std::shared_mutex> lock(cache_mu);
    if (st_buf == nullptr) {
        // means file not exists
        missing.insert(path);
        return true;
    } else {
        missing.erase(path);
        struct stat* lk_res;
        // try look up stat in cache at first
        auto lk_st = lookup(path, lk_res);
        if (lk_st != FOUND) {
            // if stat cache reach max size, then return false
            if (stat_cache_ct >= max_stat_cache_ct)
                return false;
            // if stat not exists in cache, try create one
            lk_res = static_cast<struct stat*>(stat_pool_sig::malloc());
            cached[path] = lk_res;
            stat_cache_ct ++;
        } else {
            // stat buf already alloc in ClientCache, just do nothing
        }
        memcpy(lk_res, st_buf, sizeof(struct stat));
        return true;
    }
}

void ClientCache::expire_stat(const std::string& path) {
    lock_guard<std::shared_mutex> lock(cache_mu);
    // just expire it no matter whether it exists or not
    missing.erase(path);
    bool removed = cached.erase(path);
    if (removed) stat_cache_ct--;
}

ClientCache::Status ClientCache::lookup(const std::string& key, struct stat*& res) {
  if(missing.count(key) != 0) {
    // found in missing list, means not exists.
    return FOUND_MISSING;
  } else {
    auto buf_obj = cached.find(key);
    if(buf_obj == cached.end()) {
      return NOT_FOUND;
    } else {
      res = buf_obj->second;
      return FOUND;
    }
  }
}