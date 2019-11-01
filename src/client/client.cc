#include "client/client.h"

#include "client/fuse_mod.h"
#include "client/graph_mod.h"
#include "client/msg_mod.h"
#include "client/zone_mod.h"
#include "client/ipc_mod.h"
#include "client/OPTNode/opt_node.h"
#include "client/clientuser/ClientUser.h"
#include "client/OPTNode/opt_node.h"
#include "client/queue.h"
#include "client/readahead.h"

using namespace hvs;
using namespace std;

void Client::start() {
  // init modules
  for (auto mod : uninit_modules) {
    mod->start();
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
  uninit_modules.push_back(mod);
}

string Client::get_manager() {
  auto centers = optNode->getNode(1);
  if(centers.size() > 0) {
    auto center = centers[0];
    char url[256];
    snprintf(url, 256, "http://%s:%s", center.ip_addr.c_str(), center.port.c_str());
    return string(url);
  } else {
    dout(-1) << "ERROR: Connot get manager endpoint from config file." << dendl;
    return "";
  }
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
  // register modules in manager node
  client->fuse = std::make_shared<ClientFuse>("fuse", client);
  client->rpc = std::make_shared<ClientRpc>("rpc", client);
  client->zone = std::make_shared<ClientZone>("zone", client); // 空间客户端模块
  client->graph = std::make_shared<ClientGraph>("graph", client);
   client->optNode = std::make_shared<SelectNode>("optNode", client);
  client->user = std::make_shared<ClientUser>("user", client);
  client->queue = std::make_shared<ClientBufferQueue>("queue", client);
  client->readahead = std::make_shared<ClientReadAhead>("readahead", client);

  client->registe_module(client->rpc);
  client->registe_module(client->optNode);
  client->registe_module(client->zone); // 注册空间客户端模块
  client->registe_module(std::make_shared<ClientIPC>("ipc", client));
  client->registe_module(client->graph);
  client->registe_module(client->user);
  client->registe_module(client->fuse);
  client->registe_module(client->queue);
  client->registe_module(client->readahead);

  client->start();
  return client;
}

void destroy_client(hvs::Client* client) {
  client->stop();
  delete client;
}
}  // namespace hvs