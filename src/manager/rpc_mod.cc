#include "manager/rpc_mod.h"

using namespace std;
using namespace hvs;

void ManagerRpc::start() {}

void ManagerRpc::stop() {
  for (auto rpcc : rpc_clients) {
    rpcc.second->shutdown();
  }
}

std::shared_ptr<RpcClient> ManagerRpc::rpc_channel(
    std::shared_ptr<IOProxyNode> node) {
  // Found, already established connection, maybe out-of-date.
  // Currently we not mantain the exists connection.
  rpc_mutex.lock_shared();
  auto rpcc = rpc_clients.find(node->uuid);
  rpc_mutex.unlock_shared();

  if (rpcc != rpc_clients.end()) {
    return rpcc->second;
  }
  // Just try create it
  auto rpcp = make_shared<RpcClient>(node->ip, node->rpc_port);
  rpc_mutex.lock();
  rpc_clients.try_emplace(node->uuid, rpcp);
  rpc_mutex.unlock();
  return rpcp;
}
