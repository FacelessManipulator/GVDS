#include "msg/rpc.h"
#include "msg/bind_demo.h"
using namespace hvs;
using namespace std;

RpcServer* hvs::init_rpcserver() {
  auto _config = HvsContext::get_context()->_config;
  auto ip = _config->get<string>("ioproxy.ip").value_or("0.0.0.0");
  auto port = _config->get<int>("ioproxy.rpc_port");
  auto workers = _config->get<int>("ioproxy.rpc_workers");
  if (!port || *port <= 0 || *port >= 65535) {
    dout(-1) << "ERROR: invaild rpc port, should be ungisned number" << dendl;
  } else if (!workers || *workers <= 0) {
    dout(-1) << "ERROR: invaild rpc workers, should be unsigned number"
             << dendl;
  } else {
    // success, pass
  }
  RpcServer* rpc_server = new RpcServer(ip, *port);
  hvs_rpc_bind(rpc_server);
  rpc_server->run(*workers);
  return rpc_server;
}

RpcClient::RpcClient(const std::string address, const unsigned port)
    : _address(address), _port(port) {
  auto _config = HvsContext::get_context()->_config;
  auto timeout = _config->get<int>("ioproxy.rpc_timeout");
  auto retry = _config->get<int>("ioproxy.rpc_retry");
  if (!timeout && *timeout <= 0) {
    dout(-1) << "ERROR: invaild rpc timeout, should bigger than 0." << dendl;
  } else if (!retry && *retry < 1) {
    dout(-1) << "ERROR: invaild rpc retry times." << dendl;
  }
  _client = std::make_unique<rpc::client>(address, port);
  // TODO: avoid timeout
 _client->set_timeout(*timeout);
 _client->set_keepalive(true);
  _retry = static_cast<unsigned int>(*retry);
}