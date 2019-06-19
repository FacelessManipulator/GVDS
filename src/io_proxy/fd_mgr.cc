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

using namespace hvs;
using namespace std;

int FdManager::open(const string& path, int flags, int mode) {
    auto fd_it = fds.find(path);
    if (fd_it != fds.end()) {
        fds.hit(path);
        return fd_it->data;
    } else {
        // TODO: we need handle the flags and mode properly, currently by pass
        int mask_flags = O_SYNC | O_DSYNC | O_DIRECT ;
        flags &= ~mask_flags;
        int fd = ::open(path.c_str(), flags, mode);
        // open error may happened if fd == -1
        // fd usually >=0, 0 usually stands for standard output
        if (fd == -1) {
            int err = errno;
            return err;
        } else if (fd >= 0) {
            auto fd_create = fds.touch(path);
            fd_create.data = fd;
            // TODO: close may take some time to flush, need more work such as background close queue
            checkAndExpire();
            return fd_create.data;
        } else {
            // unknown problem happened
            return EACCES;
        }
    }
}

int FdManager::close(const string& path) {
    this->flush(path);
    return 0;
}

int FdManager::flush(const string& path) {
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
