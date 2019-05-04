//
// Created by yaowen on 5/4/19.
// 北航系统结构所-存储组
//

#include "context.h"
#include "gtest/gtest.h"
#include "msg/rpc.h"
#include <unistd.h>
#include <chrono>
#include "config/ConfigureSettings.h"
#include "io_proxy/rpc_types.h"
#include "io_proxy/rpc_bindings.hpp"
#include "msg/pack.h"
using namespace std;
using namespace hvs;
#define  TFILEP "/tmp/hvs/tests/data/syncio.txt"

TEST(IOProxyRPC, yx_ioproxy_simple) {
    hvs::init_context();
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    auto res = client.call("1");
    ASSERT_TRUE(res);
    ASSERT_EQ((*res).as<int>(), 1);
}

TEST(IOProxyRPC, yx_ioproxy_stat) {
    string pathname(TFILEP);
    hvs::init_context();
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 1;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        auto res = client.call("ioproxy_stat", pathname);
        dout(-1) << res->as<ioproxy_rpc_statbuffer>().st_ino << dendl;
        std::cout << "运行到这里！" << res->as<ioproxy_rpc_statbuffer>().st_ino << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote get " << size << " metadata: " << diff.count() << " s\n";
}

TEST(IOProxyRPC, yx_ioproxy_pack) {
    hvs::init_context();
    hvs::HvsContext::get_context()->_ioproxy = hvs::init_ioproxy();
    ioproxy_rpc_statbuffer sb1 = ioproxy_stat("/tmp/hvs/tests/data/example.cfg");
    auto buffer = pack(sb1);
    ioproxy_rpc_statbuffer sb2;
    sb2 = unpack<ioproxy_rpc_statbuffer>(buffer);
    EXPECT_EQ(sb1.st_ino, sb2.st_ino);
}