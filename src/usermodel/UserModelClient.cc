/*
Author:Liubingqi
date:2019.03.22

g++ UserModelClient.cc -lpistache  -std=c++11 -o userclient
./client http://localhost:9080 3
*/

#include <atomic>

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/client.h>

using namespace Pistache;
using namespace Pistache::Http;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: http_client page [count]" << std::endl;
        return 1;
    }

    // 第二个参数传地址 第三个参数传请求数量 默认1
    std::string page = argv[1];
    int count = 1;
    if (argc == 3) {
        count = std::stoi(argv[2]);
    }

    Http::Client client;

    auto opts = Http::Client::options()
        .threads(1)
        .maxConnectionsPerHost(8);
    client.init(opts);

    //
    std::vector<Async::Promise<Http::Response>> responses;

    std::atomic<size_t> completedRequests(0);
    std::atomic<size_t> failedRequests(0);

    //计时
    auto start = std::chrono::system_clock::now();

    for (int i = 0; i < count; ++i) {
        //auto resp = client.get(page).cookie(Http::Cookie("FOO", "bar")).send();
        auto resp = client.post(page).cookie(Http::Cookie("FOO", "bar")).body("a").send();
        resp.then([&](Http::Response response) {
                ++completedRequests;
            std::cout << "Response code = " << response.code() << std::endl;
            //response body
            auto body = response.body();
            if (!body.empty()){
               std::cout << "Response body = " << body << std::endl;
               //====================
               //your code write here

               //====================
            }
        }, Async::IgnoreException);
        responses.push_back(std::move(resp));
    }

    auto sync = Async::whenAll(responses.begin(), responses.end());
    Async::Barrier<std::vector<Http::Response>> barrier(sync);

    barrier.wait_for(std::chrono::seconds(5));

    auto end = std::chrono::system_clock::now();
    std::cout << "Summary of execution" << std::endl
              << "Total number of requests sent     : " << count << std::endl
              << "Total number of responses received: " << completedRequests.load() << std::endl
              << "Total number of requests failed   : " << failedRequests.load() << std::endl
              << "Total time of execution           : "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    client.shutdown();
}