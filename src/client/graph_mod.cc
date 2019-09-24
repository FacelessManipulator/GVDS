#include "client/graph_mod.h"
#include "client/OPTNode/opt_node.h"
#include "client/msg_mod.h"

using namespace hvs;
using namespace std;

void ClientGraph::start() { fresh_ioproxy(); m_stop=false; create("graph_mode");}

void ClientGraph::stop() {
  m_stop = true;
  join();
}

void* ClientGraph::entry() {
  while (!m_stop) {
    fresh_ioproxy();
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }
}

std::tuple<std::shared_ptr<IOProxyNode>, std::string> ClientGraph::get_mapping(
    const std::string& path) {
  auto [zone, space, rpath] = client->zone->locatePosition(path);
  if (!zone || !space) {
    return {nullptr, rpath};
  }
  // TODO: best node selection mod handle this
  // TODO: 选择对应中心的IO代理
  std::shared_ptr<IOProxyNode> iop;
  std::string centerID = space->hostCenterID;
  graph_mutex.lock_shared();
  for (const auto& ioproxy : *(ioproxy_list[centerID])) {
    if (ioproxy.second->status == IOProxyNode::Running) {
        iop = ioproxy.second;
        break;
    }
  }
  graph_mutex.unlock_shared();
  auto lpath = space->spacePath;
  lpath.append(rpath);
  return make_tuple(iop, lpath);
}

std::vector<string> ClientGraph::list_space(std::string zonename) {
  vector<string> spaces;
  auto mapping = client->zone->zonemap.find(zonename);
  if (mapping != client->zone->zonemap.end()) {
    auto zone = mapping->second;
    for (auto it : zone->spaceBicInfo) {
      spaces.push_back(it->spaceName);  // 寻找空间
    }
  }
  return spaces;
}

std::vector<std::string> ClientGraph::list_zone() {
  vector<string> zones;
  for (const auto& zo : client->zone->zonemap) {
    zones.push_back(zo.first);
  }
  return zones;
}

void ClientGraph::fresh_ioproxy() {
  auto endpoints = client->optNode->getNode(1);
  char url[256];
  for (auto endpoint : endpoints) {
    auto ioproxy_list_tmp = make_shared<map<std::string, std::shared_ptr<IOProxyNode>>>() ;
    snprintf(url, 256, "http://%s:%s", endpoint.ip_addr.c_str(), endpoint.port.c_str());
    string res = client->rpc->get_request(url, "/ioproxy");
    if (res.size()) {
      json_decode(res, *ioproxy_list_tmp);
      graph_mutex.lock();
      ioproxy_list[endpoint.center_id] = ioproxy_list_tmp;
      graph_mutex.unlock();
      dout(10) << "INFO: get " << ioproxy_list.size()
               << " ioproxy from manager." << dendl;
    }
  }
}
