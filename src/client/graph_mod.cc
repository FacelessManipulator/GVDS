#include "client/graph_mod.h"
using namespace hvs;
using namespace std;

void ClientGraph::start() {
  // TODO: currently we makeup an fake space. we should replace it with space mod.
  auto ion1 = make_shared<IOProxyNode>();
  ion1->ip = "127.0.0.1";
  ion1->rpc_port = 9092;
  ion1->data_port = 9095;
  set_mapping("127001", ion1, "/");

  auto ion2 = make_shared<IOProxyNode>();
  ion2->ip = "192.168.5.224";
  ion2->rpc_port = 9092;
  ion2->data_port = 9095;
  set_mapping("192168", ion2, "/");

}

void ClientGraph::stop() {}

std::tuple<std::shared_ptr<IOProxyNode>, std::string> ClientGraph::get_mapping(
    const std::string& path) {
  std::string space_name = path.substr(0, path.find("/"));
  graph_mutex.lock_shared();
  auto mapping = mappings.find(space_name);
  graph_mutex.unlock_shared();
  if (mapping != mappings.end()) {
    return mapping->second;
  } else {
    // mapping not found, maybe deleted
    return {nullptr, ""};
  }
}

std::vector<Space> ClientGraph::list_space() {
  vector<Space> spaces;
  spaces.emplace_back("127001");
  spaces.emplace_back("192168");
  return spaces;
}