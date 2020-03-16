//
// Created by yaowen on 5/4/19.
// 北航系统结构所-存储组
//

#include <unistd.h>
#include <iostream>
#include "context.h"
#include "io_proxy/io_proxy.h"
#include "msg/rpc.h"
// #include "rpc_bindings.hpp"
#include <boost/program_options.hpp>

using namespace std;
using namespace gvds;
namespace po = boost::program_options;
po::variables_map parse_cmd(int argc, char *const argv[])
{
  // Declare the supported options.
  po::options_description desc("GVDS IOProxy daemon program");
  desc.add_options()
    ("help", "get help infomation from GVDS IOProxy.")
    ("config,c", po::value<string>(), "set config file path, default is /etc/gvds/gvds.conf or /opt/gvds/gvds.conf");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    cout << desc << endl;
    exit(EXIT_SUCCESS);
  }
  return std::move(vm);
}

int main(int argc, char* const argv[]) {
  string config_path;
  auto vm = parse_cmd(argc, argv);
  if (vm.count("help")) {
    return 0;
  } else if (vm.count("config")) {
    config_path = vm["config"].as<string>();
  }
  gvds::init_context(config_path);
  std::cout << "IOProxy Node Context initilized." << std::endl;
  gvds::IOProxy* ioproxy = gvds::init_ioproxy();
  if(ioproxy) {
    usleep(100);      // give some time waiting the reset of state machine
    std::cout << "IOProxy Node {" << ioproxy->uuid << "} is running." << std::endl;
    ioproxy->join();  // 等待IO代理退出
    gvds::destroy_ioproxy(
        static_cast<gvds::IOProxy*>(gvds::HvsContext::get_context()->node));
  }
  gvds::destroy_context();
  return 0;
}
