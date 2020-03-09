#pragma once

#include <cerrno>
#include <mutex>
#include <unordered_map>
#include "manager.h"
#include "common/Thread.h"
#include "hvs_struct.h"
#include "msg/node.h"

namespace hvs {
class ManagerRpc : public ManagerModule {
 private:
  virtual void start() override;
  virtual void stop() override;

 private:
  std::mutex rpc_mutex;
  // std::unordered_map<std::string, std::shared_ptr<RpcClient>> rpc_clients;
  // std::shared_ptr<RpcClient> rpc_channel(std::shared_ptr<IOProxyNode> node, bool reconnect = false);

 public:
  ManagerRpc(const char* name) : ManagerModule(name) {
    isThread = true;
  }

  // template <typename... Args>
  // std::shared_ptr<RPCLIB_MSGPACK::object_handle> call(
  //     std::shared_ptr<IOProxyNode> node, std::string const& func_name,
  //     Args... args);

  friend class Manager;
};

// template <typename... Args>
// std::shared_ptr<RPCLIB_MSGPACK::object_handle> ManagerRpc::call(
//     std::shared_ptr<IOProxyNode> node, std::string const& func_name,
//     Args... args) {
//   // TODO: We assume RpcClient can concurently call
//   auto rpcc = rpc_channel(node, true);
//   auto res = rpcc->call(func_name, args...);
//   if (!res) {
//     // timeout? try reconnect
//     rpcc = rpc_channel(node, true);
//     res = rpcc->call(func_name, args...);
//   }
//   if(!res) {
//     return nullptr;
//   } else {
//     return std::make_shared<RPCLIB_MSGPACK::object_handle>(std::move(*res));
//   }
// }
}  // namespace hvs