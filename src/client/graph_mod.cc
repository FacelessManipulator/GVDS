#include "client/graph_mod.h"
#include "client/msg_mod.h"

using namespace hvs;
using namespace std;

void ClientGraph::start() { fresh_ioproxy(); }

void ClientGraph::stop() {}

std::tuple<std::shared_ptr<IOProxyNode>, std::string> ClientGraph::get_mapping(
    const std::string& path) {
  auto [zone, space, rpath] = client->zone->locatePosition(path);
  if(!zone || !space) {
      return {nullptr, rpath};
  }
  // TODO: best node selection mod handle this
  // TODO: 选择对应中心的IO代理
  std::shared_ptr<IOProxyNode> iop;
  std::string centerID = space->hostCenterID;
  for (auto ioproxy : ioproxy_list) {
    if(ioproxy.second->cid == centerID)
      iop = ioproxy.second;
  }
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
  string endpoint = client->get_manager();
  string res = client->rpc->get_request(endpoint, "/ioproxy");
  if (res.size()) {
    json_decode(res, ioproxy_list);
    dout(10) << "INFO: get " << ioproxy_list.size() << " ioproxy from manager."
             << dendl;
  }
}
