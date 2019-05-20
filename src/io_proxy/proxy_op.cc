#include "io_proxy/proxy_op.h"
#include "context.h"
#include "msg/stat_demo.h"

using namespace hvs;

namespace hvs {

void prepare_op(std::shared_ptr<OP> op) {
  if (!op->should_prepare) return;
  switch (op->type) {
    case IO_PROXY_METADATA: {
      static_cast<IOProxyMetadataOP*>(op.get())->buf =
          (struct stat*)malloc(sizeof(struct stat));
      memset(static_cast<IOProxyMetadataOP*>(op.get())->buf, 0,
             sizeof(struct stat));
      break;
    }
    case IO_PROXY_DATA: {
      IOProxyDataOP* dataop = static_cast<IOProxyDataOP*>(op.get());
      dataop->obuf = (char*)malloc(dataop->size);
      break;
    }
    default:
      break;
  }
}

// TODO: this is an sync example. we should transfer it to aync
void do_op(std::shared_ptr<OP> op, boost::function0<void> callback) {
  switch (op->type) {
    case IO_PROXY_METADATA: {
      IOProxyMetadataOP* metaop = static_cast<IOProxyMetadataOP*>(op.get());
      ioproxy_do_metadata_op(metaop);
      break;
    }
    case IO_PROXY_DATA: {
      IOProxyDataOP* dataop = static_cast<IOProxyDataOP*>(op.get());
      ioproxy_do_data_op(dataop);
      break;
    }
    default:
      break;
  }
  callback();
}

void async_do_op(std::shared_ptr<OP> op, boost::function0<void> callback) {
  // not implemented yet!
  callback();
}

int ioproxy_do_metadata_op(IOProxyMetadataOP* op) {
  sync_io func_sync_io;
  switch (op->operation) {
    case IOProxyMetadataOP::stat: {
      dout(20) << "IOProxy: handle stat ["<<op->path<<"]" <<dendl;
      func_sync_io.sstat(op->path, op);
      break;
    }
    case IOProxyMetadataOP::open: {
      break;
    }
    case IOProxyMetadataOP::readdir: {
      func_sync_io.sreaddir(op->path, op);
      break;
    }
    case IOProxyMetadataOP::rename: {
      op->error_code = func_sync_io.srename(op->path, op->newpath);
      break;
    }
    case IOProxyMetadataOP::access: {
      op->error_code = func_sync_io.saccess(op->path, op->mode);
      break;
    }
    case IOProxyMetadataOP::utimes: {
      std::cout << "op.id: " << op->id << std::endl;
      op->error_code = func_sync_io.sutimes(op->path, op->sec0n, op->sec0s,
                                            op->sec1n, op->sec1s);
      break;
    }
    case IOProxyMetadataOP::chmod: {
      op->error_code =
          func_sync_io.schmod(op->path, static_cast<mode_t>(op->mode));
      break;
    }
    case IOProxyMetadataOP::chown: {
      op->error_code = func_sync_io.schown(op->path, op->uid, op->gid);
      break;
    }
    default:
      break;
  }
  return op->error_code;
}

int ioproxy_do_data_op(IOProxyDataOP* op) {
  sync_io func_sync_io;
  switch (op->operation) {
    case IOProxyDataOP::read: {
      func_sync_io.sread(op->path.c_str(), op->obuf, op->size, op->offset, op);
      break;
    }
    case IOProxyDataOP::write: {
      func_sync_io.swrite(op->path.c_str(), op->ibuf, op->size, op->offset, op);
      break;
    }
    case IOProxyDataOP::truncate: {
      op->error_code = func_sync_io.struncate(
          op->path.c_str(), op->offset);  // 第二批代码，error_code 由同步IO返回值修改
      break;
    }
    case IOProxyDataOP::mkdir: {
      op->error_code = func_sync_io.smkdir(op->path.c_str(), op->mode);
      break;
    }
    case IOProxyDataOP::rmdir: {
      op->error_code = func_sync_io.srmdir(op->path.c_str());
      break;
    }
    case IOProxyDataOP::create: {
      op->error_code = func_sync_io.screate(op->path.c_str(), op->mode);
      break;
    }
    case IOProxyDataOP::unlink: {
      op->error_code = func_sync_io.sunlink(op->path.c_str());
      break;
    }
    case IOProxyDataOP::link: {
      op->error_code = func_sync_io.slink(op->path.c_str(), op->newpath.c_str());
      break;
    }
    case IOProxyDataOP::symlink: {
      op->error_code = func_sync_io.ssymlink(op->path.c_str(), op->newpath.c_str());
      break;
    }
    case IOProxyDataOP::readlink: {
      op->error_code = func_sync_io.sreadlink(op->path.c_str(), op);
      break;
    }
    default:
      break;
  }
  return op->error_code;
}
}  // namespace hvs