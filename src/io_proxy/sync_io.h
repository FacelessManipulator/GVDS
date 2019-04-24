//
// Created by yaowen on 4/24/19.
// 北航系统结构所-存储组
//


#pragma once //HVSONE_SYNC_IO_H
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "msg/op.h"
namespace  hvs{
    class sync_io {
    public:
        sync_io();
        ~sync_io();
    public:
        void mknod(){}; // 创建一个文件节点
        void statfs(){};

        void symlink(){}; // 创建软链接
        void link(){}; // 创建硬链接
        void unlink(){}; // 移除链接文件
        void readlink(){}; // 读取链接文件链接路径

        void mkdir(){}; // 创建目录
        void rmdir(){}; // 删除目录
        void opendir(){}; // 打开目录
        void readdir(){}; // 读目录
        void releasedir(){}; // 释放目录
        void fsyncdir(){}; // 同步目录

        void rename(){}; // 重命名
        void chmod(){}; // 更改文件权限
        void chown(){}; // 更改文件所属者

        int sstat(const char* pathname, IOProxyMetadataOP*); // 获得文件属性（元数据）,通过路径
        int sfstat(int fd, IOProxyMetadataOP*); // 获取文件元数据，通过fd

        int sopen(const char* pathname, int flags, mode_t mode, OP*); // 打开文件
        ssize_t  sread(int fd, void*buf, size_t count, off_t offset, OP*); // 读数据 fd, offset
        ssize_t  sread(const char* path, void*buf, size_t count, off_t offset, OP*); // 读数据 path, offset
        ssize_t swrite(int fd, const void*buf, size_t count, off_t offset, OP*); // 写数据
        ssize_t swrite(const char* path, const void*buf, size_t count, off_t offset, OP*); // 写数据
        int sclose(int fd, struct OP*); // 关闭文件
        void truncate(){}; // 更改文件大小

        void access(){}; // 读取文件权限
        void create(){}; // 创建或者打开一个文件
        void release(){}; // 最终释放一个文件，除非再次对文件进行open操作，否则不能再次对文件进行读写

        void flush(){}; // 当文件fd 被close的时候，通常会调用flush;
        void fsync(){}; // 同步文件内容
        void read_buf(){}; // 类似读，不过数据目的地是buffer
        void write_buf(){}; // 类似写，不过数据源是buffer
        void poll(){}; // 客户端实现，用于事件机制中为poll 提供支持；
        void fallocate(){}; // 为一个打开的文件创建空间
        void flock(){}; // 执行BSD协议的LOCK操作
        void lock(){}; // 为一个文件上锁，执行POSIX协议的LOCK操作

        void getxattr(){}; // 读出 xattr
        void listxattr(){}; // 列出 xattr
        void setxattr(){}; // 设置 xattr
        void removexattr(){}; // 移除 xattr
    };
}

//HVSONE_SYNC_IO_H
