//
// Created by yaowen on 4/24/19.
// 北航系统结构所-存储组
//

#include "sync_io.h"
extern int errno;

sync_io::sync_io() = default;

sync_io::~sync_io() = default;

int sync_io::sopen(const char *pathname, int flags, mode_t mode) {
    int fd = open(pathname, flags, mode);
    if (fd == -1){
        perror("sync_io open");
    }
    return fd;
}

int sync_io::sclose(int fd) {
    return close(fd);
}

ssize_t sync_io::sread(int fd, void *buf, size_t count, long long offset) {
    if(lseek(fd, offset, SEEK_SET)==-1){
        perror("sync_io read lseek");
    }
    ssize_t ret = read(fd, buf, count);
    if (ret == -1){
        perror("sync_io read");
    }
    return ret ;
}

ssize_t sync_io::sread(const char *path, void *buf, size_t count, long long offset) {
    int fd = open(path, O_RDONLY|O_SYNC);
    if(fd == -1){
        perror("sync_io sread open");
    }
    return sread(fd, buf, count, offset);
}

ssize_t sync_io::swrite(int fd, const void *buf, size_t count, long long offset) {
    if(lseek(fd, offset, SEEK_SET)==-1){
        perror("sync_io write lseek");
    }
    ssize_t ret = write(fd, buf, count);
    if(ret == -1){
        perror("sync_io write");
    }
    return ret;
}

ssize_t sync_io::swrite(const char *path, const void *buf, size_t count, long long offset) {
    int fd = open(path, O_WRONLY|O_SYNC|O_CREAT, 0655);
    if (fd == -1){
        perror("sync_io swrite open");
    }
    return swrite(fd, buf, count, offset); // 调用上面针对 fd 的写接口
}

int sync_io::sstat(const char *pathname, struct stat *statbuf) {
    int ret = stat(pathname, statbuf);
    if (ret == -1){
        perror("sync_io stat");
    }
    return ret;
}

int sync_io::sfstat(int fd, struct stat *statbuf) {
    int ret = fstat(fd, statbuf);
    if(ret == -1){
        perror("sync_io fstat");
    }
    return ret;
}

