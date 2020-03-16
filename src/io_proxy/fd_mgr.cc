//
// Created by miller on 6/18/19.
//

#include "fd_mgr.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <sys/xattr.h>

using namespace gvds;
using namespace std;


string HvsFileHandler::get_rep_centers() {
    if (cids.size() != 0) {
        // means already read the cids from file's attr
    } else {
        string buf;
        buf.resize(256);
        int err = ::fgetxattr(fd, _GVDS_SPACE_REPLICA_CENTER_IDS, buf.data(), 256);
        lock_guard<mutex> lock(fh_mu);
        if (err <= 0) {
            // means file may not be a replica file
            // 0 means local center
            cids = "\0";
        } else {
            cids.swap(buf);
        }
    }
    return cids;
}

roaring_bitmap_t* HvsFileHandler::get_bitmap(int cid, bool create) {
    lock_guard<mutex> lock(fh_mu);
    roaring_bitmap_t* bitmap = nullptr;
    // file may not exists
    if (fd < 0)
        return nullptr;
    auto bm_it = bitmaps.find(cid);
    if (bm_it == bitmaps.end()) {
        fh_mu.unlock();
        char bitmap_xattr_name[64];
        char bitmap_xattr_value[128*1024];// the maximum size of xattr value limited by vfs is 128KB
        snprintf(bitmap_xattr_name, 64, _GVDS_SPACE_REPLICA_FILE_BITMAP_WITH_CID, cid);
        int count = ::fgetxattr(fd, bitmap_xattr_name, bitmap_xattr_value, 128*1024);
        if (count > 0) {
            bitmap = roaring_bitmap_portable_deserialize_safe(bitmap_xattr_value, count);
        } else if (create) {
            bitmap = roaring_bitmap_create();
        }
        fh_mu.lock();
        if (bitmap != nullptr) {
            bitmaps[cid] = bitmap;
        }
    } else {
        bitmap = bm_it->second;
    }
    return bitmap;
}

void HvsFileHandler::set_bitmap(int cid, roaring_bitmap_t* bitmap) {
    lock_guard<mutex> lock(fh_mu);
    auto bm_it = bitmaps.find(cid);
    if (bm_it != bitmaps.end()) {
        roaring_bitmap_free(bm_it->second);
    }
    bitmaps[cid] = bitmap;
}

bool HvsFileHandler::flush_bitmap(int cid) {
    lock_guard<mutex> lock(fh_mu);
    roaring_bitmap_t* bitmap = nullptr;
    // file may not exists
    if (fd < 0)
        return false;
    auto bm_it = bitmaps.find(cid);
    if (bm_it == bitmaps.end()) {
        // file's bitmap not changed, should be sync
        return true;
    } else {
        bitmap = bm_it->second;
        fh_mu.unlock();
        int bitmap_size = roaring_bitmap_portable_size_in_bytes(bitmap);
        // TODO: handle the situation that bitmap size bigger than 128KB
//            assert(bitmap_size < 128*1024);
        char bitmap_xattr_name[64];
        char *bitmap_xattr_value = new char[bitmap_size];// the maximum size of xattr value limited by vfs is 128KB
        roaring_bitmap_portable_serialize(bitmap, bitmap_xattr_value);
        snprintf(bitmap_xattr_name, 64, _GVDS_SPACE_REPLICA_FILE_BITMAP_WITH_CID, cid);
        int count = ::fsetxattr(fd, bitmap_xattr_name, bitmap_xattr_value, bitmap_size, 0);
        fh_mu.lock();
        if (count < 0) {
            dout(-1) << "set replica attr failed: "<< errno << dendl;
        }
        return true;
    }
}

bool HvsFileHandler::flush_bitmap() {
    for (auto& [cid, bm]: bitmaps) {
        flush_bitmap(cid);
    }
}

bool HvsFileHandler::punch_hole_local(uint64_t offset, size_t len) {
    // file may not exists
    if (fd < 0)
        return false;
    // if offset + len bigger than the current file size, should first use truncate to append the hole
    // fallocate with FALLOC_FL_INSERT_RANGE may failed
    uint64_t eof = lseek(fd, 0, SEEK_END);

    // try extend the [off, len) to page align
  uint64_t end = offset + len;
  uint64_t end_align = end;
  uint64_t offset_align = offset &~ _GVDS_BASE_FS_PAGE_MASK;
  if ((end & _GVDS_BASE_FS_PAGE_MASK) != 0) {
    end_align = (end &~ _GVDS_BASE_FS_PAGE_MASK) + _GVDS_BASE_FS_PAGE_SIZE;
    // make sure the extend offset and length will not change the promised eof
//    end = end_ext > eof ? end : end_ext;
  }
//  len = end - offset;

    // there are 3 situations:
    // 1. {0, eof, offset, offset+len}, only truncate(offset+len) is needed
    // 2. {0, offset, eof, offset+len}. need truncate(offset+len) and then fallocate(offset, eof-offset)
    // 3. {0, offset, offset+len, eof}. only fallocate(offset, offset+len) needed
    if (end > eof) {
        ::ftruncate(fd, end);
    }

  // offset may smaller than the eof which means we need punch hole in exists data space
  // cuz offset+len > eof, the unsigned type (offset+len-eof) would not smaller than 0
        int err = ::fallocate(fd, FALLOC_FL_PUNCH_HOLE|FALLOC_FL_KEEP_SIZE, offset_align, end_align-offset_align);
        if (err == -1) {
            dout(-1) << "ERROR punch hole with file. errno:" << errno << dendl;
            return false;
        }
    return true;
}

std::vector<std::pair<uint64_t, uint64_t>> HvsFileHandler::find_holes_in_range(uint64_t from, uint64_t to) {
    vector<pair<uint64_t, uint64_t>> holes;
    uint64_t eof = lseek(fd, 0, SEEK_END);
    uint64_t off = from;
    to = min(to, eof);
    while (off < to) {
        // seek the cursor to the begin of data
        uint64_t data_begin = lseek(fd, off, SEEK_DATA);
        if (data_begin > off) {
            // means there is a hole between [off, data_begin)
            if (data_begin >= to) {
                holes.emplace_back(off, to);
                break;
            } else {
                holes.emplace_back(off, data_begin);
            }
        }
        // move cursor to the end of data (aka the begin of next hole)
      uint64_t hole_begin = lseek(fd, data_begin, SEEK_HOLE);
        // means there is a hole between [hole_begin, ?), the layout is {hole[off, data_begin), data[data_begin, hole_begin)}
        // move the off cursor to the end of cur layout, if hole_begin > to, the loop would be ended, no hole found in next loop.
        // If hole_begin < to, a hole found and would be push back into vector in next loop.
        off = hole_begin;
        // continue the loop
    }
    return move(holes);
}

std::vector<std::pair<uint64_t, uint64_t>> HvsFileHandler::find_data_in_range(uint64_t from, uint64_t to) {
    vector<pair<uint64_t, uint64_t>> holes;
  uint64_t eof = lseek(fd, 0, SEEK_END);
  uint64_t off = from;
  to = min(to, eof);
    while (off < to) {
        // seek the cursor to the begin of data
        uint64_t data_begin = lseek(fd, off, SEEK_HOLE);
        if (data_begin > off) {
            // means there is a hole between [off, data_begin)
            if (data_begin >= to) {
                holes.emplace_back(off, to);
                break;
            } else {
                holes.emplace_back(off, data_begin);
            }
        }
        // move cursor to the end of data (aka the begin of next hole)
        int hole_begin = lseek(fd, data_begin, SEEK_DATA);
        // means there is a hole between [hole_begin, ?), the layout is {hole[off, data_begin), data[data_begin, hole_begin)}
        // move the off cursor to the end of cur layout, if hole_begin > to, the loop would be ended, no hole found in next loop.
        // If hole_begin < to, a hole found and would be push back into vector in next loop.
        off = hole_begin;
        // continue the loop
    }
    return move(holes);
}

int FdManager::open(const string& path, int flags, int mode) {
    lock_guard<mutex> lock(fdm_mu);
    auto fd_it = fds.find(path);
    if (fd_it != fds.end()) {
        // we should trunc this preallocate file handler if trunc flag is set
        if(flags & O_TRUNC)
            ::ftruncate(fd_it->data, 0);
        fds.hit(path);
        return fd_it->data;
    } else {
        // TODO: we need handle the flags and mode properly, currently by pass
        int mask_flags = O_SYNC | O_DSYNC | O_DIRECT | O_RDONLY | O_WRONLY;
        flags &= ~mask_flags;
        int fd = ::open(path.c_str(), flags, mode);
        // open error may happened if fd == -1
        // fd usually >=0, 0 usually stands for standard output
        if (fd == -1) {
            int err = -errno;
            return err;
        } else if (fd >= 0) {
            fds.touch(path, fd);
            // TODO: close may take some time to flush, need more work such as background close queue
            checkAndExpire();
            return fd;
        } else {
            // unknown problem happened
            return -EACCES;
        }
    }
}

int FdManager::close(const string& path) {
    this->flush(path);
    return 0;
}

int FdManager::create(const string& path, int mode) {
    lock_guard<mutex> lock(fdm_mu);
    int fd_new = ::open(path.c_str(), O_CREAT | O_SYNC ,mode);
    if(fd_new >= 0) {
        auto fd_it = fds.find(path);
        if (fd_it != fds.end()) {
            // close fd
            int fd_old = fd_it->data;
            fds.remove(fd_it->key);
            ::close(fd_old);
        }
        ::close(fd_new);
        return 0;
    } else {
        return fd_new;
    }
}

int FdManager::remove(const string& path) {
    lock_guard<mutex> lock(fdm_mu);
    auto fd_it = fds.find(path);
    if (fd_it != fds.end()) {
        // close fd
        int fd_old = fd_it->data;
        fds.remove(fd_it->key);
        ::close(fd_old);
    }
    int fd_new = ::unlink(path.c_str());
    return fd_new;
}

int FdManager::expire(const string& path) {
    lock_guard<mutex> lock(fdm_mu);
    auto fd_it = fds.find(path);
    int fd_old = -1;
    if (fd_it != fds.end()) {
        // close fd
        fd_old = fd_it->data;
        fds.remove(fd_it->key);
        ::close(fd_old);
    }
    return fd_old;
}

int FdManager::flush(const string& path) {
    lock_guard<mutex> lock(fdm_mu);
    // out handler, not really close file but only flush it
    auto fd_it = fds.find(path);
    if (fd_it != fds.end()) {
        int err = ::fsync(fd_it->data);
        // TODO: we don't care about flush result?
        return 0;
    } else {
        // may already closed by lru expired
        return 0;
    }
}

void FdManager::checkAndExpire() {
    while (fds.size() > max_fd_num) {
        auto fdo_exp = fds.expire();
        if(fdo_exp.has_value()) {
            int fd_exp = fdo_exp.value().data;
            ::close(fd_exp);
        } else {
            break;
        }
    }
}


std::shared_ptr<HvsFileHandler> FdManager::get_file_handler(const std::string& path) {
    shared_ptr<HvsFileHandler> fh;
    int fd = this->open(path, O_RDWR, 0664);
    // file may not exists
    if (fd < 0)
        return nullptr;
    lock_guard<mutex> lock(fdm_mu);
    auto fh_it = file_handlers.find(fd);
    if (fh_it == file_handlers.end()) {
        fh = make_shared<HvsFileHandler>(fd);
        file_handlers[fd] = fh;
    } else {
        fh = fh_it->second;
    }
    return fh;
}