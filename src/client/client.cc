#include "client/client.h"

#include "client/fuse_mod.h"
#include "client/graph_mod.h"
#include "client/rpc_mod.h"
#include "client/zone_mod.h"
#include "client/ipc_mod.h"

using namespace hvs;
using namespace std;

void Client::start() {
  // init rest server
  auto _config = HvsContext::get_context()->_config;
  auto rest_port = _config->get<int>("rest.port");
  auto rpc_port = _config->get<int>("rpc.port");
  auto ip = _config->get<std::string>("ip");
  if (!rest_port) {
    std::cerr << "restserver error: invalid port." << std::endl;
  } else if (!rpc_port) {
    std::cerr << "restserver error: invalid rpc port." << std::endl;
  } else if (!ip) {
    std::cerr << "restserver warning: invalid ip, turning to use 0.0.0.0"
              << std::endl;
    ip = "127.0.0.1";
  }

  // init modules
  for (auto mod : modules) {
    mod.second->start();
  }
  m_stop = false;
  create("Client");
}

void Client::stop() {
  // stop modules
  for (auto mod : modules) {
    mod.second->stop();
  }
  m_stop = true;
}

void* Client::entry() {
  // we may listen on unix socket after midterm, but currently manager thread do
  // nothing
  auto fuse_mod = modules.find("fuse");
  if (fuse_mod == modules.end()) {
    dout(-1) << "Error: fuse module not loaded." << dendl;
    return nullptr;
  }
  static_cast<ClientFuse*>(fuse_mod->second.get())->run();
  return nullptr;
}

void Client::serialize_impl() { put("name", Node::name); }

void Client::registe_module(std::shared_ptr<ClientModule> mod) {
  modules[mod->module_name] = mod;
}

namespace hvs {
hvs::Client* init_client() {
  auto _config = HvsContext::get_context()->_config;
  auto ip = _config->get<std::string>("ip");
  if (!ip) {
    std::cerr << "Warning: invalid ip, turning to use 0.0.0.0"
              << std::endl;
    ip = "0.0.0.0";
  }
  auto client = new Client();
  hvs::HvsContext::get_context()->node = client;
  // registe modlues in manager node
  client->graph = std::make_shared<ClientGraph>("graph", client);
  client->rpc = std::make_shared<ClientRpc>("rpc", client);
  client->fuse = std::make_shared<ClientFuse>("fuse", client);
  client->zone = std::make_shared<ClientZone>("zone", client); // 空间客户端模块
  client->registe_module(client->graph);
  client->registe_module(client->rpc);
  client->registe_module(client->fuse);
  client->registe_module(client->zone); // 注册空间客户端模块
  client->registe_module(std::make_shared<ClientIPC>("ipc", client));

  client->start();
  return client;
}

void destroy_client(hvs::Client* client) {
  client->stop();
  delete client;
}
}  // namespace hvs