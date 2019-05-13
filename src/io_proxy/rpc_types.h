//
// Created by yaowen on 5/4/19.
// 北航系统结构所-存储组
//

#ifndef HVSONE_RPC_TYPES_H
#define HVSONE_RPC_TYPES_H
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
namespace hvs{
    struct ioproxy_rpc_statbuffer {
        int error_code;
        int st_dev;
        int st_ino;
        int st_mode;
        int st_nlink;
        int st_uid;
        int st_gid;
        int st_rdev;
        int st_size;
        int st_blksize;
        int st_blocks;
        int st_atim_tv_nsec;
        int st_atim_tv_sec;
        int st_mtim_tv_nsec;
        int st_mtim_tv_sec;
        int st_ctim_tv_nsec;
        int st_ctim_tv_sec;
        ioproxy_rpc_statbuffer(struct stat* st) {
            error_code = 0;
            st_dev = static_cast<int>(st->st_dev);
            st_ino = static_cast<int>(st->st_ino);
            st_mode = st->st_mode;
            st_nlink = static_cast<int>(st->st_nlink);
            st_uid = st->st_uid;
            st_gid = st->st_gid;
            st_rdev = static_cast<int>(st->st_rdev);
            st_size = static_cast<int>(st->st_size);
            st_blksize = static_cast<int>(st->st_blksize);
            st_blocks = static_cast<int>(st->st_blocks);
            st_atim_tv_nsec = static_cast<int>(st->st_atim.tv_nsec);
            st_atim_tv_sec = static_cast<int>(st->st_atim.tv_sec);
            st_mtim_tv_nsec = static_cast<int>(st->st_mtim.tv_nsec);
            st_mtim_tv_sec = static_cast<int>(st->st_mtim.tv_sec);
            st_ctim_tv_nsec = static_cast<int>(st->st_ctim.tv_nsec);
            st_ctim_tv_sec = static_cast<int>(st->st_ctim.tv_sec);
        }
        ioproxy_rpc_statbuffer():error_code(-1) {}
        ioproxy_rpc_statbuffer(int i): error_code(i){/*当反回值error_code为小于0值的时候，表示产生了错误*/}
        MSGPACK_DEFINE_ARRAY(error_code, st_dev, st_ino, st_mode, st_nlink, st_uid, st_gid, st_rdev,
                             st_size, st_blksize, st_blocks, st_atim_tv_nsec, st_atim_tv_sec, st_mtim_tv_nsec, st_mtim_tv_sec,
                             st_ctim_tv_nsec, st_ctim_tv_sec)
    };

    struct ioproxy_rpc_buffer {
        int error_code;
        clmdep_msgpack::type::raw_ref buf;
        ioproxy_rpc_buffer(const char* buffer, int _size){
            error_code = 0;
            buf.ptr = buffer;
            buf.size = _size;
        }
        ioproxy_rpc_buffer():error_code(-1) {}
        ioproxy_rpc_buffer(int i): error_code(i){/*当反回值error_code为小于0值的时候，表示产生了错误*/}
        MSGPACK_DEFINE_ARRAY(error_code, buf);
    };

    struct ioproxy_rpc_dirent {
        int error_code;
        int d_ino;
        int d_off;
        unsigned short d_reclen;
        unsigned char dtype;
        std::string d_name;
        ioproxy_rpc_dirent(struct dirent *ent){
            error_code = 0;
            d_ino = static_cast<int>(ent->d_ino);
            d_off = static_cast<int>(ent->d_off);
            d_reclen = ent -> d_reclen;
            dtype = ent -> d_type;
            d_name = ent -> d_name;
        }
        ioproxy_rpc_dirent():error_code(-1) {}
        ioproxy_rpc_dirent(int i): error_code(i){/*当反回值error_code为小于0值的时候，表示产生了错误*/}
        MSGPACK_DEFINE_ARRAY(error_code, d_ino, d_off, d_reclen, dtype, d_name);
    };
}

#endif //HVSONE_RPC_TYPES_H
