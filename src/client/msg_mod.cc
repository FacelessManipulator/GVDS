#include "client/msg_mod.h"

using namespace std;
using namespace hvs;
using namespace Pistache;

void ClientRpc::start() {}

void ClientRpc::stop() {
  for (auto rpcc : rpc_clients) {
    rpcc.second->shutdown();
  }
}

std::shared_ptr<RpcClient> ClientRpc::rpc_channel(
    std::shared_ptr<IOProxyNode> node, bool reconnect) {
  // Found, already established connection, maybe out-of-date.
  // Currently we not mantain the exists connection.
  lock_guard<mutex> lock(rpc_mutex);
  auto rpcc = rpc_clients.find(node->uuid);

  if (rpcc != rpc_clients.end()) {
    auto& rpc_client = rpcc->second;
    if(reconnect) {
      rpc_clients[node->uuid].reset(new RpcClient(node->ip, node->rpc_port));
    }
    return rpc_client;
  }
  // Just try create it
  // may cost a lot of moment
  auto rpcp = make_shared<RpcClient>(node->ip, node->rpc_port);
  rpc_clients.try_emplace(node->uuid, rpcp);
  return rpcp;
}

std::shared_ptr<ClientSession> ClientRpc::udt_channel(
    std::shared_ptr<IOProxyNode> node, bool reconnect) {
  // Found, already established connection, maybe out-of-date.
  // Currently we not mantain the exists connection.
  lock_guard<mutex> lock(rpc_mutex);
  auto udtc = udt_clients.find(node->uuid);

  if (udtc != udt_clients.end()) {
    auto& udtc_old = udtc->second;
    if(reconnect) {
      auto new_con = udt_client.create_session(node->ip, node->data_port);
      udtc_old.swap(new_con);
    }
    return udtc_old;
  }
  // Just try create it
  auto udtp = udt_client.create_session(node->ip, node->data_port);
  if (!udtp.get()) return nullptr;
  udt_clients.try_emplace(node->uuid, udtp);
  return udtp;
}

int ClientRpc::write_data(std::shared_ptr<IOProxyNode> node,
                          ioproxy_rpc_buffer& buf) {
  // TODO: We assume RpcClient can concurently call
  auto udtc = udt_channel(node);
  if (!udtc.get()) return -ETIMEDOUT;
  int id = udtc->write(buf);
  if(id == -ECONNRESET) {
    udtc = udt_channel(node, true);
    id = udtc->write(buf);
  }
  if(id < 0)
    return -ECONNREFUSED;
  auto res = udtc->wait_op(id);
  if(res)
    return res->error_code;
  else
    return -ETIMEDOUT;
}

unique_ptr<ioproxy_rpc_buffer> ClientRpc::read_data(
    std::shared_ptr<IOProxyNode> node, ioproxy_rpc_buffer& buf) {
  // TODO: We assume RpcClient can concurently call
  auto udtc = udt_channel(node);
  if (!udtc.get()) return nullptr;
  int id = udtc->write(buf);
  if(id < 0) {
    udtc = udt_channel(node, true);
    id = udtc->write(buf);
  }
  if(id < 0)
    return nullptr;
  auto res = udtc->wait_op(id);
  if(res)
    return res;
  else {
    // timeout
    return nullptr;
  }
}

std::shared_ptr<Pistache::Http::Client> ClientRpc::rest_channel(
    std::string endpoint) {
  // Found, already established connection, maybe out-of-date.
  // Currently we not mantain the exists connection.
  lock_guard<mutex> lock(rpc_mutex);
  auto restc = rest_clients.find(endpoint);

  if (restc != rest_clients.end()) {
    return restc->second;
  }
  // Just try create it
  auto restcp = make_shared<Http::Client>();
  auto opts = Http::Client::options().threads(2).maxConnectionsPerHost(8);
  restcp->init(opts);
  rest_clients[endpoint] = restcp;
  return restcp;
}

std::string ClientRpc::post_request(const std::string& endpoint,
                                    const std::string& url,
                                    const std::string& data) {
  try {
    auto restc = rest_channel(endpoint);
    string real_url = endpoint + url;
    auto response = restc->post(real_url).body(data).send();
    dout(15) << "Info: Client post request to " << url << dendl;

    auto prom_p = make_shared<promise<std::string>>();
    auto fu = prom_p->get_future();
    response.then(
        [prom_p, endpoint, this](Http::Response res) {
          dout(15) << "Info: Client post get response from " << endpoint
                   << dendl;
          rest_cookies[endpoint] = res.cookies();
          prom_p->set_value(std::move(res.body()));
        },
        [prom_p, endpoint](exception_ptr& exptr) {
          try {
            if (exptr) {
              rethrow_exception(exptr);
            }
          } catch (const std::exception& e) {
            dout(10) << "Warnning: Client post get response rejected from "
                     << endpoint << " reason: " << e.what() << dendl;
          }
          prom_p->set_value("");
        });
    auto status = fu.wait_for(std::chrono::seconds(3));
    if (status == std::future_status::timeout) {
      dout(5) << "ERROR: Client connot connect to " << endpoint << dendl;
      return {};
    } else if (status == std::future_status::ready) {
      auto res = fu.get();
      return res;
    }
  } catch (const exception& e) {
    dout(5) << "ERROR: connot connect to " << endpoint << url
            << " Reason: " << e.what() << dendl;
  }
}

string ClientRpc::get_request(const string& endpoint, const string& url) {
  try {
    auto restc = rest_channel(endpoint);
    string real_url = endpoint + url;
    auto response = restc->get(real_url).send();
    dout(15) << "Info: Client get request to " << url << dendl;

    auto prom_p = make_shared<promise<std::string>>();
    auto fu = prom_p->get_future();
    response.then(
        [prom_p, endpoint, this](Http::Response res) {
          dout(15) << "Info: Client post get response from " << endpoint
                   << dendl;
          rest_cookies[endpoint] = res.cookies();
          prom_p->set_value(std::move(res.body()));
        },
        [prom_p, endpoint](exception_ptr& exptr) {
          try {
            if (exptr) {
              rethrow_exception(exptr);
            }
          } catch (const std::exception& e) {
            dout(10) << "Warnning: Client post get response rejected from "
                     << endpoint << " reason: " << e.what() << dendl;
          }
          prom_p->set_value("");
        });
    auto status = fu.wait_for(std::chrono::seconds(3));
    if (status == std::future_status::timeout) {
      dout(5) << "ERROR: Client connot connect to " << endpoint << dendl;
      return {};
    } else if (status == std::future_status::ready) {
      auto res = fu.get();
      return res;
    }
  } catch (const exception& e) {
    dout(5) << "ERROR: connot connect to " << endpoint << url
            << " Reason: " << e.what() << dendl;
  }
}

string ClientRpc::delete_request(const string& endpoint, const string& url) {
  try {
    auto restc = rest_channel(endpoint);
    string real_url = endpoint + url;
    auto response = restc->del(real_url).send();
    dout(15) << "Info: Client get request to " << url << dendl;

    auto prom_p = make_shared<promise<std::string>>();
    auto fu = prom_p->get_future();
    response.then(
            [prom_p, endpoint, this](Http::Response res) {
                dout(15) << "Info: Client post get response from " << endpoint
                         << dendl;
                prom_p->set_value(std::move(res.body()));
                rest_cookies[endpoint] = res.cookies();
            },
            [prom_p, endpoint](exception_ptr& exptr) {
                try {
                  if (exptr) {
                    rethrow_exception(exptr);
                  }
                } catch (const std::exception& e) {
                  dout(10) << "Warnning: Client post get response rejected from "
                           << endpoint << " reason: " << e.what() << dendl;
                }
                prom_p->set_value("");
            });
    auto status = fu.wait_for(std::chrono::seconds(3));
    if (status == std::future_status::timeout) {
      dout(5) << "ERROR: Client connot connect to " << endpoint << dendl;
      return {};
    } else if (status == std::future_status::ready) {
      auto res = fu.get();
      return res;
    }
  } catch (const exception& e) {
    dout(5) << "ERROR: connot connect to " << endpoint << url
            << " Reason: " << e.what() << dendl;
  }
}
