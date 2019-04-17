#pragma once
#include <memory>
#include <boost/function.hpp>
#include "msg/op.h"

namespace hvs {
void async_do_op(std::shared_ptr<OP> op, boost::function0<void> callback);

int ioproxy_do_metadata_op(IOProxyMetadataOP* op);
}