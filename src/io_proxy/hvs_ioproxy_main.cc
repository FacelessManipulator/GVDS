//
// Created by yaowen on 5/4/19.
// 北航系统结构所-存储组
//

#include <unistd.h>
#include <iostream>
#include "context.h"
#include "io_proxy/io_proxy.h"
#include "msg/rpc.h"
#include "rpc_bindings.hpp"

int main(int argc, char* argv[]) {
  hvs::init_context();
  std::cout << "IOProxy Node Context initilized." << std::endl;
  hvs::IOProxy* ioproxy = hvs::init_ioproxy();
  if(ioproxy) {
    usleep(100);      // give some time waiting the reset of state machine
    std::cout << "IOProxy Node {" << ioproxy->uuid << "} is running." << std::endl;
    ioproxy->join();  // 等待IO代理退出
    hvs::destroy_ioproxy(
        static_cast<hvs::IOProxy*>(hvs::HvsContext::get_context()->node));
  }
  hvs::destroy_context();
  return 0;
}
