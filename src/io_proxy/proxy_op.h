#pragma once
#include <memory>
#include <boost/function.hpp>
#include "msg/op.h"
#include "sync_io.h"

namespace hvs {
    class IOProxy;
    class ProxyOP {
    public:
        ProxyOP(IOProxy* ioproxy) :iop(ioproxy), func_sync_io(ioproxy) {}
        void prepare_op(std::shared_ptr<OP> op);
        void async_do_op(std::shared_ptr<OP> op, boost::function0<void> callback);
        void do_op(std::shared_ptr<OP> op, boost::function0<void> callback);

        int ioproxy_do_metadata_op(IOProxyMetadataOP* op);
        int ioproxy_do_data_op(IOProxyDataOP* op);

    public:
        IOProxy* iop;

    public:
        sync_io func_sync_io;
    };
}