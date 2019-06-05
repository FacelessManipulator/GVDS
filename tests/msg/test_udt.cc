#include "context.h"
#include "gtest/gtest.h"
#include <unistd.h>
#include <chrono>
#include "io_proxy/rpc_types.h"
#include "msg/pack.h"
#include "msg/udt_client.h"
#include <vector>
#include <dirent.h>
using namespace std;
using namespace hvs;
#define  TFILEP "/syncio.txt"

class IOProxyUDT : public ::testing::Test {
 protected:
  void SetUp() override {
    // ioproxy = static_cast<IOProxy*>(HvsContext::get_context()->node);
    // ASSERT_NE(ioproxy, nullptr);
  }
  void TearDown() override { }

 protected:
  static void SetUpTestCase() {
    hvs::init_context();
    // hvs::init_ioproxy();
  }
  static void TearDownTestCase() {
    // hvs::destroy_ioproxy(
    //     static_cast<IOProxy*>(HvsContext::get_context()->node));
    hvs::destroy_context();
  }

 public:
  // IOProxy* ioproxy;
};


//TEST_F(IOProxyUDT, simple) {
//    UDTClient client;
//    auto session = client.create_session("127.0.0.1", 9095);
//    ASSERT_TRUE(session);
//
//    string pathname(TFILEP);
//    char* buf = new char[7]; //100KB
//    memcpy(buf, "hello\n", 6);
//    ioproxy_rpc_buffer _buffer(pathname.c_str(), buf, 0, 7);
//    int id = session->write(_buffer);
//    session->wait_op(id);
//    client.close_session(session);
//}

TEST_F(IOProxyUDT, write_bench_sm) {
    UDTClient client;
    auto session = client.create_session("127.0.0.1", 9095);
    ASSERT_TRUE(session);

    string pathname(TFILEP);
    unsigned long buf_size = 102400;
    char* buf = new char[buf_size]; //100KB
    memcpy(buf, "hello\n", 6);
    ioproxy_rpc_buffer _buffer(pathname.c_str(), buf, 0, buf_size);
    ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
    unsigned size  = 10; // 100KB*1000=100MB

    auto start = std::chrono::steady_clock::now();
    int id;
    for(int i = 0; i < size; i++){
        ioproxy_rpc_buffer _buffer(pathname.c_str(), buf, buf_size*i, buf_size);
        id = session->write(_buffer);
        dout(-1) << "op-" << id << " write on client" << dendl;
        session->wait_op(id);
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "speed: " << size*buf_size/1000 / diff.count() << " KB/s\n";
}