#include <csignal>
#include "context.h"
#include "manager/manager.h"
#include <boost/program_options.hpp>

using namespace hvs;
using namespace std;
namespace po = boost::program_options;

void sig_exit(int sig)
{
  dout(-1) << "receive signal " << sig << ", exiting..." << dendl;
  hvs::destroy_manager(static_cast<Manager *>(HvsContext::get_context()->node));
  hvs::destroy_context();
  exit(1);
}

po::variables_map parse_cmd(int argc, char *const argv[])
{
  // Declare the supported options.
  po::options_description desc("GVDS Manager daemon program");
  desc.add_options()
    ("help", "get help infomation from GVDS manager.")
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

int main(int argc, char *const argv[])
{
  string config_path;
  auto vm = parse_cmd(argc, argv);
  if (vm.count("help")) {
    return 0;
  } else if (vm.count("config")) {
    config_path = vm["config"].as<string>();
  }
  init_context(config_path);
  hvs::Manager *mgr = init_manager();
  sleep(1);
  dout(-1) << "manager server {" << mgr->uuid << "} frontend started at "
           << mgr->rest_port() << dendl;
  std::signal(SIGINT, sig_exit);
  std::signal(SIGTERM, sig_exit);
  mgr->join();
}