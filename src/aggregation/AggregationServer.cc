#include "AggregationServer.h"
#include "StorageResBicInfo.h"
#include "context.h"
#include "datastore/couchbase_helper.h"
#include "datastore/datastore.h"

using namespace hvs;
using namespace Pistache;
using namespace std;

AggregationServer* AggregationServer::instance = nullptr;
AggregationServer::AggregationServer() {
  auto _config = HvsContext::get_context()->_config;
  auto _bn = _config->get<std::string>("couchbase.bucket");
  if (!_bn) {
    dout(-1) << "ERROR: invaild couchback bucket." << dendl;
    exit(EXIT_FAILURE);
  } else {
    bucket_name = *_bn;
  }
}

void AggregationServer::StorageResRegisterRest(const Rest::Request& request,
                                               Http::ResponseWriter response) {
  auto info = request.body();
  StorageResBicInfo resourceBicInfo;
  resourceBicInfo.deserialize(info);
  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket_name, hvs::DatastoreType::couchbase, true);
  //查看是否有该资源
  auto [pvalue, error_0] = dbPtr->get(resourceBicInfo.key());

  if (error_0) {
    dout(5) << "db get error" << dendl;
    response.send(Http::Code::Not_Found, "db get error");
  }

  if (!pvalue.get()) {
    dout(5) << "DB[resource]: the storage source already exit" << dendl;
    response.send(Http::Code::Not_Found,
                  "DB[resource]: the storage source already exit");
    return;
  }

  resourceBicInfo.state = Normal;
  std::string res_seri = resourceBicInfo.serialize();
  int rst = dbPtr->set(resourceBicInfo.key(), res_seri);

  if (rst != 0) {
    dout(5) << "db set error" << dendl;
    response.send(Http::Code::Not_Found, "db set error");
    return;
  }
  response.send(Http::Code::Ok, "StorageResRegister successful");
}

void AggregationServer::StorageResLogoutRest(const Rest::Request& request,
                                             Http::ResponseWriter response) {
  cout << "====== start AggregationServer function: StorageResLoginRest ======"
       << endl;
  auto info = request.body();

  StorageResBicInfo resourceBicInfo;
  resourceBicInfo.deserialize(info);

  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket_name, hvs::DatastoreType::couchbase, true);

  //查看是否有该资源
  auto [pvalue, error_0] = dbPtr->get(resourceBicInfo.key());

  if (error_0) {
    dout(5) << "db get error" << dendl;
    response.send(Http::Code::Not_Found, "db get error");
    return;
  }

  if (!pvalue.get()) {
    dout(5) << "DB[resource]: the storage source already exit" << dendl;
    response.send(Http::Code::Not_Found,
                  "DB[resource]: the storage source not exit");
    return;
  }

  resourceBicInfo.state = Logouting;
  std::string res_seri = resourceBicInfo.serialize();
  int rst = dbPtr->set(resourceBicInfo.key(), res_seri);

  if (rst != 0) {
    dout(5) << "db set error" << dendl;
    response.send(Http::Code::Not_Found, "db set error");
    return;
  }
  response.send(Http::Code::Ok, "StorageResLogin successful");
}

AggregationServer::~AggregationServer() {}
