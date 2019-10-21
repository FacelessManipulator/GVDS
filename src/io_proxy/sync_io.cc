//
// Created by yaowen on 4/24/19.
// 北航系统结构所-存储组
//

#include <context.h>
#include "sync_io.h"
#include "io_proxy/io_proxy.h"
#include "io_proxy/fd_mgr.h"

using namespace hvs;

sync_io::sync_io(IOProxy* ioProxy): iop(ioProxy) {}

sync_io::~sync_io() {}

int sync_io::sopen(const char *pathname, int flags, mode_t mode, OP* op) {
    int fd = open(pathname, flags, mode);
    op->error_code = 0;
    if (fd == -1){
        perror("sync_io open");
        op->error_code = -errno;
    }
    return fd;
}

int sync_io::sclose(int fd, struct OP* op) {
//    int ret = close(fd);
    op->error_code = 0;
    int ret = 0;
    if (ret == -1){
        perror("sync_io close");
        op->error_code = -errno;
    }
    return ret;
}

ssize_t sync_io::sread(int fd, void *buf, size_t count, off_t offset, OP* op) {
    ssize_t ret = pread(fd, buf, count, offset); // 该操作是原子操作
    op->error_code = 0;
    if (ret == -1){
        perror("sync_io sread");
        op->error_code = -errno;
    }
    return ret ;
}

ssize_t sync_io::sread(const std::string& path, void *buf, size_t count, off_t offset, OP* op) {
    int fd = iop->fdm.open(path, op->open_flags | O_RDWR, 0655);
//    int fd = op->fid;
    op->error_code = 0;
    if(fd < 0){
        op->error_code = -fd;
    }
    ssize_t ret = sread(fd, buf, count, offset, op);
    return ret;
}

ssize_t sync_io::swrite(int fd, const void *buf, size_t count, off_t offset, struct OP* op) {
    ssize_t ret = pwrite(fd, buf, count, offset); // 该操作是原子操作
    op->error_code = static_cast<int>(ret);
    if(ret == -1){
        op->error_code = -errno;
        dout(-1) << "write error: " << op->error_code << " res: " << strerror(op->error_code) << dendl;
    }
    return ret;
}

ssize_t sync_io::swrite(const std::string& path, const void *buf, size_t count, off_t offset, struct OP* op) {
    // TODO: there shouldn't have default flags and mode
    int fd = iop->fdm.open(path, op->open_flags | O_RDWR, 0655);
    op->error_code = 0;
    if (fd == -1){
        dout(-1) << "sync_io swrite open "<< path << dendl;
        op->error_code = -errno;
        return -errno;
    } else {
        ssize_t ret = swrite(fd, buf, count, offset, op);
        return ret; // 调用上面针对 fd 的写接口
    }
}

int sync_io::sstat(const char *pathname, IOProxyMetadataOP* op) {
    int ret = stat(pathname, op->buf);
    op->error_code = 0;
    if (ret == -1){
        perror("sync_io stat");
        std::cout << pathname << std::endl;
        op->error_code = -errno;
    }
    return ret;
}

int sync_io::sfstat(int fd, IOProxyMetadataOP* op) {
    int ret = fstat(fd, op->buf);
    op->error_code = 0;
    if(ret == -1){
        perror("sync_io fstat");
        op->error_code = -errno;
    }
    return ret;
}

int sync_io::sreaddir(const char *path, IOProxyMetadataOP *op) {
    int ret = 0;
    op->error_code = 0;
    DIR *dp = opendir(path);
    struct dirent *de;
    if (dp == nullptr){
        perror("sync_io sreaddir");
        op->error_code = -errno;
        return op->error_code;
    }
    de = readdir(dp);
    if (de == nullptr) {
        perror("sync_io sreaddir");
        op->error_code = -errno;
        closedir(dp);
        return op->error_code;
    }
    do {
        op->dirvector.emplace_back(*de); // 传送到 dirvector 之中
        errno = 0;
    } while ((de = readdir(dp)) != nullptr);
    op->error_code = -errno;
    closedir(dp);
    return -op->error_code;
}

int sync_io::struncate(const char *path, off_t length) {
    int ret = 0;
    iop->fdm.expire(path);
    ret = truncate(path, length);
    if (ret == -1){
        perror("sync_io truncate");
        ret = -errno;
    }
    return ret;
}

int sync_io::srename(const char *path, const char *newpath) {
    int ret = 0;
    iop->fdm.expire(path);
    iop->fdm.expire(newpath);
    ret = rename(path, newpath);
    if (ret == -1){
        perror("sync_io srename");
        ret = -errno;
    }
    return ret;
}

int sync_io::smkdir(const char *path, mode_t mode) {
    int ret = 0;
    ret = mkdir(path, mode);
    if (ret == -1){
        perror("sync_io mkdir");
        ret = -errno;
    }
    return ret;
}

int sync_io::srmdir(const char *path) {
    int ret = 0;
    ret = rmdir(path);
    if (ret == -1){
        perror("sync_io rmdir");
        ret = -errno;
    }
    return ret;
}

int sync_io::screate(const char *path, mode_t mode) {
    int ret = iop->fdm.create(path, mode); // 默认创建文件权限为 0644 rw-r--r--
    if (ret < 0){
        perror("sync_io create");
    }
    return ret;
}

int sync_io::sunlink(const char *path) {
    int ret = iop->fdm.remove(path);
    if (ret < 0){
        perror("sync_io unlink");
    }
    return ret;
}

int sync_io::slink(const char *path, const char *newpath) {
    int ret = 0;
    ret = link(path, newpath);
    if (ret == -1){
        perror("sync_io link");
        ret = -errno;
    }
    return ret;
}

int sync_io::saccess(const char *path, int mode) {
    int ret = 0;
    ret = access(path, mode);
    if (ret == -1){
        perror("sync_io access");
        ret = -errno;
    }
    return 0;
}

int sync_io::sutimes(const char *path, long int sec0n, long int sec0s, long int sec1n, long int sec1s) {
    int ret = 0;
    struct timespec time[2];
    time[0].tv_nsec = sec0n;
    time[0].tv_sec = sec0s;
    time[1].tv_nsec = sec1n;
    time[1].tv_sec = sec1s;
    ret = utimensat(AT_FDCWD, path, time, 0);
    if (ret == -1){
        perror("sync_io utimensat");
        ret = -errno;
    }
    return ret;
}

int sync_io::ssymlink(const char *target, const char *newlinkpath) {
    int ret = 0;
    ret = symlink(target, newlinkpath);
    if (ret == -1){
        perror("sync_io symlink");
        ret = -errno;
    }
    return ret;
}

int sync_io::sreadlink(const char *path, IOProxyDataOP *op) {
    char buf[op->size];
    int ret = 0;
    ret = static_cast<int>(readlink(path, buf, op->size - 1));
    if (ret == -1){
        perror("sync_io readlink");
        ret = -errno;
    }
    if (ret >= 0) {
        buf[ret] = '\0';
        op->linkbuf = buf;
    }
    return 0;
}

int sync_io::schmod(const char *path, mode_t mode) {
    int ret = 0;
    iop->fdm.expire(path);
    ret = chmod(path, mode);
    if (ret == -1){
        perror("sync_io chmod");
        ret = -errno;
    }
    return ret;
}

int sync_io::schown(const char *path, uid_t uid, gid_t gid) {
    int ret = 0;
    iop->fdm.expire(path);
    ret = chown(path, uid, gid);
    if (ret == -1){
        perror("sync_io chown");
        ret = -errno;
    }
    return ret;
}

