#pragma once
#include <memory>
#include <boost/function.hpp>
#include "msg/op.h"
#include "sync_io.h"

namespace hvs {
void prepare_op(std::shared_ptr<OP> op);
void async_do_op(std::shared_ptr<OP> op, boost::function0<void> callback);
void do_op(std::shared_ptr<OP> op, boost::function0<void> callback);

int ioproxy_do_metadata_op(IOProxyMetadataOP* op);
int ioproxy_do_data_op(IOProxyDataOP* op);
}