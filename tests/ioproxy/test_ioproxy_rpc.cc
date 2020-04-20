//
// Created by yaowen on 5/4/19.
// 北航系统结构所-存储组
//

#include "gvds_context.h"
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
#include "io_proxy/sync_io.h"
using namespace std;
using namespace gvds;
#define  TFILEP "/syncio.txt"
#define  TDIRP "/."

class IOProxyRPC : public ::testing::Test {
 protected:
  void SetUp() override {
    ioproxy = static_cast<IOProxy*>(HvsContext::get_context()->node);
    ASSERT_NE(ioproxy, nullptr);
  }
  void TearDown() override { ioproxy = nullptr; }

 protected:
  static void SetUpTestCase() {
    gvds::init_context();
    gvds::init_ioproxy();
  }
  static void TearDownTestCase() {
    gvds::destroy_ioproxy(
        static_cast<IOProxy*>(HvsContext::get_context()->node));
    gvds::destroy_context();
  }

 public:
  IOProxy* ioproxy;
};


TEST_F(IOProxyRPC, yx_ioproxy_simple) {
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    auto res = client.call("1");
    ASSERT_TRUE(res);
    ASSERT_EQ((*res).as<int>(), 1);
}

TEST_F(IOProxyRPC, yx_ioproxy_stat) {
    string pathname(TFILEP);
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
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

TEST_F(IOProxyRPC, yx_ioproxy_read) {
    string pathname(TFILEP);
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
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

TEST_F(IOProxyRPC, yx_ioproxy_write) {
    string pathname(TFILEP);
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 1;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        std::string data("superman");
    ioproxy_rpc_buffer _buffer(pathname.c_str(), data.c_str(), 0, data.size());
        auto res = client.call("ioproxy_write", pathname, _buffer, data.size(), 0);
        dout(-1) << res->as<int>() << dendl;
        std::cout << "输出结果：" << res->as<int>() << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote op " << size << " write: " << diff.count() << " s\n";
}

TEST_F(IOProxyRPC, yx_ioproxy_open){
    string pathname(TFILEP);
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
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

TEST_F(IOProxyRPC, yx_ioproxy_close) {
    string pathname(TFILEP);
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
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

TEST_F(IOProxyRPC, yx_ioproxy_opendir) {
    string pathname(TFILEP);
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
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

TEST_F(IOProxyRPC, yx_ioproxy_readdir) {
    string pathname(TDIRP);
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
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


TEST_F(IOProxyRPC, yx_ioproxy_pack) {
    ioproxy_rpc_statbuffer sb1 = ioproxy_stat("/tmp/gvds/tests/data/example.cfg");
    auto buffer = pack(sb1);
    ioproxy_rpc_statbuffer sb2;
    sb2 = unpack<ioproxy_rpc_statbuffer>(buffer);
    EXPECT_EQ(sb1.st_ino, sb2.st_ino);
}

TEST_F(IOProxyRPC, yx_ioproxy_write_bench) {
    string pathname(TFILEP);
    char* buf = new char[102400]; //100KB
    ioproxy_rpc_buffer _buffer(pathname.c_str(), buf, 0, 102400);
    ConfigureSettings* config = gvds::HvsContext::get_context()->_config;
    auto ip = config->get<string>("ip");
    auto port = config->get<int>("rpc.port");
    RpcClient client(*ip, static_cast<const unsigned int>(*port));
    unsigned size  = 100; // 100KB*100=10MB

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++){
        auto res = client.call("ioproxy_write", pathname, _buffer, 102400, 0);
        // dout(-1) << res->as<int>() << dendl;
        // std::cout << "输出结果：" << res->as<int>() << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "speed: " << size*100 / diff.count() << " KB/s\n";
}