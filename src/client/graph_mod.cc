#include "client/graph_mod.h"
#include "graph_mod.h"

using namespace hvs;
using namespace std;

void ClientGraph::start() {
  // TODO: currently we makeup an fake space. we should replace it with space mod.
  auto ion1 = make_shared<IOProxyNode>();
  ion1->ip = "127.0.0.1";
  ion1->rpc_port = 9092;
  ion1->data_port = 9095;
  set_mapping("306a36a6-0b69-40c5-b8e7-8eac7ad2bf7b", ion1, "/306a36a6-0b69-40c5-b8e7-8eac7ad2bf7b");
  set_mapping("9221b15c-2b99-4edd-953b-4aaf629aa3b0", ion1, "/9221b15c-2b99-4edd-953b-4aaf629aa3b0");
  set_mapping("f0a15be3-3196-4145-9b90-0f46b94d2eca", ion1, "/f0a15be3-3196-4145-9b90-0f46b94d2eca");

  auto ion2 = make_shared<IOProxyNode>();
  ion2->ip = "192.168.5.224";
  ion2->rpc_port = 9092;
  ion2->data_port = 9095;
  set_mapping("152a15b5-06f0-4a7b-91d9-21884a41b234", ion2, "/152a15b5-06f0-4a7b-91d9-21884a41b234");
  set_mapping("ae05b3c2-3689-4fe3-80c2-f82c8f7d3b49", ion2, "/ae05b3c2-3689-4fe3-80c2-f82c8f7d3b49");
  set_mapping("c84e6857-d34d-4e80-a2cc-a77972dbd35c", ion2, "/c84e6857-d34d-4e80-a2cc-a77972dbd35c");
}

void ClientGraph::stop() {}

std::tuple<std::shared_ptr<IOProxyNode>, std::string> ClientGraph::get_mapping(
    const std::string& space_uuid) {
  graph_mutex.lock_shared();
  auto mapping = mappings.find(space_uuid);
  graph_mutex.unlock_shared();
  if (mapping != mappings.end()) {
    return mapping->second;
  } else {
    // mapping not found, maybe deleted
    return {nullptr, ""};
  }
}

std::vector<Space> ClientGraph::list_space(std::string zonename) {
  vector<Space> spaces;
  auto mapping = client->zone->zonemap.find(zonename);
  if(mapping !=  client->zone->zonemap.end()) {
    ZoneInfo zoneinfo;
    zoneinfo.deserialize(mapping->second);
    for(auto it : zoneinfo.spaceBicInfo.spaceID){
        spaces.emplace_back(zoneinfo.spaceBicInfo.spaceName[it], it, zoneinfo.spaceBicInfo.spaceSize[it]); // 寻找空间
    }
  }
  return spaces;
}

std::vector<Zone> ClientGraph::list_zone() {
  vector<Zone> zones;
//  client->zone->GetZoneInfo("127.0.0.1", 54485, "101");
  for(const auto &zo : client->zone->zonemap){
    zones.emplace_back(zo.first);
  }
  return zones;
}
