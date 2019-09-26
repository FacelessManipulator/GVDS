#include "manager/resaggregation_mgr.h"
#include "datastore/datastore.h"
#include "datastore/couchbase_helper.h"

using namespace hvs;
using namespace std;
using namespace Pistache::Rest;
using namespace Pistache::Http;

void ResAggregation_MGR::start()
{
  auto _config = HvsContext::get_context()->_config;
  auto _bn = _config->get<std::string>("couchbase.bucket");
  bucket = _bn.value_or("test"); // use test bucket or predefined
}

void ResAggregation_MGR::stop() {}

void ResAggregation_MGR::router(Router &router)
{

  Routes::Post(router, "/resource/register", Routes::bind(&ResAggregation_MGR::add, this));
  Routes::Get(router, "/resource/query/:id", Routes::bind(&ResAggregation_MGR::list, this));
  Routes::Delete(router, "/resource/delete/:id", Routes::bind(&ResAggregation_MGR::del, this));
  Routes::Post(router, "/resource/update", Routes::bind(&ResAggregation_MGR::update, this));
}

bool ResAggregation_MGR::add(const Rest::Request &req, Http::ResponseWriter res)
{
  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket, hvs::DatastoreType::couchbase, true);
  auto storage_res = parse_request(req);
  // broken request
  if (!storage_res)
  {
    res.send(Code::Bad_Request, "resource parse fail");
    return false;
  }

  auto cbd = static_cast<CouchbaseDatastore *>(dbPtr.get());
  auto [vp, err] = dbPtr->get(storage_res->key());
  if (!err)
  {
    res.send(Code::Bad_Request, "fail, resource already exist");
    return false;
  }
  err = cbd->insert(storage_res->key(), storage_res->json_value());
  usleep(100000); // may take 100ms to be effective
  if (!err)
  {
    res.send(Code::Accepted, "ok");
    return true;
  }
  else
  {
    res.send(Code::Bad_Request, "fail");
    return false;
  }
}

bool ResAggregation_MGR::list(const Rest::Request &req, Http::ResponseWriter res)
{
  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket, hvs::DatastoreType::couchbase, true);
  auto cbd = static_cast<CouchbaseDatastore *>(dbPtr.get());
  auto uuid = req.param(":id").as<std::string>();
  string reluuid = StorageResource::prefix() + uuid;
  char query[256];
  if (uuid == "all") //查询所有
  {
    snprintf(query, 256,
             "select assign_capacity,host_center_id,host_center_name,mgs_address,state, "
             "storage_src_id, storage_src_name, total_capacity"
             " from `%s` where META().id like '%s%%' order by "
             "META().id",
             bucket.c_str(), StorageResource::prefix().c_str());
  }
  else //查询id对应资源
  {
    snprintf(query, 256, "select * from `%s` where META().id == '%s'", bucket.c_str(), reluuid.c_str());
  }
  auto [iop_infos, err] = cbd->n1ql(string(query));
  if (!err)
  {
    vector<shared_ptr<StorageResource>> rstlists(iop_infos->size());
    for (int i = 0; i < iop_infos->size(); i++)
    {
      json_decode(iop_infos->at(i), rstlists[i]);
    }
    auto reslist_str = json_encode(rstlists);
    res.send(Code::Ok, move(reslist_str));
    return true;
  }
  else
  {
    res.send(Code::Bad_Request, "");
    return false;
  }
}

bool ResAggregation_MGR::del(const Rest::Request &req, Http::ResponseWriter res)
{
  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket, hvs::DatastoreType::couchbase, true);
  auto cbd = static_cast<CouchbaseDatastore *>(dbPtr.get());
  auto uuid = req.param(":id").as<std::string>();
  string reluuid = StorageResource::prefix() + uuid;
  char delstr[256];

  if (uuid == "all") //删除所有
  {
    snprintf(delstr, 256,
             "delete from `%s` where SUBSTR(META().id,0,%d) == '%s'",
             bucket.c_str(), StorageResource::prefix().length(),
             StorageResource::prefix().c_str());
  }
  else //删除id对应资源
  {
    snprintf(delstr, 256, "delete from `%s` where META().id == '%s'", bucket.c_str(), reluuid.c_str());
  }
  auto [iop_infos, err] = cbd->n1ql(string(delstr));
  if (!err)
  {
    res.send(Code::Ok, "ok");
    return true;
  }
  else
  {
    res.send(Code::Bad_Request, "fail");
    return false;
  }
}

bool ResAggregation_MGR::update(const Rest::Request &req, Http::ResponseWriter res)
{
  std::shared_ptr<Datastore> dbPtr = DatastoreFactory::create_datastore(
      bucket, hvs::DatastoreType::couchbase, true);
  auto storage_res = parse_request(req);
  // broken request
  if (!storage_res)
  {
    res.send(Code::Bad_Request, "resource parse fail");
    return false;
  }

  //--- auto cbd = static_cast<CouchbaseDatastore *>(dbPtr.get());
  auto [vp, err] = dbPtr->get(storage_res->key());
  if (err)
  {
    res.send(Code::Bad_Request, "fail, resource dose not exist");
    return false;
  }
  //dbPtr->set(storage_res->key(), storage_res->json_value());
  //err = cbd->insert(storage_res->key(), storage_res->json_value());
  usleep(100000); // may take 100ms to be effective
  // int error=1000;
  std::shared_ptr<hvs::StorageResource> storRe;

  json_decode(*vp, storRe);
  // res.send(Code::Accepted,"Debug:"+storRe->json_value());
  // return true;
  // storage_res->storage_src_id;   // 存储资源UUID
  // storage_res->storage_src_name; // 存储资源名称
  // storage_res->host_center_id;   // 存储资源所在超算中心UUID
  // storage_res->host_center_name; // 存储资源所在超算中心名称
  // storage_res->total_capacity;   // 存储资源空间容量大小
  // storage_res->assign_capacity;  // 已经分配空间容量大小
  // storage_res->mgs_address;      // 存储资源MGS地址
  // storage_res->state;            // 存储资源状态

  //检查是否修改如果修改存储资源的某个值，则进行改动
  storRe->storage_src_id = (storage_res->storage_src_id.c_str()[0] != '#') ? storage_res->storage_src_id : storRe->storage_src_id;
  storRe->storage_src_name = (storage_res->storage_src_name.c_str()[0] != '#') ? storage_res->storage_src_name : storRe->storage_src_name;
  storRe->host_center_id = (storage_res->host_center_id.c_str()[0] != '#') ? storage_res->host_center_id : storRe->host_center_id;
  storRe->host_center_name = (storage_res->host_center_name.c_str()[0] != '#') ? storage_res->host_center_name : storRe->host_center_name;
  storRe->total_capacity = (storage_res->total_capacity != -1) ? storage_res->total_capacity : storRe->total_capacity;
  storRe->assign_capacity = (storage_res->assign_capacity != -1) ? storage_res->assign_capacity : storRe->assign_capacity;
  storRe->mgs_address = (storage_res->mgs_address.c_str()[0] != '#') ? storage_res->mgs_address : storRe->mgs_address;
  storRe->state = (storage_res->state != -1) ? storage_res->state : storRe->state;

  if (!(dbPtr->set(storage_res->key(), storRe->json_value())))
  {
    res.send(Code::Accepted, "ok");
    return true;
  }
  else
  {
    res.send(Code::Bad_Request, "fail");
    return false;
  }
}

std::shared_ptr<StorageResource> ResAggregation_MGR::parse_request(
    const Rest::Request &req)
{
  auto cnt = req.body();
  try
  {
    auto ion = make_shared<StorageResource>();
    ion->deserialize(cnt);
    return ion;
  }
  catch (Expectation &e)
  {
    return nullptr;
  }
}