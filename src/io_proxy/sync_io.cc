//
// Created by yaowen on 4/24/19.
// 北航系统结构所-存储组
//

#include <context.h>
#include "sync_io.h"
using namespace hvs;

sync_io::sync_io() = default;

sync_io::~sync_io() = default;

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
    int ret = close(fd);
    op->error_code = 0;
    if (ret == -1){
        perror("sync_io close");
        op->error_code = -errno;
    }
    return ret;
}

ssize_t sync_io::sread(int fd, void *buf, size_t count, off_t offset, OP* op) {
    /*
    if(lseek(fd, offset, SEEK_SET)==-1){
        perror("sync_io read lseek");
    }

    ssize_t ret = read(fd, buf, count);
     */
    ssize_t ret = pread(fd, buf, count, offset); // 该操作是原子操作
    op->error_code = static_cast<int>(ret);
    if (ret == -1){
        perror("sync_io sread");
        op->error_code = -errno;
    }
    return ret ;
}

ssize_t sync_io::sread(const char *path, void *buf, size_t count, off_t offset, OP* op) {
    int fd = open(path, O_RDONLY|O_SYNC);
    op->error_code = 0;
    if(fd == -1){
        perror("sync_io sread open");
        op->error_code = -errno;
    }
    ssize_t ret = sread(fd, buf, count, offset, op);
    close(fd);
    return ret;
}

ssize_t sync_io::swrite(int fd, const void *buf, size_t count, off_t offset, struct OP* op) {
    /*
    if(lseek(fd, offset, SEEK_SET)==-1){
        perror("sync_io write lseek");
    }
    ssize_t ret = write(fd, buf, count);
     */
    ssize_t ret = pwrite(fd, buf, count, offset); // 该操作是原子操作
    op->error_code = static_cast<int>(ret);
    if(ret == -1){
        perror("sync_io write");
        op->error_code = -errno;
    }
    return ret;
}

ssize_t sync_io::swrite(const char *path, const void *buf, size_t count, off_t offset, struct OP* op) {
    int fd = open(path, O_WRONLY|O_SYNC|O_CREAT, 0655);
    op->error_code = 0;
    if (fd == -1){
        perror("sync_io swrite open");
        op->error_code = -errno;
    }
    ssize_t ret = swrite(fd, buf, count, offset, op);
    close(fd);
    return ret; // 调用上面针对 fd 的写接口
}

int sync_io::sstat(const char *pathname, IOProxyMetadataOP* op) {
//    std::cout << pathname << std::endl;
    int ret = stat(pathname, op->buf);
    op->error_code = 0;
    if (ret == -1){
        perror("sync_io stat");
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
//    std::cout << "dirent!" << std::endl;
    DIR *dp = opendir(path);
    struct dirent *de;
    if (dp == nullptr){
        op->error_code = -errno;
        return op->error_code;
    }
    de = readdir(dp);
    if (de == nullptr) {
        op->error_code = -errno;
        return op->error_code;
    }
    do {
        op->dirvector.emplace_back(*de); // 传送到 dirvector 之中
        errno = 0;
    } while ((de = readdir(dp)) != nullptr);
    op->error_code = -errno;
    return -op->error_code;
}

//void sync_io::hvsfs_fullpath(char *fpath, const char *path) {
//    // 此函数用来拼接路径。
//    strcpy(fpath, hvs::HvsContext::get_context()->ioproxy_rootdir.c_str());
//    strncat(fpath, path, PATH_MAX);
//}

