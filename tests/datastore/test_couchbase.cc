#include "common/json.h"
#include "context.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"

using namespace std;

class CouchbaseTest : public ::testing::Test {
 protected:
  void SetUp() override {
    dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("beer-sample"));
    ASSERT_EQ(dbPtr->init(), 0);
  }
  void TearDown() override { dbPtr.reset(); }
  static void SetUpTestCase() { hvs::init_context(); }
  static void TearDownTestCase() { hvs::destroy_context(); }

 public:
  std::shared_ptr<hvs::CouchbaseDatastore> dbPtr;
};

TEST_F(CouchbaseTest, Init) { EXPECT_EQ("couchbase", dbPtr->get_typename()); }

TEST_F(CouchbaseTest, CURD) {
  std::string key = "test_key";
  std::string value = "test_value";
  EXPECT_EQ(0, dbPtr->set(key, value));
  auto [vp, err] = dbPtr->get(key);
  EXPECT_EQ(value, *vp);
  EXPECT_EQ(0, dbPtr->remove(key));
  tie(vp, err) = dbPtr->get(key);
  EXPECT_EQ(13, err);
}

TEST_F(CouchbaseTest, SubCommand) {
  std::string key = "21st_amendment_brewery_cafe";
  std::string path = "geo.lat2";
  std::string value = "37.7825";
  dbPtr->set(key, path, value);
  auto [vp, err] = dbPtr->get(key, path);
  EXPECT_EQ(value, *vp);
}

TEST_F(CouchbaseTest, N1QL) {
  // correct syntax
  std::string query =
      "select * from `beer-sample` where country = \"United States\" limit 5;";
  auto [vp, err] = dbPtr->n1ql(query);
  EXPECT_EQ(vp->size(), 5);

  // wrong syntax
  query =
      "select ** from `beer-sample` where country = \"United States\" limit 5;";
  tie(vp, err) = dbPtr->n1ql(query);
  EXPECT_EQ(-EINVAL, err);
}