#pragma once

#include <cerrno>
#include <shared_mutex>
#include <unordered_map>
#include "client.h"
#include "common/Thread.h"
#include "hvs_struct.h"
#include "msg/node.h"
#include "msg/udt_client.h"

namespace hvs {
class ClientRpc : public ClientModule {
 private:
  virtual void start() override;
  virtual void stop() override;

 private:
  UDTClient udt_client;
  std::shared_mutex rpc_mutex;
  std::unordered_map<std::string, std::shared_ptr<RpcClient>> rpc_clients;
  std::unordered_map<std::string, std::shared_ptr<ClientSession>> udt_clients;
  std::shared_ptr<RpcClient> rpc_channel(std::shared_ptr<IOProxyNode> node);
  std::shared_ptr<ClientSession> udt_channel(std::shared_ptr<IOProxyNode> node);

 public:
  ClientRpc(const char* name, Client* cli) : ClientModule(name, cli) {
    isThread = true;
  }

  template <typename... Args>
  std::shared_ptr<RPCLIB_MSGPACK::object_handle> call(
      std::shared_ptr<IOProxyNode> node, std::string const& func_name,
      Args... args);

  int write_data(std::shared_ptr<IOProxyNode> node, ioproxy_rpc_buffer& buf);

  friend class Client;
};

template <typename... Args>
std::shared_ptr<RPCLIB_MSGPACK::object_handle> ClientRpc::call(
    std::shared_ptr<IOProxyNode> node, std::string const& func_name,
    Args... args) {
  // TODO: We assume RpcClient can concurently call
  auto rpcc = rpc_channel(node);
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