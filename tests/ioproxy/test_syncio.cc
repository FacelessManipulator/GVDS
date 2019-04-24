//
// Created by yaowen on 4/24/19.
// 北航系统结构所-存储组
//

#include "gtest/gtest.h"
#include "io_proxy/sync_io.h"
#include "sys/types.h"
#include "sys/stat.h"
#include <stdlib.h>
#include <iostream>
#include "msg/op.h"
#include "io_proxy/proxy_op.h"
#include <memory>

#define  TFILEP "/tmp/hvs/tests/data/syncio.txt"
using namespace hvs;
TEST(SYNCIO, OPEN){
    hvs::OP op;
    sync_io global_sync_io;
    int fd = global_sync_io.sopen(TFILEP, O_RDONLY|O_SYNC, 0655, &op);
    std::cout << std::endl << fd << std::endl;
    global_sync_io.sclose(fd, &op);
}

TEST(SYNCIO, READ){
    hvs::OP op;
    sync_io global_sync_io;
    int cnt = 512;
    char* buf = (char*) malloc(static_cast<size_t>(cnt));
    memset(buf, 0, static_cast<size_t>(cnt));
    std::cout << std::endl << global_sync_io.sread(TFILEP, buf, static_cast<size_t>(cnt), 0, &op) << std::endl << buf <<std::endl;
}

TEST(SYNCIO, WRITE){
    hvs::OP op;
    sync_io global_sync_io;
    int cnt = 50;
    char* buf = (char*) malloc(static_cast<size_t>(cnt));
    memset(buf, 'A', static_cast<size_t>(cnt));
    std::cout << std::endl << global_sync_io.swrite(TFILEP, buf, static_cast<size_t>(cnt), 0, &op) << std::endl << buf <<std::endl;
}

TEST(SYNCIO, STAT){
    sync_io global_sync_io;
    std::shared_ptr<IOProxyMetadataOP> op_ptr = std::make_shared<IOProxyMetadataOP>();
    op_ptr->operation = IOProxyMetadataOP::stat;
    op_ptr->type = hvs::OPType::IO_PROXY_METADATA;
    prepare_op(op_ptr);
    hvs::IOProxyMetadataOP *op = op_ptr.get();
    std::cout << std::endl << global_sync_io.sstat(TFILEP, op) << std::endl;
    struct stat fstat = *(op->buf);
    std::cout << "Meta data:" << std::endl;
    std::cout << op->buf->st_dev << std::endl;
    std::cout << fstat.st_ino << std::endl;
    std::cout << fstat.st_mode << std::endl;
    std::cout << fstat.st_nlink << std::endl;
    std::cout << fstat.st_uid << std::endl;
    std::cout << fstat.st_gid << std::endl;
    std::cout << fstat.st_rdev << std::endl;
    std::cout << fstat.st_size << std::endl;
    std::cout << fstat.st_blksize << std::endl;
    std::cout << fstat.st_blocks << std::endl;
    std::cout << fstat.st_atim.tv_sec << std::endl;
    std::cout << fstat.st_atim.tv_nsec << std::endl;
}