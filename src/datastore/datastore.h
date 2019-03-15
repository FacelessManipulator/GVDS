#pragma once
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

namespace hvs {
class Datastore;

using DatastorePtr = std::shared_ptr<Datastore>;
using DatastoreKey = std::string;
using DatastoreValue = std::string;
using DatastoreValuePtr = std::shared_ptr<DatastoreValue>;
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
  Datastore(std::string name) : name(name) {}
  Datastore() : Datastore("default") {}
  virtual std::string get_typename() { return "NULL"; }
  virtual int init(){};
  virtual int set(DatastoreKey key,
                  DatastoreValuePtr value){};                 // not implement
  virtual int set(DatastoreKey key, DatastoreValue value){};  // not implement
  virtual DatastoreValuePtr get(DatastoreKey key){};
  virtual int remove(DatastoreKey key){};
};

class DatastoreFactory {
 public:
  static DatastorePtr create_datastore(
      std::string datastore_name, DatastoreType datastore_type = couchbase);
};
}  // namespace hvs