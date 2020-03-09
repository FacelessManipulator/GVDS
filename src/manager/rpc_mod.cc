#include "manager/rpc_mod.h"

using namespace std;
using namespace hvs;

void ManagerRpc::start() {}

void ManagerRpc::stop() {
  // for (auto rpcc : rpc_clients) {
  //   rpcc.second->shutdown();
  // }
}

// std::shared_ptr<RpcClient> ManagerRpc::rpc_channel(
//     std::shared_ptr<IOProxyNode> node, bool reconnect) {
//   // Found, already established connection, maybe out-of-date.
//   // Currently we not mantain the exists connection.
//   lock_guard<mutex> lock(rpc_mutex);
//   auto rpcc = rpc_clients.find(node->uuid);

//   if (rpcc != rpc_clients.end()) {
//     auto& rpc_client = rpcc->second;
//     if(reconnect) {
//       auto rpcp = make_shared<RpcClient>(node->ip, node->rpc_port);
//       rpc_clients[node->uuid].swap(rpcp);
//     }
//     return rpc_client;
//   }
//   // Just try create it
//   // may cost a lot of moment
//   auto rpcp = make_shared<RpcClient>(node->ip, node->rpc_port);
//   rpc_clients.try_emplace(node->uuid, rpcp);
//   return rpcp;
// }