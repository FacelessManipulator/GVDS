#pragma once
#include <cassert>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include "hvsdef.h"

namespace hvs {
class Datastore;
enum DatastoreType {
  null,
  couchbase,
  memcached,
};

// Abstract class of real datastore, contains common function and interface
// to be implemeted
// Example:
//      Datastore* ds = (Datastore *) couchbase;
//      ds->store("KEY", "VALUE");
//      delete ds;
class Datastore {
 public:
  std::string name;

 public:
  Datastore(const std::string& name) : name(name) {}
  Datastore() : Datastore("default") {}
  virtual std::string get_typename() { return "NULL"; }
  virtual int init() { return 0; };
  virtual int set(const std::string& key,
                  std::string& value){};  // not implement
  virtual int set(const std::string& key, const std::string& subpath,
                  std::string& value){};
  virtual std::tuple<std::shared_ptr<std::string>, int> get(
      const std::string& key){};
  virtual std::tuple<std::shared_ptr<std::string>, int> get(
      const std::string& key, const std::string& subpath){};
  virtual int remove(const std::string& key){};
};

class DatastoreFactory {
 public:
  static std::shared_ptr<Datastore> create_datastore(
      std::string datastore_name, DatastoreType datastore_type = couchbase,
      bool reuse = false);

 private:
  static std::map<std::pair<long int, std::string>, std::shared_ptr<Datastore>>
      _reuse_map;
};
}  // namespace hvs