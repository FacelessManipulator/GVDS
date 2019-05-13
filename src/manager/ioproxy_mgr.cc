#include "manager/ioproxy_mgr.h"
#include "datastore/datastore.h"
#include "datastore/couchbase_helper.h"

using namespace hvs;
using namespace std;
using namespace Pistache::Rest;
using namespace Pistache::Http;

void IOProxy_MGR::start() {
  auto _config = HvsContext::get_context()->_config;
  auto _bn = _config->get<std::string>("couchbase.bucket");
  bucket = _bn.value_or("test");  // use test bucket or predefined
}

void IOProxy_MGR::stop() {}

void IOProxy_MGR::router(Router& router) {
  Routes::Get(router, "/ioproxy", Routes::bind(&IOProxy_MGR::list, this));
  Routes::Get(router, "/ioproxy/:id", Routes::bind(&IOProxy_MGR::list, this));
  Routes::Post(router, "/ioproxy", Routes::bind(&IOProxy_MGR::add, this));
  Routes::Delete(router, "/ioproxy/:id", Routes::bind(&IOProxy_MGR::del, this));
  Routes::Put(router, "/ioproxy/:id", Routes::bind(&IOProxy_MGR::update, this));
}

bool IOProxy_MGR::add(const Rest::Request& req, Http::ResponseWriter res) {
  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket, hvs::DatastoreType::couchbase, true);
  auto iop = parse_request(req);
  // broken request, cannot assemble a ioproxy_node
  if (!iop) {
    res.send(Code::Bad_Request);
    return false;
  }
  auto cbd = static_cast<CouchbaseDatastore*>(dbPtr.get());
  auto err = cbd->insert(iop->key(), iop->json_value());
  usleep(100000); // may take 100ms to be effective
  res.send(Code::Accepted, iop->key());
  return true;
}

bool IOProxy_MGR::list(const Rest::Request& req, Http::ResponseWriter res) {
  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket, hvs::DatastoreType::couchbase, true);
  auto cbd = static_cast<CouchbaseDatastore*>(dbPtr.get());
  char query[256];
  snprintf(query, 256,
           "select * from `%s` where SUBSTR(META().id,0,%d) == '%s' order by "
           "META().id",
           bucket.c_str(), IOProxyNode::prefix().length(),
           IOProxyNode::prefix().c_str());
  auto [iop_infos, err] = cbd->n1ql(string(query));
  if (!err) {
    res.send(Code::Ok, json_encode(*iop_infos));
  } else {
    res.send(Code::Service_Unavailable);
  }
}

bool IOProxy_MGR::del(const Rest::Request& req, Http::ResponseWriter res) {
  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket, hvs::DatastoreType::couchbase, true);
  auto uuid = req.param(":id").as<std::string>();
  auto err = dbPtr->remove(uuid);
  if (!err)
    res.send(Code::Ok, uuid);
  else
    res.send(Code::Bad_Request, uuid);
  return true;
}

bool IOProxy_MGR::update(const Rest::Request& req, Http::ResponseWriter res) {}
std::shared_ptr<IOProxyNode> IOProxy_MGR::parse_request(
    const Rest::Request& req) {
  auto cnt = req.body();
  try {
    auto ion = make_shared<IOProxyNode>();
    ion->deserialize(cnt);
    return ion;
  } catch (Expectation& e) {
    return nullptr;
  }
}