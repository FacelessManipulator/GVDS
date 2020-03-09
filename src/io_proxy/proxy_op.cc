/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:38:46
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:38:46
 */
#include <sys/stat.h>
#include <sys/xattr.h>

#include <utime.h>
#include "context.h"
#include "io_proxy/fd_mgr.h"
#include "io_proxy/io_proxy.h"
#include "io_proxy/proxy_op.h"
#include "msg/stat_demo.h"

using namespace hvs;
using namespace gvds;
using namespace std;
namespace fs = std::experimental::filesystem;
using gvds::Attr;
using gvds::OpReply;
using gvds::OpRequest;
using gvds::OpType;

namespace hvs {

void ProxyOP::prepare_op(std::shared_ptr<OP> op) {
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
void ProxyOP::do_op(std::shared_ptr<OP> op, boost::function0<void> callback) {
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

void ProxyOP::do_op(gvds::OpRequest& request, gvds::OpReply& reply,
                    boost::function0<void> callback) {
  std::error_code ec;
  reply.set_id(request.id());
  reply.set_err_code(0);
  switch (request.type()) {
    // metadata ops:
    case OpType::getattr: {
      auto attr = new Attr();
      int err = get_attr(data_path / request.filepath(), *attr);
      if (err != 0) {
        reply.set_err_code(err);
      } else {
        reply.set_allocated_attr(attr);
        reply.set_err_code(0);
      }
      break;
    }
    case OpType::rename: {
      fs::rename(data_path / request.filepath(), data_path / request.newpath(),
                 ec);
      reply.set_err_code(-ec.value());
      break;
    }
    case OpType::utimens: {
      struct timespec ts[2];
      // TODO: for convenience I reuse size to preset the time vector. This is a
      // bad habit!
      ts[1].tv_sec =
          request.io_param().size();  // times[0] is access time which we don't
                                      // care if it's correct or not
      ts[1].tv_nsec =
          request.io_param()
              .offset();  // times[1] is modification time which we care about
      ts[0].tv_sec = request.io_param().size();
      ts[0].tv_nsec = request.io_param().offset();

      int err = ::utimensat(0, (data_path / request.filepath()).c_str(), ts,
                            AT_SYMLINK_NOFOLLOW);
      if (err == -1) reply.set_err_code(-errno);
      break;
    }
    case OpType::chmod: {
      fs::perms perm = static_cast<fs::perms>(request.mode()) & fs::perms::mask;
      fs::permissions(data_path / request.filepath(), perm, ec);
      reply.set_err_code(-ec.value());
      break;
    }
    case OpType::truncate: {
      fs::resize_file(data_path / request.filepath(), request.size(), ec);
      reply.set_err_code(-ec.value());
      break;
    }
    case OpType::open: {
      // open should not have a effect!
      // cause I reuse the file handler so the opened file fs should support
      // read and write
      int fd = iop->fdm.open(data_path / request.filepath(),
                             request.mode() | O_RDWR, 0644);
      reply.set_err_code(fd);
      break;
    }
    case OpType::release: {
      int err = iop->fdm.close(data_path / request.filepath());
      reply.set_err_code(err);
      break;
    }

    case OpType::create: {
      int err = iop->fdm.create(data_path / request.filepath(), request.mode());
      reply.set_err_code(err);
      break;
    }
    case OpType::fallocate: {
      /* O_CREAT makes sense only for the default fallocate(2) behavior
       * when mode is no specified and new space is allocated */
      // TODO: but currently we add O_CREAT for convenience
      int fd =
          iop->fdm.open(data_path / request.filepath(), O_CREAT | O_RDWR, 0666);
      if (fd > 0) {
        int err = ::fallocate(fd, request.mode(), request.io_param().offset(),
                              request.io_param().size());
        if (err == -1) reply.set_err_code(-errno);
      } else {
        reply.set_err_code(-EBADF);
      }
      break;
    }
    case OpType::mkdir: {
      fs::create_directory(data_path / request.filepath(), ec);
      // don't support set the permission of a directory
      reply.set_err_code(-ec.value());
      break;
    }
    case OpType::rmdir: {
      // rmdir deletes a directory which must be empty
      fs::remove(data_path / request.filepath(), ec);
      reply.set_err_code(-ec.value());
      break;
    }
    case OpType::readdir: {
      DIR* dp = ::opendir((data_path / request.filepath()).c_str());
      struct dirent* de;
      if (dp == nullptr) {
        ::closedir(dp);
        reply.set_err_code(-errno);
        break;
      }
      de = ::readdir(dp);
      if (de == nullptr) {
        ::closedir(dp);
        reply.set_err_code(-errno);
        break;
      }
      do {
        // dir entry contains inode, offset, length of record, type and name
        string filename(de->d_name);
        auto attr = new Attr();
        // the url of dest entry is { Data Directory in IOP Server / Directory
        // relative path / entry name }
        int err = get_attr(data_path / request.filepath() / filename, *attr);
        if (err != 0) {
          // this is rare in readdir
          delete attr;
          dout(-1) << "cannot found [" << request.filepath() << "/" << filename
                   << "]" << dendl;
          continue;
        } else {
          reply.mutable_entry_names()->Add(std::move(filename));
          reply.mutable_entries()->AddAllocated(attr);
        }
        // the ownership of filename and attr ends here
      } while ((de = ::readdir(dp)) != nullptr);
      closedir(dp);
      break;
    }

    // data operations
    case OpType::read: {
      int fd = iop->fdm.open(data_path / request.filepath(), O_RDWR, 0655);
      if (fd < 0) {
        reply.set_err_code(fd);
        break;
      }
      // after c++11 string provide contiguous storage so I can memcpy raw data
      // to string's data area
      string* buf = new string;
      buf->resize(request.io_param().size());
      ssize_t ret = pread(fd, buf->data(), request.io_param().size(),
                          request.io_param().offset());
      if (ret == -1) {
        delete buf;
        reply.set_err_code(-errno);
      } else {
        reply.set_allocated_data(buf);
        reply.set_err_code(ret);
      }
      // the ownership of buf ends up here
      break;
    }
    case OpType::write: {
      int fd = iop->fdm.open(data_path / request.filepath(), O_RDWR, 0655);
      if (fd < 0) {
        reply.set_err_code(fd);
        break;
      }
      ssize_t ret =
          pwrite(fd, request.data().c_str(), request.io_param().size(),
                 request.io_param().offset());
      if (ret == -1) {
        reply.set_err_code(-errno);
      } else {
        reply.set_err_code(ret);
      }
      break;
    }
    case OpType::unlink: {
      int ret = iop->fdm.remove(data_path / request.filepath());
      reply.set_err_code(ret);
      break;
    }

    // symlink operations, currently not supported
    case OpType::readlink:
    case OpType::symlink:
    case OpType::link: {
      break;
    }

    // TODO: not implemented yet!
    case OpType::setxattr: {
        if (request.xattr_name() == "user.replicate.create") {
            int err = iop->repm.create_replicated_space(request.filepath(), request.data());
            reply.set_err_code(err);
        } else {
            int err = ::setxattr((data_path/request.filepath()).c_str(), request.xattr_name().c_str(), request.data().c_str(),
                                 request.size(), request.mode());
            if (err == -1) {
                reply.set_err_code(-errno);
            }
        }
        break;
    }
    case OpType::getxattr: {
        string* buf = new string();
        buf->resize(request.size());
        int err = ::getxattr((data_path/request.filepath()).c_str(), request.xattr_name().c_str(), buf->data(), request.size());
        if (err == -1) {
            reply.set_err_code(-errno);
        } else {
            reply.set_err_code(err);
            reply.set_allocated_data(buf);
        }
        break;
    }
    case OpType::listxattr: {
        string* buf = new string();
        buf->resize(request.size());
        int err = ::listxattr((data_path/request.filepath()).c_str(), buf->data(), request.size());
        if (err == -1) {
            reply.set_err_code(-errno);
        } else {
            reply.set_err_code(err);
            reply.set_allocated_data(buf);
        }
        break;
    }
    case OpType::removexattr: {
        int err = ::removexattr((data_path/request.filepath()).c_str(), request.xattr_name().c_str());
        if (err == -1) {
            reply.set_err_code(-errno);
        } else {
            reply.set_err_code(err);
        }
        break;
    }
    case OpType::flock:
    case OpType::opendir:
    case OpType::releasedir:
    case OpType::lock: {
      break;
    }

    // TODO: no effect, the ops below should be handled by client
    case OpType::fsync:
    case OpType::flush:
    case OpType::chown:
    case OpType::write_buf:
    case OpType::read_buf:
    case OpType::fsyncdir:
    case OpType::access: {
      break;
    }

    case OpType::bmap:
    case OpType::ioctl:
    case OpType::poll: {
      break;
    }
      default:
          break;
  }
  iop->repm.handle_replica_async(move(request), reply);
  callback();
}

int ProxyOP::get_attr(const std::experimental::filesystem::path& path,
                      gvds::Attr& attr) {
  struct stat st;
  int err = ::stat(path.c_str(), &st);
  if (err == -1) {
    return -errno;
  } else {
    attr.set_ctime(st.st_ctim.tv_sec);
    switch (st.st_mode & S_IFMT) {
      case S_IFDIR:
        attr.set_type(Attr::Directory);
        break;
      case S_IFREG:
        attr.set_type(Attr::RegularFile);
        break;
      case S_IFLNK:
        attr.set_type(Attr::Link);
        break;
      default:
        break;
    }
    attr.set_size(st.st_size);
    attr.set_mtime(st.st_mtim.tv_sec);
    attr.set_permission(st.st_mode);
    return 0;
  }
}

void ProxyOP::async_do_op(std::shared_ptr<OP> op,
                          boost::function0<void> callback) {
  // not implemented yet!
  callback();
}

int ProxyOP::ioproxy_do_metadata_op(IOProxyMetadataOP* op) {
  switch (op->operation) {
    case IOProxyMetadataOP::stat: {
      func_sync_io.sstat(op->path, op);
      break;
    }
    case IOProxyMetadataOP::open: {
      op->error_code = iop->fdm.open(op->path, op->open_flags | O_RDWR,
                                     0644);  // 默认文件权限为 rw-r--r--
      break;
    }
    case IOProxyMetadataOP::close: {
      op->error_code = iop->fdm.close(op->path);
      break;
    }
    case IOProxyMetadataOP::flush: {
      op->error_code = iop->fdm.flush(op->path);
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

int ProxyOP::ioproxy_do_data_op(IOProxyDataOP* op) {
  switch (op->operation) {
    case IOProxyDataOP::read: {
      op->size = func_sync_io.sread(op->path.c_str(), op->obuf, op->size,
                                    op->offset, op);
      break;
    }
    case IOProxyDataOP::write: {
      func_sync_io.swrite(op->path.c_str(), op->ibuf, op->size, op->offset, op);
      break;
    }
    case IOProxyDataOP::truncate: {
      op->error_code = func_sync_io.struncate(
          op->path.c_str(),
          op->offset);  // 第二批代码，error_code 由同步IO返回值修改
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
      op->error_code =
          func_sync_io.slink(op->path.c_str(), op->newpath.c_str());
      break;
    }
    case IOProxyDataOP::symlink: {
      op->error_code =
          func_sync_io.ssymlink(op->path.c_str(), op->newpath.c_str());
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