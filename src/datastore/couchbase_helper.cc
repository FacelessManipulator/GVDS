#include "couchbase_helper.h"
#include "libcouchbase/couchbase++.h"
#include <memory>
// #include "config.h"

int hvs::CouchbaseDatastore::init() {
    return _connect(name);
}

int hvs::CouchbaseDatastore::set(DatastoreKey key, DatastoreValuePtr value) {
    return _set(key, *value);
}

int hvs::CouchbaseDatastore::set(DatastoreKey key, DatastoreValue value) {
    return _set(key, value);
}

hvs::DatastoreValuePtr hvs::CouchbaseDatastore::get(DatastoreKey key) {
    return std::make_shared<std::string>(_get(key));
}

int hvs::CouchbaseDatastore::remove(DatastoreKey key) {
    return _remove(key);
}

int hvs::CouchbaseDatastore::_connect(const std::string& bucket) {
    // format the connection string
    // TODO: options below should get from config module
    const std::string server_address = "localhost";
    const std::string username = "admin";
    const std::string passwd = "123456";

    std::string connstr_f = "couchbase://%s/%s";
    // calculate the size of connection string before concentrate
    size_t nsize = connstr_f.length() + server_address.length() + bucket.length();
    std::unique_ptr<char[]> connstr_p(new char[nsize]);
    snprintf(connstr_p.get(), nsize, connstr_f.c_str(), server_address.c_str(), bucket.c_str());
    HVS_LOG("DEBUG: format couchbase connection string: %s\n", connstr_p.get());
    
    client = std::shared_ptr<Couchbase::Client>(new Couchbase::Client(connstr_p.get(), passwd, username));
    Couchbase::Status rc = client->connect();
    if(!rc.success()) {
        HVS_LOG("ERROR: couldn't connect to couchbase. %s\n", rc.description());
    }
    // assert(rc.success());
    return rc.errcode();
}

int hvs::CouchbaseDatastore::_set(const std::string& key, const std::string& doc) {
    Couchbase::StoreResponse rs = client->upsert(key, doc);
    if(!rs.status().success()) {
        HVS_LOG("ERROR: Couchbase helper couldn't set kv pair %s-%s, Reason:%s\n", key.c_str(), doc.c_str(),
            rs.status().description());
    }
    return rs.status().errcode();
}

int hvs::CouchbaseDatastore::_set(const std::string& key, const std::string& path, const std::string& subdoc) {
    assert(false); // SUBDOC feature not implemented yet
}

std::string hvs::CouchbaseDatastore::_get(const std::string& key) {
    Couchbase::GetResponse rs = client->get(key);
    if(!rs.status().success()) {
        HVS_LOG("ERROR: Couchbase helper couldn't get kv pair %s, Reason: %s\n", key.c_str(), rs.status().description());
    }
    return rs.value().to_string();
}

int hvs::CouchbaseDatastore::_remove(const std::string& key) {
    Couchbase::RemoveResponse rs = client->remove(key);
    if(!rs.status().success()) {
        HVS_LOG("ERROR: Couchbase helper couldn't remove kv pair %s, Reason:%s\n", key.c_str(),
            rs.status().description());
    }
    return rs.status().errcode();
}