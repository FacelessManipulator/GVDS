#include "context.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"

using namespace std;

class CouchbaseTest : public ::testing::Test {
 protected:
  void SetUp() override {
    hvs::init_context();
    dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("beer-sample"));
    EXPECT_EQ(dbPtr->init(), 0);
  }
  void TearDown() override {
    dbPtr.reset();
    hvs::destroy_context();
  }

 public:
  std::shared_ptr<hvs::CouchbaseDatastore> dbPtr;
};

TEST_F(CouchbaseTest, Init) { EXPECT_EQ("couchbase", dbPtr->get_typename()); }

TEST_F(CouchbaseTest, CURD) {
  dbPtr->init();
  std::string key = "test_key";
  std::string value = "test_value";
  EXPECT_EQ(0, dbPtr->set(key, value));
  EXPECT_EQ(value, dbPtr->get(key));
  EXPECT_EQ(0, dbPtr->remove(key));
  EXPECT_EQ("", dbPtr->get(key));
}

TEST_F(CouchbaseTest, Command) {
  std::string key = "21st_amendment_brewery_cafe";
  std::string value = "37.7825";
  EXPECT_EQ(value, dbPtr->get(key, "geo.lat"));
}

TEST_F(CouchbaseTest, N1QL) {
  // wrong syntax 
  std::string query = "select ** from `beer-sample` where country = \"United States\" limit 5;";
  std::cout << dbPtr->n1ql(query) << endl;
  EXPECT_TRUE(1);
}