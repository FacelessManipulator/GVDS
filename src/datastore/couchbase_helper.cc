#include "datastore/couchbase_helper.h"
#include <cerrno>
#include <memory>
#include "common/debug.h"
#include "libcouchbase/couchbase++.h"
#include "libcouchbase/couchbase++/query.h"
// #include "config.h"

void hvs::N1qlResponse::deserialize_impl() {
  std::string status_s;
  std::map<std::string, unsigned> metrics;
  get("requestID", id);
  get("errors", errors);
  get("status", status_s);
  get("metrics", metrics);
  resultCount = metrics["resultCount"];
  resultSize = metrics["resultSize"];
  errorCount = metrics["errorCount"];
  if (status_s == "fatal") {
    status = -EINVAL;
  } else if (status_s == "success") {
    status = 0;
  }
}

int hvs::CouchbaseDatastore::init() { return _connect(name); }

int hvs::CouchbaseDatastore::_connect(const std::string& bucket) {
  // format the connection string
  // TODO: options below should get from config module
  const std::string server_address = "192.168.10.235";
  const std::string username = "dev";
  const std::string passwd = "buaaica";

  std::string connstr_f = "couchbase://%s/%s";
  // calculate the size of connection string before concentrate
  size_t nsize = connstr_f.length() + server_address.length() + bucket.length();
  std::unique_ptr<char[]> connstr_p(new char[nsize]);
  snprintf(connstr_p.get(), nsize, connstr_f.c_str(), server_address.c_str(),
           bucket.c_str());
  dout(20) << "DEBUG: format couchbase connection string: " << connstr_p.get()
           << dendl;

  client = std::shared_ptr<Couchbase::Client>(
      new Couchbase::Client(connstr_p.get(), passwd, username));
  Couchbase::Status rc = client->connect();
  if (!rc.success()) {
    dout(-1) << "ERROR: couldn't connect to couchbase. " << rc.description()
             << dendl;
  }
  // assert(rc.success());
  return rc.errcode();
}

int hvs::CouchbaseDatastore::_set(const std::string& key,
                                  const std::string& doc) {
  Couchbase::StoreResponse rs = client->upsert(key, doc);
  if (!rs.status().success()) {
    dout(5) << "ERROR: Couchbase helper couldn't set kv pair " << key.c_str()
            << "-" << doc.c_str() << ", Reason: " << rs.status().description()
            << dendl;
  }
  return rs.status().errcode();
}

int hvs::CouchbaseDatastore::_set(const std::string& key,
                                  const std::string& path,
                                  const std::string& subdoc) {
  Couchbase::SubdocResponse rs = client->upsert_sub(key, path, subdoc);
  if (!rs.status().success()) {
    dout(5) << "ERROR: Couchbase helper couldn't get kv pair " << key.c_str()
            << ", Reason: " << rs.status().description() << dendl;
  }
  return rs.status().errcode();
}

std::tuple<std::shared_ptr<std::string>, int> hvs::CouchbaseDatastore::_get(
    const std::string& key) {
  Couchbase::GetResponse rs = client->get(key);
  std::shared_ptr<std::string> content;
  if (rs.status().success()) {
    content.reset(new std::string(rs.value().data()));
  } else {
    dout(5) << "ERROR: Couchbase helper couldn't get kv pair " << key.c_str()
            << ", Reason: " << rs.status().description() << dendl;
  }
  return {content, rs.status().errcode()};
}

std::tuple<std::shared_ptr<std::string>, int> hvs::CouchbaseDatastore::_get(
    const std::string& key, const std::string& path) {
  Couchbase::SubdocResponse rs = client->get_sub(key, path);
  std::shared_ptr<std::string> content;
  if (rs.status().success()) {
    content.reset(new std::string(rs.value().data()));
  } else {
    dout(5) << "ERROR: Couchbase helper couldn't get kv pair " << key.c_str()
            << ", Reason: " << rs.status().description() << dendl;
  }
  return {content, rs.status().errcode()};
}

int hvs::CouchbaseDatastore::_remove(const std::string& key) {
  Couchbase::RemoveResponse rs = client->remove(key);
  if (!rs.status().success()) {
    dout(5) << "ERROR: Couchbase helper couldn't remove kv pair " << key.c_str()
            << ", Reason: " << rs.status().description() << dendl;
  }
  return rs.status().errcode();
}

std::tuple<std::shared_ptr<std::vector<std::string>>, int>
hvs::CouchbaseDatastore::_n1ql(const std::string& query) {
  Couchbase::QueryCommand qcmd(query);
  Couchbase::Status status;
  N1qlResponse res;
  int errcode = 0;
  Couchbase::Query q(*client, qcmd, status);
  auto content = std::make_shared<std::vector<std::string>>();
  client->wait();
  if (status && q.meta().status()) {
    for (auto row : q) {
      content->push_back(row.json());
    }
  } else if (!status) {
    errcode = status.errcode();
    dout(5) << "ERROR: Couchbase helper couldn't execute N1QL: " << query
            << ", Reason: " << status.description() << dendl;
  } else {
    res.deserialize(q.meta().body().data());
    errcode = res.status;
    dout(5) << "ERROR: Couchbase helper N1QL ERROR: " << query << ", Reason: "
            << (res.errors.size() > 0 ? res.errors[0].msg : "Unknown") << dendl;
  }
  return {content, errcode};
}