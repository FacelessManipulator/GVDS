#include <iostream>
#include <csignal>
#include "context.h"
#include "msg/op.h"
#include "msg/rpc.h"
#include "client/client.h"
#include "client/zone_mod.h"
#include <thread>
#include <boost/program_options.hpp>

using namespace std;
using namespace hvs;
namespace po = boost::program_options;
po::variables_map parse_cmd(int argc, char *const argv[])
{
  // Declare the supported options.
  po::options_description desc("GVDS IOProxy daemon program");
  desc.add_options()
    ("help", "get help infomation from GVDS IOProxy.")
    ("config,c", po::value<string>(), "set config file path, default is /etc/hvs/hvs.conf or /opt/hvs/hvs.conf");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    cout << desc << endl;
    exit(EXIT_SUCCESS);
  }
  return std::move(vm);
}

int zonechecker_run = true;

void signal_handler(int signal)
{
  static_cast<Client*>(HvsContext::get_context()->node)->stop();
  exit(1);
}

int main(int argc, char* const argv[]) {
  string config_path;
  auto vm = parse_cmd(argc, argv);
  if (vm.count("help")) {
    return 0;
  } else if (vm.count("config")) {
    config_path = vm["config"].as<string>();
  }
  init_context(config_path);
  auto client = init_client();
  signal(SIGINT, signal_handler);
  client->join();
}
