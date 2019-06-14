#include "client/graph_mod.h"
#include "client/msg_mod.h"

using namespace hvs;
using namespace std;

void ClientGraph::start() {
  fresh_ioproxy();
  // TODO: 当前空间对应的IO代理是固定的，之后会重新建造一个映射表
//  auto ion1 = make_shared<IOProxyNode>();
//  ion1->ip = "127.0.0.1";
//  ion1->rpc_port = 9092;
//  ion1->data_port = 9095;
//  set_mapping("306a36a6-0b69-40c5-b8e7-8eac7ad2bf7b", ion1, "/306a36a6-0b69-40c5-b8e7-8eac7ad2bf7b");
//  set_mapping("9221b15c-2b99-4edd-953b-4aaf629aa3b0", ion1, "/9221b15c-2b99-4edd-953b-4aaf629aa3b0");
//  set_mapping("f0a15be3-3196-4145-9b90-0f46b94d2eca", ion1, "/f0a15be3-3196-4145-9b90-0f46b94d2eca");
//  set_mapping("ff035007-0134-4c88-89db-bb520e572f90", ion1, "/ff035007-0134-4c88-89db-bb520e572f90");
//  set_mapping("1351d46e-5c77-4d89-b5da-1be57a079bbd", ion1, "/1351d46e-5c77-4d89-b5da-1be57a079bbd");
//
//  auto ion2 = make_shared<IOProxyNode>();
//  ion2->ip = "192.168.5.224";
//  ion2->rpc_port = 9092;
//  ion2->data_port = 9095;
//  set_mapping("152a15b5-06f0-4a7b-91d9-21884a41b234", ion2, "/152a15b5-06f0-4a7b-91d9-21884a41b234");
//  set_mapping("ae05b3c2-3689-4fe3-80c2-f82c8f7d3b49", ion2, "/ae05b3c2-3689-4fe3-80c2-f82c8f7d3b49");
//  set_mapping("c84e6857-d34d-4e80-a2cc-a77972dbd35c", ion2, "/c84e6857-d34d-4e80-a2cc-a77972dbd35c");
}

void ClientGraph::stop() {}

std::tuple<std::shared_ptr<IOProxyNode>, std::string> ClientGraph::get_mapping(
    const std::string& space_uuid) {
//  graph_mutex.lock_shared();
//  auto mapping = mappings.find(space_uuid);
//  graph_mutex.unlock_shared();
//  if (mapping != mappings.end()) {
//    return mapping->second;
//  } else {
//    // mapping not found, maybe deleted
//    return {nullptr, ""};
//  }
    // TODO: best node selection mod handle this
    // TODO: 选择对应中心的IO代理
    std::shared_ptr<IOProxyNode> iop;
    std::string centerID;
    for(auto iop_it : ioproxy_list) {
        // TODO： 根据空间元数据信息，定位到空间位于哪个存储集群
        centerID = client->zone->spaceuuid_to_hostcenterID(space_uuid);
//        if(iop_it.second->ip == "127.0.0.1" && centerID == "10001"){
//            iop = iop_it.second;
//            break;
//        }
//        if(centerID == "20001" && iop_it.second->ip != "127.0.0.1"){
//            iop = iop_it.second;
//            iop->ip = "192.168.5.224";
//            break;
//        }
         iop = iop_it.second;
        iop->ip = "127.0.0.1";
      break;
    }
    std::cout << "IOPROXY: " << space_uuid << " " << centerID << " " << iop->ip << std::endl;

    if(ioproxy_list.size() > 0) {
        string forward_path("/");
//        forward_path.append(space_uuid);
        forward_path.append(client->zone->spaceuuid_to_spacerpath(space_uuid));  // TODO:获取远程路径，并拼接路径
//        std::cout << "Mapping: " << client->zone->spaceuuid_to_spacerpath(space_uuid)<< std::endl;
        return make_tuple(iop, forward_path);
    } else {
        return make_tuple(nullptr, "");
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

void ClientGraph::fresh_ioproxy() {
  string endpoint = client->get_manager();
  string res = client->rpc->get_request(endpoint, "/ioproxy");
  if(res.size()) {
    json_decode(res, ioproxy_list);
    dout(10) << "INFO: get " << ioproxy_list.size() << " ioproxy from manager." << dendl;
  }
}
