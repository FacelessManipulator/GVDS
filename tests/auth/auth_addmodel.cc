//=============
#include <iostream>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"



// #include "manager/usermodel/Account.h"
 #include "manager/authmodel/AuthModelServer.h"

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>

#include "manager/manager.h"

//#include "include/context.h"
using namespace Pistache;
using namespace Pistache::Http;
using namespace hvs;
using namespace std;

class HVSAuthTest : public ::testing::Test {
 protected:
  void SetUp() override {
    manager = static_cast<Manager*>(HvsContext::get_context()->node);
    ASSERT_NE(manager, nullptr);
  }
  void TearDown() override { manager = nullptr; }

 protected:
  static void SetUpTestCase() {
    hvs::init_context();
    hvs::init_manager();
    usleep(100000); // wait 100 ms. rest server may started.
  }
  static void TearDownTestCase() {
    hvs::destroy_manager(
        static_cast<Manager*>(HvsContext::get_context()->node));
    hvs::destroy_context();
  }

 public:
  Manager* manager;
};


TEST_F(HVSAuthTest, auth_addmodel) {
    
    AuthModelServer *p_auth = static_cast<AuthModelServer*>(manager->get_module("auth").get());
    // //1、权限增加模块
    // //区域初始权限记录接口
    // cout << "开始：区域初始权限记录接口" << endl;
    string zoneID = "1809d61f-5c6a-4ca2-928c-fe135e933128";
    string ownerID = "127";
    // int spaceauthadd = p_auth->ZonePermissionAdd(zoneID, ownerID);
    
    // if (spaceauthadd == 0) cout << "success: 区域初始权限记录接口" << endl;
    // else cout << "fail: 区域初始权限记录接口" << endl;
    // cout << "结束：区域初始权限记录接口" << endl << endl;
    // cout << endl;


    //空间权限同步接口
    cout << "开始：空间权限同步接口" << endl;
    string spaceID = "b71268dc-c8ad-4cf0-a7a3-6e9c262b36ce";    // 那个zoneID 对应了三个 spaceID 这块测试的时候需要调用 三回，设置用户、用户组、权限
    int spacesyne = p_auth->SpacePermissionSyne(spaceID, zoneID, ownerID);
    
    if (spacesyne == 0) cout << "success: 空间权限同步接口" << endl;
    else cout << "fail: 空间权限同步接口" << endl;
    cout << "结束：空间权限同步接口" << endl << endl;


    // //成员权限增加接口
    // cout << "开始：成员权限增加接口" << endl;
    // vector<string> memberID;
    // memberID.push_back("128");   // 127   add  128
    // int memadd = p_auth->ZoneMemberAdd(zoneID, ownerID, memberID);
    // if (memadd == 0) cout << "success: 成员权限增加接口" << endl;
    // else cout << "fail: 成员权限增加接口" << endl;
    // cout << "结束：成员权限增加接口" << endl << endl;
    // // ownerID:127 zhongkeyuan(shanghai) test1    ---     ownerID:128 zhongkeyuan(shanghai) test10
    // //运行命令: groups test10  
    // //显示:  [test10: test10, test1]   表明成功

    // //2、权限删除模块
    // //2.1 区域权限删除接口

    // //2.2 成员权限删除接口
    // cout << "开始：成员权限删除接口" << endl;
    // int memdel = p_auth->ZoneMemberDel(zoneID, ownerID, memberID);
    // if (memdel == 0) cout << "success: 成员权限删除接口" << endl;
    // else cout << "fail: 成员权限删除接口" << endl;
    // cout << "结束：成员权限删除接口" << endl << endl;
    // // 127 zhongkeyuan(shanghai) test1    ---     128 zhongkeyuan(shanghai) test10
    // //运行命令: groups test10  
    // //显示:  [test10: test10]   表明成功

    // //注意：有三个空间，服务端内部发送了3次rest请求，但后两次会显示 gpasswd: user 'test10' is not a member of 'test1'
    // //这是因为三个空间在一个地方，第一次rest请求已经将用户从组中剔除了，所以显示这个没有问题

    // //2.4 空间权限删除接口 
    // cout << "开始：空间权限删除接口" << endl;
    // int spaceauthdel = p_auth->SpacePermissionDelete(spaceID);
    // if (spaceauthdel == 0) cout << "success: 空间权限删除接口" << endl;
    // else cout << "fail: 空间权限删除接口" << endl;
    // cout << "结束：空间权限删除接口" << endl << endl;
    // 查看 spaceID 的权限
    // 显示: [root : root]   表明成功

  /*
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/users/login", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  
  std::string mes = "{\"HVSAccountName\":\"lbq-7\",\"HVSPassword\":\"123456\"}";
  std::string mtoken;


  auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(mes).send();
        dout(-1) << "Client Info: post request " << url << dendl;

  std::promise<bool> prom;
  auto fu = prom.get_future();
  response.then(
      [&](Http::Response res) {
        //dout(-1) << "Manager Info: " << res.body() << dendl;
        std::cout << "Response code = " << res.code() << std::endl;
        auto body = res.body();
        if (!body.empty()){
            std::cout << "Response body = " << body << std::endl;
            //====================
            //your code write here

            //====================
        }
        std::cout<< "Response cookie = ";
        auto cookies = res.cookies();
        for (const auto& c: cookies) {
            std::cout << c.name << " : " << c.value << std::endl;
            mtoken = c.value;
        }
        prom.set_value(true);
      },
      Async::IgnoreException);
  EXPECT_TRUE(fu.get());

  sleep(5);
  //cancel
  snprintf(url, 256, "http://localhost:%d/users/cancel/127", manager->rest_port());
  auto response_1 = client.get(url).cookie(Http::Cookie("token", mtoken)).send();
        dout(-1) << "Client Info: get request " << url << dendl;

  std::promise<bool> prom_1;
  auto fu_1 = prom_1.get_future();
  response_1.then(
      [&](Http::Response res) {
        //dout(-1) << "Manager Info: " << res.body() << dendl;
        std::cout << "Response code = " << res.code() << std::endl;
        auto body = res.body();
        if (!body.empty()){
            std::cout << "Response body = " << body << std::endl;
            //====================
            //your code write here

            //====================
        }
        prom_1.set_value(true);
      },
      Async::IgnoreException);
  EXPECT_TRUE(fu_1.get());

  client.shutdown();
  */
}
