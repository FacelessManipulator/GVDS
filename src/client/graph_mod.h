#pragma once

#include <cerrno>
#include <shared_mutex>
#include "client.h"
#include "common/Thread.h"
#include "gvds_struct.h"
#include "msg/node.h"
#include "zone_mod.h"
#include <unordered_map>

namespace gvds {
class ClientGraph : public ClientModule, public Thread {
  typedef std::vector<std::shared_ptr<IOProxyNode>> IOProxyListType;
 private:
  virtual void start() override;
  virtual void stop() override;
  void* entry();

 private:
  std::shared_mutex graph_mutex;
  bool m_stop;
  // currently I store space-gpath mappings in a map, please dev a space mod to handle this
//  std::unordered_map<std::string, std::tuple<std::shared_ptr<IOProxyNode>, std::string>> mappings;
  std::map<std::string, std::shared_ptr<IOProxyListType>> ioproxy_list;
  std::map<std::string, std::string> force_map;

 public:
  ClientGraph(const char* name, Client* cli) : ClientModule(name, cli), m_stop(true) {
    isThread = false;
  }

  // get global path of path
  std::tuple<std::shared_ptr<IOProxyNode>, std::string> get_mapping(const std::string& path);
  int set_force_mapping(const std::string& path, const std::string& cid);
  std::vector<std::string> list_space(std::string zonename);
  std::vector<std::string> list_zone();
  std::vector<std::string> list_owner_zone();
  void fresh_ioproxy();
  // 缓存空间路径到IO代理节点的映射
//  void set_mapping(const std::string& path, std::shared_ptr<IOProxyNode> ion, const std::string& rpath) {
//    mappings[path] = {ion, rpath};
//  }

  friend class Client;
};

extern struct fuse_operations gvdsfs_oper;
}  // namespace gvds