/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:40:58
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:40:58
 */
#include "common/json.h"
#include "context.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"

using namespace std;
using namespace hvs;

class CouchbaseTest : public ::testing::Test {
 protected:
  void SetUp() override {
    dbPtr = dynamic_pointer_cast<CouchbaseDatastore>(
        DatastoreFactory::create_datastore("test2", couchbase, true));
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
  dbPtr->set(".DB_UNITTEST", "{}");
  std::string key = ".DB_UNITTEST";
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

TEST_F(CouchbaseTest, ArrCommand) {
  dbPtr->set(".TEST_SPACES", "\"replica\":[]");
  auto dbraw = dbPtr->get_raw_client();
  dbraw->arr_insert_uniq(".TEST_SPACES", "replica", "");
  std::string key = ".USER_UUID_MAP";
  std::string path = "admin";
  std::string value = "5d297210-a1dc-41d4-88dd-7fed4a9e2607";
  auto [vp, err] = dbPtr->get(".USER_UUID_MAP", path);
  //    dbPtr->set(key, path, json_encode(value));
  cout << *vp << endl;
  string res;
  json_decode(*vp, res);
  EXPECT_EQ(value, res);
}

TEST_F(CouchbaseTest, SubCommand2) {
  std::string key = ".USER_UUID_MAP";
  std::string path = "admin";
  std::string value = "5d297210-a1dc-41d4-88dd-7fed4a9e2607";
  //    dbPtr->set(key, path, json_encode(value));
  auto [vp, err] = dbPtr->get(".USER_UUID_MAP", path);
  cout << *vp << endl;
  string res;
  json_decode(*vp, res);
  EXPECT_EQ(value, res);
}