#include <iostream>
//#include <boost/program_options.hpp>
#include "config/ConfigureSettings.h"
#include "context.h"
#include "gtest/gtest.h"
#include <iomanip>

using namespace std;
using namespace libconfig;
using namespace gvds;

class ConfigTest : public ::testing::Test {
 protected:
  void SetUp() override {
      init_context();
      csp = HvsContext::get_context()->_config;
  }
  void TearDown() override {
    destroy_context();
    csp = nullptr;
  }

 public:
  ConfigureSettings* csp;
};

TEST_F(ConfigTest, Parse) {
  // path search
  auto mon = csp->get<int>("hours.mon.open");
  EXPECT_TRUE(mon);
  EXPECT_EQ(*mon, 9);

  // error type
  auto _mon = csp->get<int>("hours.mon");
  EXPECT_FALSE(_mon);

  // not exists
  auto _no = csp->get<int>("hours.mons");
  EXPECT_FALSE(_no);

  // int type
  auto age = csp->get<int>("attr.age");
  EXPECT_TRUE(age);
  EXPECT_EQ(*age, 18);
  // float type
  auto height = csp->get<float>("attr.height");
  EXPECT_TRUE(height);
  EXPECT_EQ(*height, 172.5);
  // list type
  auto cats = csp->get_list<string>("attr.cats");
  EXPECT_TRUE(cats.get() != NULL);
  EXPECT_EQ(cats->size(), 2);
  EXPECT_EQ((*cats)[0], "hello");
  // bool type
  auto bo = csp->get<bool>("attr.man");
  EXPECT_TRUE(bo && *bo);
  // string type
  auto name = csp->get<string>("name");
  EXPECT_TRUE(name);
  EXPECT_EQ(*name, "Books, Movies & More");
}

TEST_F(ConfigTest, AddOption) {
    // int type
    csp->add("int", 1);
    // float type
    csp->add("attr2.float", 3.14);
    // bool type
    csp->add("attr2.bool.bool", false);
    // string type
    csp->add("attr2.string", std::string("hello"));
    // list type
    csp->add("attr2.list", std::vector{1,2,3});
    csp->writeFile("/tmp/gvds/tests/data/example.cfg");
}
