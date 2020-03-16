#include "context.h"
#include "gtest/gtest.h"
#include "msg/rpc.h"
#include "msg/stat_demo.h"
#include <unistd.h>
#include <chrono>

using namespace std;
using namespace gvds;

class RPCTEST : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_NE(svr, nullptr);
  }
  void TearDown() override {
    svr = nullptr;
  }

protected:
    static void SetUpTestCase() {
        gvds::init_context();
    }
    static void TearDownTestCase() {
        gvds::destroy_context();
  }
 public:
  RpcServer* svr;
};

TEST_F(RPCTEST, simple) {
    RpcClient client(svr->ip, svr->port);
    auto res = client.call("1");
    ASSERT_TRUE(res);
    ASSERT_EQ((*res).as<int>(), 1);
}

TEST_F(RPCTEST, stat) {
    string pathname("/tmp/gvds/tests/data/example.cfg");
    RpcClient client(svr->ip, svr->port);
    unsigned size  = 10000;
    
    auto start = std::chrono::steady_clock::now();

    for(int i = 0; i < size; i++)
        auto res = client.call("stat_test", pathname);
    //dout(-1) << res->as<stat_buffer>().st_ino << dendl;

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote get " << size << " metadata: " << diff.count() << " s\n";
}

class M_rpc_client : public Thread {
public:
    M_rpc_client(string ip, unsigned port) : _ip(ip), _port(port) {}
    void* entry() override {
        string pathname("/tmp/gvds/tests/data/example.cfg");
        RpcClient client(_ip, _port);
        for(int i = 0; i < 2000; i++)
            client.call("stat_test", pathname);
    }
private:
    string _ip;
    unsigned _port;
};

TEST_F(RPCTEST, MultiStat) {
    vector<M_rpc_client*> vc;
    char name[16];
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 5; i++) {
        vc.emplace_back(new M_rpc_client(svr->ip, svr->port));
        snprintf(name, 16, "RpcTester-%d", i);
        vc.back()->create(name);
    }
    for (auto iter: vc) {
        iter->join();
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to remote get " << 10000 << " metadata in 5 clients: " << diff.count() << " s\n";
}
