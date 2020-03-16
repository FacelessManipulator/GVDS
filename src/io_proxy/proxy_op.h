/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:39:14
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:39:14
 */
#pragma once
#include <boost/function.hpp>
#include <experimental/filesystem>
#include <memory>
#include "msg/op.h"
#include "op.pb.h"
#include "sync_io.h"

namespace gvds {
class IOProxy;
class ProxyOP {
 public:
  ProxyOP(IOProxy* ioproxy) : iop(ioproxy), func_sync_io(ioproxy) {}
  void prepare_op(std::shared_ptr<OP> op);
  void async_do_op(std::shared_ptr<OP> op, boost::function0<void> callback);
  void do_op(std::shared_ptr<OP> op, boost::function0<void> callback);
  void do_op(gvds::OpRequest& request, gvds::OpReply& reply,
             boost::function0<void> callback);

  int ioproxy_do_metadata_op(IOProxyMetadataOP* op);
  int ioproxy_do_data_op(IOProxyDataOP* op);
  int get_attr(const std::experimental::filesystem::path& path,
               gvds::Attr& attr);

 public:
  IOProxy* iop;
  std::experimental::filesystem::path data_path;

 public:
  sync_io func_sync_io;
};
}  // namespace gvds