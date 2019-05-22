#include "client/rpc_mod.h"

using namespace std;
using namespace hvs;

void ClientRpc::start() {}

void ClientRpc::stop() {
  for (auto rpcc : rpc_clients) {
    rpcc.second->shutdown();
  }
}

std::shared_ptr<RpcClient> ClientRpc::rpc_channel(
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

std::shared_ptr<ClientSession> ClientRpc::udt_channel(
    std::shared_ptr<IOProxyNode> node) {
  // Found, already established connection, maybe out-of-date.
  // Currently we not mantain the exists connection.
  rpc_mutex.lock_shared();
  auto udtc = udt_clients.find(node->uuid);
  rpc_mutex.unlock_shared();

  if (udtc != udt_clients.end()) {
    return udtc->second;
  }
  // Just try create it
  auto udtp = udt_client.create_session(node->ip, node->data_port);
  if(!udtp.get())
    return nullptr;
  rpc_mutex.lock();
  udt_clients.try_emplace(node->uuid, udtp);
  rpc_mutex.unlock();
  return udtp;
}

int ClientRpc::write_data(std::shared_ptr<IOProxyNode> node, ioproxy_rpc_buffer& buf) {
  // TODO: We assume RpcClient can concurently call
  auto udtc = udt_channel(node);
  if(!udtc.get())
    return -ETIMEDOUT;
  int id = udtc->write(buf);
  int res = udtc->wait_op(id);
  return res;
}