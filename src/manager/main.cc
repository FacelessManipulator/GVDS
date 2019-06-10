#include <csignal>
#include "context.h"
#include "manager/manager.h"

using namespace hvs;
using namespace std;

void sig_exit(int sig) {
  dout(-1) << "receive signal " << sig << ", exiting..." << dendl;
  hvs::destroy_manager(static_cast<Manager*>(HvsContext::get_context()->node));
  hvs::destroy_context();
  exit(1);
}

int main() {
  init_context();
  hvs::Manager* mgr = init_manager();
  sleep(1);
  dout(-1) << "manager server {" << mgr->uuid << "} frontend started at "
           << mgr->rest_port() << dendl;
  std::signal(SIGINT, sig_exit);
  std::signal(SIGTERM, sig_exit);
  mgr->join();
}