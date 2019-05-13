#include <iostream>
#include "context.h"
#include "msg/op.h"
#include "msg/rpc.h"

using namespace hvs;
using namespace std;

int main() {
  init_context();
  RpcClient client("127.0.0.1", 9092);
  vector<string> pathnames;
  long size = 100;
  for (int i = 0; i < size; i++) {
    pathnames.emplace_back("haha");
  }
  auto start = std::chrono::steady_clock::now();
    auto res = client.call("ioproxy_stat_multi", pathnames);
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "queue op time " << diff.count() << " s\n";
}
