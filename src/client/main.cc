#include <iostream>
#include <csignal>
#include "context.h"
#include "msg/op.h"
#include "msg/rpc.h"
#include "client/client.h"
#include "client/zone_mod.h"
#include <thread>

using namespace hvs;
using namespace std;

int zonechecker_run = true;

void signal_handler(int signal)
{
  static_cast<Client*>(HvsContext::get_context()->node)->stop();
  exit(1);
}

int main() {
  init_context();
  auto client = init_client();
  signal(SIGINT, signal_handler);
  client->join();
}
