#include "io_proxy/proxy_op.h"
#include "context.h"
using namespace hvs;

namespace hvs{

// TODO: this is an sync example. we should transfer it to aync
void async_do_op(std::shared_ptr<OP> op, boost::function0<void> callback) {
  switch (op->type) {
    case IO_PROXY_METADATA:
    {
      IOProxyMetadataOP* metaop = static_cast<IOProxyMetadataOP*>(op.get());
      ioproxy_do_metadata_op(metaop);
      break;
    }
    case IO_PROXY_DATA:
      break;
    default:
      break;
  }
  callback();
}

int ioproxy_do_metadata_op(IOProxyMetadataOP* op) {
//  dout(5) << "async do op: " << op->id << dendl;
  if(op->retval) {
    // copy the result value
    strcpy((char *)op->retval, "op finished");
  }
}
}