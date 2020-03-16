/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-21 15:32:21
 * @Last Modified by: Hanjie,Zhou
 * @Last Modified time: 2020-02-21 15:45:52
 */

#include "replica_mgr.h"
#include <sys/xattr.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <stack>
#include <unistd.h>
#include "io_proxy.h"

using namespace std;
using namespace gvds;
using namespace boost::algorithm;
using namespace gvds;

void ReplicaMgr::start() {
  replicate_data_path = iop->data_path;
  mcid = stoi(iop->center_id);
  pthread_mutex_lock(&m_queue_mutex);
  m_stop = false;
  pthread_mutex_unlock(&m_queue_mutex);
  create("replica-manager-t");
}

void ReplicaMgr::stop() {
  if (is_started()) {
    pthread_mutex_lock(&m_queue_mutex);
    m_stop = true;
    pthread_cond_signal(&m_cond_flusher);
    pthread_cond_broadcast(&m_cond_loggers);
    pthread_mutex_unlock(&m_queue_mutex);
    join();
  }
}

void* ReplicaMgr::entry() {
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  while (!m_stop) {
    if (!m_new.empty()) {
      m_queue_mutex_holder = 0;
      pthread_mutex_unlock(&m_queue_mutex);
      flush();
      pthread_mutex_lock(&m_queue_mutex);
      m_queue_mutex_holder = pthread_self();
      continue;
    }

    pthread_cond_wait(&m_cond_flusher, &m_queue_mutex);
  }
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  flush();
  return NULL;
}

std::shared_ptr<OperatorClient> ReplicaMgr::get_operator(
    std::shared_ptr<IOProxyNode> node, int channel_id) {
  string channel_key(node->uuid);
  channel_key.append(to_string(channel_id));
  lock_guard<mutex> lock(mu_);
  auto oper_pair = operators.find(channel_key);

  if (oper_pair != operators.end()) {
    auto& oper = oper_pair->second;
    return oper;
  }
  // Just try create it
  // may cost a lot of moment
  char target_str[256];
  snprintf(target_str, 256, "%s:%u", node->ip.c_str(), node->rpc_port);
  auto oper = make_shared<OperatorClient>(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  operators.try_emplace(channel_key, oper);
  return oper;
}

void ReplicaMgr::handle_replica_async(OpRequest&& request, OpReply& reply) {
  // do nothing if op has no specific target
  if (request.filepath().empty()) return;
  // 1. at first check if the op should be replicated between ioproxy
  // 1.a if the request type is readonly, no need to replicate
  switch (request.type()) {
    case OpType::getattr:
    case OpType::access:
    case OpType::getxattr:
    case OpType::listxattr:
    case OpType::read:
    case OpType::flush:
    case OpType::release:
    case OpType::fsync:
    case OpType::write_buf:
    case OpType::read_buf:
    case OpType::opendir:
    case OpType::releasedir:
    case OpType::readdir:
    case OpType::fsyncdir:
    case OpType::readlink:
    case OpType::bmap:
    case OpType::ioctl:
    case OpType::poll:
      return;

      // open change the atime but we don't care atime, open may carry the flag
      // O_TRUNC or O_CREAT which is important
    case OpType::open:
    case OpType::rename:
    case OpType::utimens:
    case OpType::lock:
    case OpType::setxattr:
    case OpType::removexattr:
    case OpType::chmod:
    case OpType::chown:
    case OpType::truncate:
    case OpType::create:
    case OpType::fallocate:
    case OpType::flock:
    case OpType::mkdir:
    case OpType::rmdir:
    case OpType::unlink:
    case OpType::symlink:
    case OpType::link:
      // means is a replica request
      if (request.extra().size() != 0)
        return;
      break;

    // write modify the index and data, but for now I don't want to replicated
    // the data so I abandon the write data to save space
    case OpType::write:
      request.clear_data();
      break;

    // for the operation below, no need to queue the reqeust again cuz it's
    // already handled by specific function
    case OpType::rep_init_space:
      _handle_create_replicated_space(request, reply);
      return;
    case OpType::rep_sync_data:
      _handle_file_sync_data(request, reply);
      return;
      case OpType::rep_update_index:
          _handle_file_index_update(request, reply);
          return;
    default:
      break;
  }

  // 1.b for the above operations, the error reply means no side effect has
  // apply to the storage system
  if (reply.err_code() < 0) return;

  // 1.c if space has replicated center
  auto space_path = iop->data_path / *(fs::path(request.filepath()).begin());
  //  256 bytes buf is enough cuz cid is unique and not exceeds 2^8
  char cid[256];
  // the replicated center IDs composed by bytes and each bytes store a center
  // id which means the maximum id should not exceeds 2^8(256).
  size_t sz =
      ::getxattr(space_path.c_str(), _GVDS_SPACE_REPLICA_CENTER_IDS, cid, 256);
  // sz smaller means the current space has only itself as the replicated space,
  // no need to perform replicated
  if (sz == -1 || sz < 2) {
    return;
  }

  // 2. push the request into the replicated queue
  // since there the life of request transfer into the replicated queue
  auto req_ptr = make_shared<OpRequest>(request);
  req_ptr->set_extra(cid, sz);
  // the space replicated cid could be reused in replicating phase
  submit_replicated_request(req_ptr);
  dout(-1) << "queue replicated request: " << req_ptr->filepath() << " >"
           << OpType_Name(req_ptr->type()) << dendl;
}

void ReplicaMgr::submit_replicated_request(
    std::shared_ptr<OpRequest>& request) {
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();

  // wait for flush to catch up
  while (m_new.size() > m_max_new)
    pthread_cond_wait(&m_cond_loggers, &m_queue_mutex);

  m_new.push(request);

  pthread_cond_signal(&m_cond_flusher);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
}

int ReplicaMgr::create_replicated_space(const string& filepath,
                                        const std::string& cids_str) {
  // 1. split the cids from string "1,2,3" to vector [1, 2, 3]
  vector<string> tmp;
  split(tmp, cids_str, is_any_of(",/|"), token_compress_on);

  // check correctness and compress cids from human-readable format to array
  string cids;
  cids.push_back(mcid);
  for (auto& cid : tmp) {
    // may though exception if cid is not number, but it should be check before
    // set center id
    int id = stoi(cid);
    // cannot found the target cid in current ioproxy list, which means wrong
    // cid or no ioproxy exists in this center. Both two situation are not
    // suitable for replicated space.
    if (ioproxys.count(id) == 0) {
      return -ENXIO;
    }
    //        else if (cids.contains(id)) {
    //            // we should avoid duplicate ids, but it should be checked in
    //            client to relieve pressure for server
    //        }
    else {
      cids.push_back(id);
    }
  }

  // 2. set local space directory's xattr to store the replicated state
  auto space_path = iop->data_path / *(fs::path(filepath).begin());
  size_t sz = ::setxattr(space_path.c_str(), _GVDS_SPACE_REPLICA_CENTER_IDS,
                         cids.c_str(), cids.size(), XATTR_CREATE);
  if (sz == -1) {
    // TODO: should handle the situation that the space already has replicated
    // space in other center
    return -EMLINK;
  }

  // 3. do the initial replicated stuff such as replicating the exists directory
  // and files this phase should block the client utils metadata has been
  // replicated
  auto request = _replicate_space_request(*(fs::path(filepath).begin()), cids);
  for (int i = 1; i < cids.size(); i++) {
    uint32_t cid = cids[i];
    auto oper = get_operator(ioproxys[cid], 100);
    OpReply reply;
    auto status = oper->Submit(*request, reply);
    dout(-1) << "sent replicated request to target ioproxy "
             << ioproxys[cid]->cid << dendl;
    if (status.ok()) {
      dout(-1) << "replicated result from target ioproxy " << ioproxys[cid]->cid
               << " :" << reply.err_code() << dendl;
    }
  }
  return 0;
}

void ReplicaMgr::_handle_create_replicated_space(const OpRequest& request,
                                                 OpReply& reply) {
  int err;
  // 1. create the space dir
  auto rep_space_path = replicate_data_path / request.filepath();
  err = ::mkdir(rep_space_path.c_str(), 0775);
  if (err == -1 && errno != EEXIST) {
    reply.set_err_code(-errno);
    return;
  }
  // 2. set xattr for replicate space dir
  size_t sz =
      ::setxattr(rep_space_path.c_str(), _GVDS_SPACE_REPLICA_CENTER_IDS,
                 request.extra().c_str(), request.extra().size(), XATTR_CREATE);
  if (sz == -1) {
    reply.set_err_code(-errno);
    return;
  }

  // 3. create the directory and files
  // 3.a decode directory entries from protobuf
  OpReply dir_entries;
  dir_entries.ParseFromString(request.data());
  for (int cur_node = 0; cur_node < dir_entries.entry_names_size();
       cur_node++) {
    dout(-1) << "creating " << dir_entries.entry_names(cur_node) << dendl;
    auto& cur_attr = dir_entries.entries(cur_node);
    auto nodepath = (replicate_data_path / dir_entries.entry_names(cur_node));
    switch (cur_attr.type()) {
      case Attr_NodeType_Directory:
        ::mkdir(nodepath.c_str(), cur_attr.permission());
        break;
      case Attr_NodeType_RegularFile: {
        ::creat(nodepath.c_str(), cur_attr.permission());
        ::truncate(nodepath.c_str(), cur_attr.size());
        // set remote cid bitmap
        auto fh = iop->fdm.get_file_handler(nodepath);
        auto* bitmap = roaring_bitmap_from_range(0, cur_attr.size(), 1);
        // set the bitmap from center id, no need to delete bitmap after it
        fh->set_bitmap(request.extra()[0], bitmap);
        break;
      }
      default:
        break;
    }
    struct timespec ts[2];
    // TODO: for convenience I reuse size to preset the time vector. This is a
    // bad habit!
    ts[1].tv_sec = cur_attr.mtime();
    // care if it's correct or not
    ts[1].tv_nsec = 0;  // times[1] is modification time which we care about
    ts[0].tv_sec = cur_attr.ctime();
    ts[0].tv_nsec = 0;
    ::utimensat(0, nodepath.c_str(), ts, AT_SYMLINK_NOFOLLOW);
  }
  reply.set_err_code(0);
}

void ReplicaMgr::_handle_file_sync_data(const OpRequest& request, OpReply& reply) {
    auto filepath = replicate_data_path / request.filepath();
    uint64_t off = request.io_param().offset(), len = request.io_param().size();
    auto fd = iop->fdm.get_file_handler(filepath);
    auto data_ranges = fd->find_data_in_range(off, off+len);
  dout(-1) << "handle file"<< filepath <<" sync data at range [" << off << ","<<off+len <<"] data range size:" << data_ranges.size() <<dendl;

    string* buf = new string;
    uint64_t cursor = 0;
    buf->resize(len);
    for (auto [off1, off2] : data_ranges) {
        ssize_t ret = ::pread(fd->fd, buf->data() + cursor,off2-off1, off1);
        if (ret == -1) {
            delete buf;
            reply.set_err_code(-errno);
            return;
        } else {
            cursor += ret;
            auto param = reply.add_io_params();
            param->set_offset(off1);
            param->set_size(ret);
        }
    }
    // TODO: do resize to reduce the size of data, may cause bugs.
    buf->resize(cursor);
    reply.set_allocated_data(buf);
    reply.set_err_code(0);
}

void ReplicaMgr::_handle_file_index_update(const OpRequest& request,
                                           OpReply& reply) {
    auto filepath = replicate_data_path / request.filepath();
    uint64_t off = request.io_param().offset(), len = request.io_param().size();
    auto fd = iop->fdm.get_file_handler(filepath);
    uint8_t id = request.extra()[0];
    assert(request.type() == OpType::rep_update_index);
    // currently update index is only associated to write operation
    auto bitmap = fd->get_bitmap(id, true);
    // if remote center update [100, 300), than we should mark block {0, 1} as newest with closed range
    roaring_bitmap_add_range_closed(bitmap, off/_GVDS_REPLICATE_SYNC_BLK, (off+len)/_GVDS_REPLICATE_SYNC_BLK);
    // we are not punch the hole 2 block {0,1}, cuz local center still have the newest data
    fd->punch_hole_local(off, len);
    fd->flush_bitmap(id);
    reply.set_err_code(0);
    dout(-1) << "update local file index" << dendl;
}

int ReplicaMgr::sync_filedata(const string& path, uint64_t off, uint64_t len) {
    auto fh = iop->fdm.get_file_handler(iop->data_path / path);
    auto holes = fh->find_holes_in_range(off,off+len);
    if (holes.size() == 0) {
        // no holes in the specific range
        return 0;
    }

  auto space_path = iop->data_path / *(fs::path(path).begin());
  char cid[256];
  // the replicated center IDs composed by bytes and each bytes store a center
  // id which means the maximum id should not exceeds 2^8(256).
  size_t sz =
    ::getxattr(space_path.c_str(), _GVDS_SPACE_REPLICA_CENTER_IDS, cid, 256);
  if (sz == -1) {
    return 0;
  }
  string cids(cid, sz);
    OpRequest request;
    request.set_filepath(path);
    request.set_type(OpType::rep_sync_data);
    request.mutable_io_param()->set_offset(off);
    request.mutable_io_param()->set_size(len);
    for (int i = 0; i < cids.size(); i++) {
        uint32_t id = cids[i];
        if (id == mcid)
          continue;
        auto oper = get_operator(ioproxys[id], 100);
        OpReply reply;
        auto status = oper->Submit(request, reply);
        auto params = reply.io_params();
        int cursor = 0;
        for (int i = 0; i < params.size(); i++) {
            ::pwrite(fh->fd, reply.data().c_str()+cursor, params[i].size(), params[i].offset());
            cursor += params[i].size();
        }
        dout(-1) << "sent sync request to target ioproxy "
                 << ioproxys[id]->cid << dendl;
        if (status.ok()) {
            dout(-1) << "replicated sync result from target ioproxy [" << ioproxys[id]->cid
                     <<","<<ioproxys[id]->ip << ":" << ioproxys[id]->rpc_port << "] error " << reply.err_code() << dendl;
        }
    }
    return 0;
}

int ReplicaMgr::sync_filedata(const OpRequest& request) {
    // 1. at first check if the space is an replicated space, if not, do nothing


}

int ReplicaMgr::sync_unalign_page(const std::string& path, uint64_t off, uint64_t len) {
  // TODO: this is shit code but it miraculously worked.
  uint64_t pg_bg = off / _GVDS_BASE_FS_PAGE_SIZE;
  uint64_t pg_ed = (off+len) / _GVDS_BASE_FS_PAGE_SIZE;
  if ((off & _GVDS_BASE_FS_PAGE_MASK) != 0) {
    // means the first page is unaligned
    sync_filedata(path, pg_bg*_GVDS_BASE_FS_PAGE_SIZE, _GVDS_BASE_FS_PAGE_SIZE);
  }
  if ((off+len & _GVDS_BASE_FS_PAGE_MASK) != 0) {
    // means the last page is unaligned
    sync_filedata(path, pg_ed*_GVDS_BASE_FS_PAGE_SIZE, _GVDS_BASE_FS_PAGE_SIZE);
  }
  return 0;
}

void ReplicaMgr::flush() {
  pthread_mutex_lock(&m_flush_mutex);
  m_flush_mutex_holder = pthread_self();
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  queue<shared_ptr<OpRequest>> t;
  t.swap(m_new);
  pthread_cond_broadcast(&m_cond_loggers);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  _flush(&t);

  m_flush_mutex_holder = 0;
  pthread_mutex_unlock(&m_flush_mutex);
}

void ReplicaMgr::_flush(queue<shared_ptr<OpRequest>>* t) {
  shared_ptr<OpRequest> r;
  while (!t->empty()) {
    r = t->front();
    t->pop();
    assert(r.get());  // EntryPtr shouldn't be empty

    // modify the request to suit the replicate state
    // the replicated path modify should be done on other ioproxy
    //        auto rep_path = fs::path("replicate") / r->filepath();
    //        r->set_filepath(rep_path.string());
    if (r->type() == OpType::write) {
      r->set_type(OpType::rep_update_index);
    } else if (r->type() == OpType::setxattr) {
      if (r->xattr_name().substr(0, 9) == "user.gvds")
        continue;
    }
    auto cids = r->extra();
    string fromcid;
    fromcid.push_back(mcid);
    r->set_extra(move(fromcid));
    for (int i = 0; i < cids.size(); i++) {
      uint32_t id = cids[i];
      if (id == mcid) continue;
      auto oper = get_operator(ioproxys[id], 100);
      OpReply reply;
      auto status = oper->Submit(*r, reply);
      dout(-1) << "sent replicated request to target ioproxy "
               << ioproxys[id]->cid << dendl;
      if (status.ok()) {
        dout(-1) << "replicated result from target ioproxy "
                 << ioproxys[id]->cid << " :" << reply.err_code() << dendl;
      }
    }
    dout(-1) << "replicating request: " << r->filepath() << " >"
             << OpType_Name(r->type()) << dendl;
  }
}

shared_ptr<OpRequest> ReplicaMgr::_replicate_space_request(fs::path space, const string& cids) {
  // I use OpReply as the container of directory structure
  auto request = make_shared<OpRequest>();
  request->set_filepath(space.string());
  request->set_type(OpType::rep_init_space);
  request->set_extra(cids);
  OpReply dir_entries;
  fs::path rela_dir = space;
  stack<fs::path> dirs;
  dirs.push(space);
  // use stack to deep scan through the space dir tree
  while (!dirs.empty()) {
    auto cur_dir = dirs.top();
    dirs.pop();
    DIR* dp = ::opendir((iop->data_path / cur_dir).c_str());
    struct dirent* de;
    if (dp == nullptr) {
      ::closedir(dp);
      // cannot open dir, may been deleted by others
      continue;
    }
    de = ::readdir(dp);
    if (de == nullptr) {
      // cannot read dir, it's rare
      continue;
    }
    // push the dir entry into entries
    do {
      // dir entry contains inode, offset, length of record, type and name
      string filename(de->d_name);
      if (filename == "." || filename == "..") continue;
      auto attr = new Attr();
      // the url of dest entry is { Data Directory in IOP Server / Directory
      // relative path / entry name }
      int err =
          iop->proxy_op.get_attr(iop->data_path / cur_dir / filename, *attr);
      if (err != 0) {
        // this is rare in readdir
        delete attr;
        continue;
      } else {
        // not follow the link so dead cycle would not happen
        if (attr->type() == Attr_NodeType_Directory) {
          dirs.push(cur_dir / filename);
        }
        dir_entries.mutable_entry_names()->Add(cur_dir / filename);
        dir_entries.mutable_entries()->AddAllocated(attr);
      }
      // the ownership of filename and attr ends here
    } while ((de = ::readdir(dp)) != nullptr);
    ::closedir(dp);
  }
  // I explicitly avoid the deep copy although the construct from the return of
  // string may optimized by compiler
  auto serialize_entries = new string;
  dir_entries.SerializeToString(serialize_entries);
  request->set_allocated_data(serialize_entries);
  return request;
}