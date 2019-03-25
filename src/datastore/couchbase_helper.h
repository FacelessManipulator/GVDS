#pragma once

#include <string>
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
  DatastoreValue get(const DatastoreKey& key, const std::string& subpath) override;
  int remove(const DatastoreKey& key) override;
  int init() override;

 private:
  int _connect(const std::string& bucket);
  int _set(const std::string& key, const std::string& doc);
  int _set(const std::string& key, const std::string& path,
           const std::string& subdoc);
  std::string _get(const std::string& key);
  std::string _get(const std::string& key, const std::string& path);
  int _remove(const std::string& key);

 private:
  std::shared_ptr<Couchbase::Client> client;
};
}  // namespace hvs