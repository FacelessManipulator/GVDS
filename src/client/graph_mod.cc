#include "client/graph_mod.h"
using namespace hvs;
using namespace std;

void ClientGraph::start() {
  // TODO: currently we makeup an fake space. we should replace it with space mod.
  auto ion = make_shared<IOProxyNode>();
  ion->ip = "127.0.0.1";
  ion->rpc_port = 9092;
  set_mapping("space", ion, "/");
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
  spaces.emplace_back("space");
  return spaces;
}