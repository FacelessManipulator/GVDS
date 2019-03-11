#include "datastore.h"
#include "couchbase_helper.h"

using namespace hvs;
DatastorePtr DatastoreFactory::create_datastore(std::string name,
                                                DatastoreType type) {
  std::shared_ptr<Datastore> dsp = nullptr;
  switch (type) {
    case couchbase:
      dsp = std::make_shared<CouchbaseDatastore>(CouchbaseDatastore(name));
      HVS_LOG("DatastoreFactory: create %s instance\n",
              dsp->get_typename().c_str());
      break;
    case memcached:
      break;
    default:
      break;
  }
  return dsp;
}