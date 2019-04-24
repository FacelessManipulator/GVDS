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

#define  TFILEP "/tmp/hvs/tests/data/syncio.txt"

TEST(SYNCIO, OPEN){
    sync_io global_sync_io;
    std::cout << std::endl << global_sync_io.sopen(TFILEP, O_RDONLY|O_SYNC, 0655) << std::endl;
}

TEST(SYNCIO, READ){
    sync_io global_sync_io;
    int cnt = 512;
    char* buf = (char*) malloc(static_cast<size_t>(cnt));
    memset(buf, 0, static_cast<size_t>(cnt));
    std::cout << std::endl << global_sync_io.sread(TFILEP, buf, static_cast<size_t>(cnt), 0) << std::endl << buf <<std::endl;
}

TEST(SYNCIO, WRITE){
    sync_io global_sync_io;
    int cnt = 50;
    char* buf = (char*) malloc(static_cast<size_t>(cnt));
    memset(buf, 'A', static_cast<size_t>(cnt));
    std::cout << std::endl << global_sync_io.swrite(TFILEP, buf, static_cast<size_t>(cnt), 0) << std::endl << buf <<std::endl;
}

TEST(SYNCIO, STAT){
    sync_io global_sync_io;
    struct stat fstat{}; // 生命结构体，并进行初始化；
    std::cout << std::endl << global_sync_io.sstat(TFILEP, &fstat) << std::endl;
    std::cout << "Meta data:" << std::endl;
    std::cout << fstat.st_dev << std::endl;
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