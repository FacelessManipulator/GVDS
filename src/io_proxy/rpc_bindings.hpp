//
// Created by yaowen on 5/4/19.
// 北航系统结构所-存储组
//
#pragma once
#include <iostream>
#include <error.h>
#include "msg/rpc.h"
#include "rpc_types.h"
#include "io_proxy.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <limits.h>

namespace hvs {
    inline std::string hvsfs_fullpath(const std::string& path_rel) {
        // 此函数用来拼接路径。
        auto path_abs = static_cast<IOProxy*>(HvsContext::get_context()->node)->absolute_path(path_rel);
        return path_abs;
    }


    inline ioproxy_rpc_statbuffer ioproxy_stat(const std::string pathname) {
        std::string fullpath = hvsfs_fullpath(pathname);
        auto op = std::make_shared<hvs::IOProxyMetadataOP>();
        op->id = 0;
        op->operation = hvs::IOProxyMetadataOP::stat;
        op->path = fullpath.c_str();
        op->type = hvs::IO_PROXY_METADATA;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        if (op->error_code == 0) {
            return ioproxy_rpc_statbuffer(op->buf); //返回消息
        } else {
            return ioproxy_rpc_statbuffer(op->error_code);
        }
    }

    inline std::vector<ioproxy_rpc_statbuffer> ioproxy_stat_multi(std::vector<std::string> pathnames) {
        std::vector<std::string> real_path;
        std::vector<std::shared_ptr<hvs::OP>> ops;
        std::vector<ioproxy_rpc_statbuffer> results;
        for(auto& path : pathnames) {
            real_path.emplace_back(hvsfs_fullpath(path));
        auto op = std::make_shared<hvs::IOProxyMetadataOP>();
            op->id = 0;
            op->operation = hvs::IOProxyMetadataOP::stat;
            op->path = path.c_str();
            op->type = hvs::IO_PROXY_METADATA;
            ops.push_back(op);
        }
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(ops);
        for(auto& op:ops) {
            if (op->error_code == 0) {
                results.emplace_back(static_cast<IOProxyMetadataOP*>(op.get())->buf); //返回消息
            } else {
                results.emplace_back(op->error_code);
            }
        }
        return results;
    }

    inline ioproxy_rpc_buffer ioproxy_read(const std::string pathname, int size, int offset){
        std::string fullpath = hvsfs_fullpath(pathname);
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 1;
        op->operation = IOProxyDataOP::read;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->should_prepare = true;
        op->size = static_cast<size_t>(size);
        op->offset = offset;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        if (op->error_code >= 0) {
            ioproxy_rpc_buffer res(pathname.c_str(), op->release_obuf(), offset, size);
            res.finalize_buf = true;
            res.error_code = static_cast<int>(op->error_code);
            return res; //返回消息
        } else {
            return ioproxy_rpc_buffer(op->error_code);
        }
    }

    inline int ioproxy_write(const std::string pathname, ioproxy_rpc_buffer obuf,int size, int offset){
        std::string fullpath = hvsfs_fullpath(pathname);
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 2;
        op->operation = IOProxyDataOP::write;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->size = static_cast<size_t>(size);
        op->offset = offset;
        op->ibuf = obuf.buf.ptr;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_open(const std::string pathname){
        std::string fullpath = hvsfs_fullpath(pathname);
        std::cout << "open: " << fullpath.c_str() << std::endl;
        return 0;
    }

    inline int ioproxy_close(const int fd){
        std::cout << "close fd: " << fd << std::endl;
        return 0;
    }

    inline int ioproxy_opendir(const std::string pathname){
        std::string fullpath = hvsfs_fullpath(pathname);
        std::cout << "opendir: " << fullpath.c_str() << std::endl;
        return 0;
    }

    inline std::vector<ioproxy_rpc_dirent> ioproxy_readdir(const std::string pathname){
        std::string fullpath = hvsfs_fullpath(pathname);
        std::vector<ioproxy_rpc_dirent> retvec;
        std::cout << "readdir: " << pathname << std::endl;
        auto op = std::make_shared<IOProxyMetadataOP>();
        op->id = 3;
        op->operation = IOProxyMetadataOP::readdir;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_METADATA;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        if (op->error_code == 0) {
            for(dirent ent : op->dirvector){
                retvec.emplace_back(ioproxy_rpc_dirent(&ent));
            }
        } else {
            retvec.emplace_back(ioproxy_rpc_dirent(op->error_code));
            return retvec;
        }
    }

    inline int ioproxy_truncate(const std::string path, int offset){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "truncate: " << path << std::endl;
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 4;
        op->operation = IOProxyDataOP::truncate;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->offset = offset;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_rename(const std::string path, const std::string newpath){
        std::string fullpath = hvsfs_fullpath(path);
        std::string newfullpath = hvsfs_fullpath(newpath);
        std::cout << "rename: " << path << " -> " << newpath << std::endl;
        auto op = std::make_shared<IOProxyMetadataOP>();
        op->id = 5;
        op->operation = IOProxyMetadataOP::rename;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_METADATA;
        op->newpath = newfullpath.c_str();
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_mkdir(const std::string path, mode_t mode){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "mkdir: " << path << " - " << mode << std::endl;
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 6;
        op->operation = IOProxyDataOP::mkdir;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->mode = mode;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_rmdir(const std::string path){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "rmdir: " << path << std::endl;
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 7;
        op->operation = IOProxyDataOP::rmdir;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_create(const std::string path, mode_t mode){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "create: " << path << " - " << mode << std::endl;
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 8;
        op->operation = IOProxyDataOP::create;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->mode = mode;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_unlink(const std::string path){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "unlink: " << path << std::endl;
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 9;
        op->operation = IOProxyDataOP::unlink;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_link(const std::string path, const std::string newpath){
        std::string fullpath = hvsfs_fullpath(path);
        std::string newfullpath = hvsfs_fullpath(newpath);
        std::cout << "link: " << path << " <=> " << newpath << std::endl;
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 10;
        op->operation = IOProxyDataOP::link;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->newpath = newfullpath.c_str();
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_access(const std::string path, int mode){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "access: " << path << " - " << mode << std::endl;
        auto op = std::make_shared<IOProxyMetadataOP>();
        op->id = 11;
        op->operation = IOProxyMetadataOP::access;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_METADATA;
        op->mode = mode;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_utimes(const std::string path, long int sec0n, long int sec0s, long int sec1n, long int sec1s){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "utimes: " << path << std::endl;
        auto op = std::make_shared<IOProxyMetadataOP>();
        op->id = 12;
        op->operation = IOProxyMetadataOP::utimes;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_METADATA;
        op->sec1s = sec1s;
        op->sec1n = sec1n;
        op->sec0s = sec0s;
        op->sec0n = sec0n;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_symlink(const std::string path, const std::string newpath){
        std::string fullpath = hvsfs_fullpath(path);
        std::string newfullpath = hvsfs_fullpath(newpath);
        std::cout << "symlink: " << fullpath << " - " << path << " <=> " << newpath << std::endl;
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 13;
        op->operation = IOProxyDataOP::symlink;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->newpath = newfullpath.c_str();
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline std::string ioproxy_readlink(const std::string path, size_t size){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "readlink: " << fullpath << " - " << path << std::endl;
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 14;
        op->operation = IOProxyDataOP::readlink;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->size = size;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->linkbuf;
    }

    inline int ioproxy_chmod(const std::string path, mode_t mode){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "chmod: " << fullpath << " - " << path << " " << mode << std::endl;
        auto op = std::make_shared<IOProxyMetadataOP>();
        op->id = 14;
        op->operation = IOProxyMetadataOP::chmod;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_METADATA;
        op->mode = mode;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_chown(const std::string path, uid_t uid, gid_t gid){
        std::string fullpath = hvsfs_fullpath(path);
        std::cout << "chown: " << fullpath << " - " << path << std::endl;
        auto op = std::make_shared<IOProxyMetadataOP>();
        op->id = 15;
        op->operation = IOProxyMetadataOP::chown;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_METADATA;
        op->uid = uid;
        op->gid = gid;
        static_cast<IOProxy*>(hvs::HvsContext::get_context()->node)->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_heartbeat() {
        dout(15) << "INFO: heartbeat from manager" << dendl;
        return 1;
    }

    inline void hvs_ioproxy_rpc_bind(RpcServer* rpc_server) {
        rpc_server->bind("ioproxy_stat", ioproxy_stat);
        rpc_server->bind("ioproxy_read", ioproxy_read);
        rpc_server->bind("ioproxy_write", ioproxy_write);
        rpc_server->bind("ioproxy_open", ioproxy_open);
        rpc_server->bind("ioproxy_close", ioproxy_close);
        rpc_server->bind("ioproxy_opendir", ioproxy_opendir);
        rpc_server->bind("ioproxy_readdir", ioproxy_readdir);
        rpc_server->bind("ioproxy_truncate", ioproxy_truncate);
        rpc_server->bind("ioproxy_rename", ioproxy_rename);
        rpc_server->bind("ioproxy_mkdir", ioproxy_mkdir);
        rpc_server->bind("ioproxy_rmdir", ioproxy_rmdir);
        rpc_server->bind("ioproxy_create", ioproxy_create);
        rpc_server->bind("ioproxy_unlink", ioproxy_unlink);
        rpc_server->bind("ioproxy_link", ioproxy_link);
        rpc_server->bind("ioproxy_access", ioproxy_access);
        rpc_server->bind("ioproxy_utimes", ioproxy_utimes);
        rpc_server->bind("ioproxy_symlink", ioproxy_symlink);
        rpc_server->bind("ioproxy_readlink", ioproxy_readlink);
        rpc_server->bind("ioproxy_chmod", ioproxy_chmod);
        rpc_server->bind("ioproxy_chown", ioproxy_chown);
        rpc_server->bind("ioproxy_stat_multi", ioproxy_stat);
        rpc_server->bind("ioproxy_heartbeat", ioproxy_heartbeat);
    }
}