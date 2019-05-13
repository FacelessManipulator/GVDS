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
#include <vector>
#include <dirent.h>
using namespace std;
using namespace hvs;
#define  TFILEP "/syncio.txt"
#define  TDIRP "/."

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

TEST(IOProxyRPC, yx_ioproxy_read) {
    string pathname(TFILEP);
    hvs::init_context();
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 1;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        auto res = client.call("ioproxy_read", pathname, 6, 0);
        dout(-1) << res->as<ioproxy_rpc_buffer>().buf.ptr << dendl;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote op " << size << " read: " << diff.count() << " s\n";
}

TEST(IOProxyRPC, yx_ioproxy_write) {
    string pathname(TFILEP);
    hvs::init_context();
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 1;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        std::string data("superman");
        auto res = client.call("ioproxy_write", pathname, data, data.size(), 0);
        dout(-1) << res->as<int>() << dendl;
        std::cout << "输出结果：" << res->as<int>() << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote op " << size << " write: " << diff.count() << " s\n";
}

TEST(IOProxyRPC, yx_ioproxy_open){
    string pathname(TFILEP);
    hvs::init_context();
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 1;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        auto res = client.call("ioproxy_open", pathname);
        dout(-1) << res->as<int>() << dendl;
        std::cout << "输出结果：" << res->as<int>() << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote op " << size << " open: " << diff.count() << " s\n";
}

TEST(IOProxyRPC, yx_ioproxy_close) {
    string pathname(TFILEP);
    hvs::init_context();
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 1;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        auto res = client.call("ioproxy_close", 0);
        dout(-1) << res->as<int>() << dendl;
        std::cout << "输出结果：" << res->as<int>() << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote op " << size << " close: " << diff.count() << " s\n";
}

TEST(IOProxyRPC, yx_ioproxy_opendir) {
    string pathname(TFILEP);
    hvs::init_context();
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 1;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        auto res = client.call("ioproxy_opendir", pathname);
        dout(-1) << res->as<int>() << dendl;
        std::cout << "输出结果：" << res->as<int>() << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote op " << size << " opendir: " << diff.count() << " s\n";
}

TEST(IOProxyRPC, yx_ioproxy_readdir) {
    string pathname(TDIRP);
    hvs::init_context();
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 1;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        auto res = client.call("ioproxy_readdir", pathname);
        dout(-1) << res->as<vector<ioproxy_rpc_dirent>>()[0].d_name << dendl;
        //std::cout << "输出结果：" << res->as<int>() << std::endl;
        for (const ioproxy_rpc_dirent &ent : res->as<vector<ioproxy_rpc_dirent>>()) {
            std::cout << ent.d_name << std::endl;
        }
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote op " << size << " readdir: " << diff.count() << " s\n";
}


TEST(IOProxyRPC, yx_ioproxy_pack) {
    hvs::init_context();
    hvs::init_ioproxy();
    ioproxy_rpc_statbuffer sb1 = ioproxy_stat("/tmp/hvs/tests/data/example.cfg");
    auto buffer = pack(sb1);
    ioproxy_rpc_statbuffer sb2;
    sb2 = unpack<ioproxy_rpc_statbuffer>(buffer);
    EXPECT_EQ(sb1.st_ino, sb2.st_ino);
}