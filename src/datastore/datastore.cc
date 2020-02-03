#include "datastore.h"
#include <sys/syscall.h>
#include "context.h"
#include "datastore/couchbase_helper.h"

using namespace hvs;

std::map<std::pair<long int, std::string>, std::shared_ptr<Datastore>>
    DatastoreFactory::_reuse_map = {};
std::shared_ptr<Datastore> DatastoreFactory::create_datastore(
    std::string name, DatastoreType type, bool reuse) {
  std::shared_ptr<Datastore> dsp = nullptr;
  long int tid;
  do {
    if (!reuse) break;
    tid = syscall(SYS_gettid);
    auto iter = _reuse_map.find({tid, name});
    if (iter == _reuse_map.end()) break;
    // The client may aleady released by other thread, factory don't keep the
    // ownership or reference.
    if (iter->second.get()) {
      // found an valid client with same bucket and thread id.
      dout(10) << "DatastoreFactory: reuse datastore instance "
               <<  name << ": " << tid << dendl;
      return iter->second;
    } else {
      _reuse_map.erase(iter);
      break;
    }
  } while (0);

  switch (type) {
    case couchbase:
      dsp = std::make_shared<CouchbaseDatastore>(name);
      dout(10) << "DatastoreFactory: create datastore instance "
               << dsp->get_typename() << dendl;
      break;
    case memcached:
      break;
    default:
      break;
  }

  if (dsp.get() && dsp->init() == 0) {
    if (reuse) {
      _reuse_map[{tid, name}] = dsp;
    }
    return dsp;
  } else
    return nullptr;
}