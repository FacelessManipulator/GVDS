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
#include <vector>
#include <limits.h>

namespace hvs {
    inline std::string hvsfs_fullpath(const char *path) {
        // 此函数用来拼接路径。
        char fpath[PATH_MAX];
        strcpy(fpath, hvs::HvsContext::get_context()->ioproxy_rootdir.c_str());
        strncat(fpath, path, PATH_MAX);
        return std::string(fpath);
    }


    inline ioproxy_rpc_statbuffer ioproxy_stat(const std::string pathname) {
        std::string fullpath = hvsfs_fullpath(pathname.c_str());
        auto op = std::make_shared<hvs::IOProxyMetadataOP>();
        op->id = 0;
        op->operation = hvs::IOProxyMetadataOP::stat;
        op->path = fullpath.c_str();
        op->type = hvs::IO_PROXY_METADATA;
        hvs::HvsContext::get_context()->_ioproxy->queue_and_wait(op);
        if (op->error_code == 0) {
            return ioproxy_rpc_statbuffer(op->buf); //返回消息
        } else {
            return ioproxy_rpc_statbuffer(op->error_code);
        }
    }

    inline ioproxy_rpc_readbuffer ioproxy_read(const std::string pathname, int size, int offset){
        std::string fullpath = hvsfs_fullpath(pathname.c_str());
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 1;
        op->operation = IOProxyDataOP::read;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->should_prepare = true;
        op->size = static_cast<size_t>(size);
        op->offset = offset;
        hvs::HvsContext::get_context()->_ioproxy->queue_and_wait(op);
        if (op->error_code >= 0) {
            return ioproxy_rpc_readbuffer(op->obuf, static_cast<int>(op->error_code)); //返回消息
        } else {
            return ioproxy_rpc_readbuffer(op->error_code);
        }
    }

    inline int ioproxy_write(const std::string pathname, const std::string buf,int size, int offset){
        std::string fullpath = hvsfs_fullpath(pathname.c_str());
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 2;
        op->operation = IOProxyDataOP::write;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_DATA;
        op->size = static_cast<size_t>(size);
        op->offset = offset;
        op->ibuf = buf.c_str();
        hvs::HvsContext::get_context()->_ioproxy->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_open(const std::string pathname){
        std::string fullpath = hvsfs_fullpath(pathname.c_str());
        std::cout << "open: " << fullpath.c_str() << std::endl;
        return 0;
    }

    inline int ioproxy_close(const int fd){
        std::cout << "close fd: " << fd << std::endl;
        return 0;
    }

    inline int ioproxy_opendir(const std::string pathname){
        std::string fullpath = hvsfs_fullpath(pathname.c_str());
        std::cout << "opendir: " << fullpath.c_str() << std::endl;
        return 0;
    }

    inline std::vector<ioproxy_rpc_dirent> ioproxy_readdir(const std::string pathname){
        std::string fullpath = hvsfs_fullpath(pathname.c_str());
        std::vector<ioproxy_rpc_dirent> retvec;
        std::cout << "readdir: " << pathname << std::endl;
        auto op = std::make_shared<IOProxyMetadataOP>();
        op->id = 0;
        op->operation = IOProxyMetadataOP::readdir;
        op->path = fullpath.c_str();
        op->type = IO_PROXY_METADATA;
        hvs::HvsContext::get_context()->_ioproxy->queue_and_wait(op);
        if (op->error_code == 0) {
            for(dirent ent : op->dirvector){
                retvec.emplace_back(ioproxy_rpc_dirent(&ent));
            }
        } else {
            retvec.emplace_back(ioproxy_rpc_dirent(op->error_code));
            return retvec;
        }
    }

    inline void hvs_ioproxy_rpc_bind(RpcServer* rpc_server) {
        rpc_server->bind("ioproxy_stat", ioproxy_stat);
        rpc_server->bind("ioproxy_read", ioproxy_read);
        rpc_server->bind("ioproxy_write", ioproxy_write);
        rpc_server->bind("ioproxy_open", ioproxy_open);
        rpc_server->bind("ioproxy_close", ioproxy_close);
        rpc_server->bind("ioproxy_opendir", ioproxy_opendir);
        rpc_server->bind("ioproxy_readdir", ioproxy_readdir);
    }
}