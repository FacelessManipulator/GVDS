#pragma once

#include <cerrno>
#include <shared_mutex>
#include "client.h"
#include "common/Thread.h"
#include "hvs_struct.h"
#include "msg/node.h"
#include <unordered_map>

namespace hvs {
class ClientRpc : public ClientModule {
 private:
  virtual void start() override;
  virtual void stop() override;

 private:
  std::shared_mutex rpc_mutex;
  std::unordered_map<std::string, std::shared_ptr<RpcClient>> rpc_clients;
  std::shared_ptr<RpcClient> connect(std::shared_ptr<IOProxyNode> node);

 public:
  ClientRpc(const char* name, Client* cli) : ClientModule(name, cli) {
    isThread = true;
  }

  template <typename... Args>
  std::shared_ptr<RPCLIB_MSGPACK::object_handle> call(
      std::shared_ptr<IOProxyNode> node, std::string const& func_name,
      Args... args);

  friend class Client;
};

template <typename... Args>
std::shared_ptr<RPCLIB_MSGPACK::object_handle> ClientRpc::call(
    std::shared_ptr<IOProxyNode> node, std::string const& func_name,
    Args... args) {
  // TODO: We assume RpcClient can concurently call
  auto rpcc = connect(node);
  auto res = rpcc->call(func_name, args...);
  if (!res) {
    return nullptr;
  } else {
    return std::make_shared<RPCLIB_MSGPACK::object_handle>(std::move(*res));
    ;
  }
}

extern struct fuse_operations hvsfs_oper;
}  // namespace hvs