#pragma once

#include <string>
#include <tuple>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "libcouchbase/couchbase++.h"

namespace hvs {
struct N1qlResponse;
class CouchbaseDatastore : public hvs::Datastore {
 public:
  CouchbaseDatastore(const std::string& bucket_name)
      : Datastore(bucket_name), initilized(false) {}
  CouchbaseDatastore() : initilized(false){};

 public:
  std::string get_typename() override { return "couchbase"; }
  int set(const std::string& key, std::string& value) override {
    return _set(key, value);
  };
  int set(const std::string& key, const std::string& subpath,
          std::string& value) override {
    return _set(key, subpath, value);
  };
  std::tuple<std::shared_ptr<std::string>, int> get(
      const std::string& key) override {
    return _get(key);
  };
  std::tuple<std::shared_ptr<std::string>, int> get(
      const std::string& key, const std::string& subpath) override {
    return _get(key, subpath);
  };
  int remove(const std::string& key) override { return _remove(key); };
  int init() override;

 public:
  std::tuple<std::shared_ptr<std::vector<std::string>>, int> n1ql(
      const std::string& query) {
    return _n1ql(query);
  }

 private:
  int _connect(const std::string& bucket);
  int _set(const std::string& key, const std::string& doc);
  int _set(const std::string& key, const std::string& path,
           const std::string& subdoc);
  std::tuple<std::shared_ptr<std::string>, int> _get(const std::string& key);
  std::tuple<std::shared_ptr<std::string>, int> _get(const std::string& key,
                                                     const std::string& path);
  int _remove(const std::string& key);
  std::tuple<std::shared_ptr<std::vector<std::string>>, int> _n1ql(
      const std::string& query);

 private:
  std::shared_ptr<Couchbase::Client> client;
  bool initilized;
};

struct N1qlResponse : public hvs::JsonSerializer {
  struct error : public JsonSerializer {
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