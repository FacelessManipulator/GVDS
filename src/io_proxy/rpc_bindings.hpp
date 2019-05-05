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

namespace hvs {

    inline ioproxy_rpc_statbuffer ioproxy_stat(const std::string pathname) {
        struct stat _st{};
        int ret;
        auto op = std::make_shared<hvs::IOProxyMetadataOP>();
        op->id = 0;
        op->operation = hvs::IOProxyMetadataOP::stat;
        op->path = pathname.c_str();
        op->type = hvs::IO_PROXY_METADATA;
        hvs::HvsContext::get_context()->_ioproxy->queue_and_wait(op);
//        std::cout << op->buf->st_ino  << std::endl;
        if (op->error_code == 0) {
            return ioproxy_rpc_statbuffer(op->buf); //返回消息
        } else {
            return ioproxy_rpc_statbuffer(op->error_code);
        }
    }

    inline ioproxy_rpc_readbuffer ioproxy_read(const std::string pathname, int size, int offset){
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 1;
        op->operation = IOProxyDataOP::read;
        op->path = pathname.c_str();
        op->type = IO_PROXY_DATA;
        op->should_prepare = true;
        op->size = static_cast<size_t>(size);
        op->offset = offset;
        hvs::HvsContext::get_context()->_ioproxy->queue_and_wait(op);
        //std::cout << "查找结果: " << op->obuf << std::endl;
        if (op->error_code >= 0) {
            return ioproxy_rpc_readbuffer(op->obuf, static_cast<int>(op->size)); //返回消息
        } else {
            return ioproxy_rpc_readbuffer(op->error_code);
        }
    }

    inline int ioproxy_write(const std::string pathname, const std::string buf,int size, int offset){
        auto op = std::make_shared<IOProxyDataOP>();
        op->id = 2;
        op->operation = IOProxyDataOP::write;
        op->path = pathname.c_str();
        op->type = IO_PROXY_DATA;
        op->size = static_cast<size_t>(size);
        op->offset = offset;
        op->ibuf = buf.c_str();
        hvs::HvsContext::get_context()->_ioproxy->queue_and_wait(op);
        return op->error_code;
    }

    inline int ioproxy_open(const std::string pathname){
        std::cout << "open: " << pathname << std::endl;
        return 0;
    }

    inline int ioproxy_close(const int fd){
        std::cout << "close fd: " << fd << std::endl;
        return 0;
    }

    inline int ioproxy_opendir(const std::string pathname){
        std::cout << "opendir: " << pathname << std::endl;
        return 0;
    }

    inline std::vector<ioproxy_rpc_dirent> ioproxy_readdir(const std::string pathname){
        std::vector<ioproxy_rpc_dirent> retvec;
        std::cout << "readdir: " << pathname << std::endl;
        auto op = std::make_shared<IOProxyMetadataOP>();
        op->id = 0;
        op->operation = IOProxyMetadataOP::readdir;
        op->path = pathname.c_str();
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