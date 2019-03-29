#include "datastore.h"
#include "datastore/couchbase_helper.h"
#include "common/debug.h"

using namespace hvs;
std::shared_ptr<Datastore> DatastoreFactory::create_datastore(std::string name,
                                                DatastoreType type) {
  std::shared_ptr<Datastore> dsp = nullptr;
  switch (type) {
    case couchbase:
      dsp = std::make_shared<CouchbaseDatastore>(CouchbaseDatastore(name));
      dout(10) << "DatastoreFactory: create datastoree instance " << dsp->get_typename() << dendl;
      break;
    case memcached:
      break;
    default:
      break;
  }
  return dsp;
}