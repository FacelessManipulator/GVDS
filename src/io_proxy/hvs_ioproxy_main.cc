//
// Created by yaowen on 5/4/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include <unistd.h>
#include "context.h"
#include "io_proxy/io_proxy.h"
#include "msg/rpc.h"
#include "rpc_bindings.hpp"

void init();
void destroy();
void run();

hvs::IOProxy* ioproxy;

int main(int argc, char* argv[]){
    std::cout << "IO代理节点" << std::endl;
    init();
    run();
    destroy();
    return 0;
}

void run(){
    std::cout << "正在运行！" << std::endl;
}

void init(){
    hvs::init_context();
    hvs::HvsContext::get_context()->_ioproxy = hvs::init_ioproxy();
    // 初始化 rpcserver
    hvs::HvsContext::get_context()->_rpc = hvs::init_rpcserver();
    // 绑定 rpc 函数
    hvs_ioproxy_rpc_bind(hvs::HvsContext::get_context()->_rpc);
    ioproxy = static_cast<hvs::IOProxy*>(hvs::HvsContext::get_context()->node);
    // 设置IO代理根文件夹
    hvs::HvsContext::get_context()->ioproxy_rootdir = "/tmp/hvs/tests/data";
}

void destroy(){
    usleep(100); // give some time waiting the reset of state machine
    ioproxy->join(); // 等待IO代理退出
    ioproxy = nullptr;
    hvs::destroy_ioproxy(static_cast<hvs::IOProxy*>(hvs::HvsContext::get_context()->node));
    hvs::destroy_context();
}