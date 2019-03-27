#pragma once

#include <string>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "libcouchbase/couchbase++.h"

namespace hvs {
class CouchbaseDatastore : public hvs::Datastore {
 public:
  CouchbaseDatastore(std::string bucket_name) : Datastore(bucket_name) {}
  CouchbaseDatastore() = default;

 public:
  std::string get_typename() override { return "couchbase"; }
  int set(const DatastoreKey& key, DatastoreValue& value) override;
  DatastoreValue get(const DatastoreKey& key) override;
  DatastoreValue get(const DatastoreKey& key,
                     const std::string& subpath) override;
  int remove(const DatastoreKey& key) override;
  int init() override;

 public:
  std::string n1ql(const std::string& query) { return _n1ql(query); }

 private:
  int _connect(const std::string& bucket);
  int _set(const std::string& key, const std::string& doc);
  int _set(const std::string& key, const std::string& path,
           const std::string& subdoc);
  std::string _get(const std::string& key);
  std::string _get(const std::string& key, const std::string& path);
  int _remove(const std::string& key);
  std::string _n1ql(const std::string& query);

 private:
  std::shared_ptr<Couchbase::Client> client;
};

struct N1qlResponse : public hvs::JsonSerializer {
struct error :public JsonSerializer {
 public:
  unsigned code;
  std::string msg;

 protected:
  void deserialize_impl() override {
    get("code", code);
    get("msg", msg);
  };
  void serialize_impl() override { ASSERT(false, "not implemented"); };
};
 protected:
  void deserialize_impl() override;
  void serialize_impl() override { ASSERT(false, "not implemented"); };

 public:
  std::string id;
  std::vector<struct error> errors;
  int status;
  unsigned resultCount;
  unsigned resultSize;
  unsigned errorCount;
};

}  // namespace hvs