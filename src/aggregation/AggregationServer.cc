#include "AggregationServer.h"
#include "datastore/datastore.h"
#include "datastore/couchbase_helper.h"
#include "common/debug.h"
#include "StorageResBicInfo.h"

using namespace hvs;

AggregationServer::AggregationServer()
{


}

void AggregationServer::StorageResRegisterRest(const Rest::Request& request, Http::ResponseWriter response)
{

    cout << "====== start AggregationServer function: StorageResRegisterRest ======"<< endl;
    auto info = request.body();

    StorageResBicInfo resourceBicInfo;
    resourceBicInfo.deserialize(info);
    
    std::shared_ptr<CouchbaseDatastore> dbPtr = std::make_shared<CouchbaseDatastore>(
    CouchbaseDatastore("resource_aggregation")); //TODO 更改表名
    dbPtr->init();


    //查看是否有该资源
    auto [pvalue, error_0] = dbPtr->get(resourceBicInfo.storage_src_id);

    if(error_0){
        dout(5) << "db get error" << dendl;
        response.send(Http::Code::Not_Found,"db get error");
    }

    if(*pvalue==""){
        dout(5) << "DB[resource]: the storage source already exit" << dendl;
        response.send(Http::Code::Not_Found,"DB[resource]: the storage source already exit");
    }

  resourceBicInfo.state = Normal;
  std::string res_seri = resourceBicInfo.serialize();
  int rst = dbPtr->set(resourceBicInfo.storage_src_id, res_seri);

  if(rst!=0){
        dout(5) << "db set error" << dendl;
        response.send(Http::Code::Not_Found,"db set error");
  }
  response.send(Http::Code::Ok,"StorageResRegister successful");

}

void AggregationServer::StorageResLogoutRest(const Rest::Request& request, Http::ResponseWriter response)
{

    cout << "====== start AggregationServer function: StorageResLoginRest ======"<< endl;
    auto info = request.body();

    StorageResBicInfo resourceBicInfo;
    resourceBicInfo.deserialize(info);
    
    std::shared_ptr<CouchbaseDatastore> dbPtr = std::make_shared<CouchbaseDatastore>(
    CouchbaseDatastore("resource_aggregation")); //TODO 更改表名
    dbPtr->init();


    //查看是否有该资源
    auto [pvalue, error_0] = dbPtr->get(resourceBicInfo.storage_src_id);

    if(error_0){
        dout(5) << "db get error" << dendl;
        response.send(Http::Code::Not_Found,"db get error");
    }

    if(*pvalue==""){
        dout(5) << "DB[resource]: the storage source already exit" << dendl;
        response.send(Http::Code::Not_Found,"DB[resource]: the storage source not exit");
    }

  resourceBicInfo.state = Logouting;
  std::string res_seri = resourceBicInfo.serialize();
  int rst = dbPtr->set(resourceBicInfo.storage_src_id, res_seri);

  if(rst!=0){
        dout(5) << "db set error" << dendl;
        response.send(Http::Code::Not_Found,"db set error");
  }
  response.send(Http::Code::Ok,"StorageResLogin successful");

}

AggregationServer::~AggregationServer()
{

}




