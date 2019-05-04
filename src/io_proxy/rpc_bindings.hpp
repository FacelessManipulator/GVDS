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

namespace hvs {

    inline ioproxy_rpc_statbuffer ioproxy_stat(const std::string &pathname) {
        struct stat _st{};
        int ret;
        auto op = std::make_shared<hvs::IOProxyMetadataOP>();
        op->id = 0;
        op->operation = hvs::IOProxyMetadataOP::stat;
        op->path = pathname.c_str();
        op->type = hvs::IO_PROXY_METADATA;
        hvs::HvsContext::get_context()->_ioproxy->queue_and_wait(op);
        std::cout << op->buf->st_ino  << std::endl;
        if (op->error_code == 0) {
            return ioproxy_rpc_statbuffer(op->buf); //返回消息
        } else {
            return ioproxy_rpc_statbuffer(op->error_code);
        }
    }


    inline void hvs_ioproxy_rpc_bind(RpcServer* rpc_server) {
        rpc_server->bind("ioproxy_stat", ioproxy_stat);
        //rpc_server->bind("stat", get_stat);
    }
}