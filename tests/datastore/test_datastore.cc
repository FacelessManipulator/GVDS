#include "gvds_context.h"
#include "datastore/datastore.h"
#include "gtest/gtest.h"

class DatastoreTest : public ::testing::Test {
 protected:
  void SetUp() override {
    dbPtr = gvds::DatastoreFactory::create_datastore(
        "test", gvds::DatastoreType::couchbase);
    EXPECT_EQ(dbPtr->init(), 0);
  }
  void TearDown() override {}
  static void SetUpTestCase() { gvds::init_context(); }
  static void TearDownTestCase() { gvds::destroy_context(); }

 public:
  std::shared_ptr<gvds::Datastore> dbPtr;
};

TEST_F(DatastoreTest, CouchbaseInit) {
  EXPECT_EQ("couchbase", dbPtr->get_typename());
}

TEST_F(DatastoreTest, CouchbaseCURD) {
  std::string key = "test_key";
  std::string value = "test_value";
  EXPECT_EQ(0, dbPtr->set(key, value));
  auto [vp, err] = dbPtr->get(key);
  EXPECT_EQ(value, *vp);
  err = dbPtr->remove(key);
  EXPECT_EQ(0, err);
  tie(vp, err) = dbPtr->get(key);
  // TODO: transfer to standard error code, ENOENT
  EXPECT_EQ(13, err);
}

class SharedClientTest : public Thread {
 public:
  void* entry() override {
    auto dbp_last = gvds::DatastoreFactory::create_datastore(
        "test", gvds::DatastoreType::couchbase, true);
    for (int i = 0; i < 10; i++) {
      auto dbp_cur = gvds::DatastoreFactory::create_datastore(
        "test", gvds::DatastoreType::couchbase, true);
      EXPECT_EQ(dbp_last, dbp_cur);
    }
  }
};

TEST_F(DatastoreTest, SharedClient) {
  SharedClientTest sct[10];
  char name[32];
  for (int i = 0; i < 10; i++) {
    snprintf(name, 32, "SharedClientTest-%d", i);
    sct[i].create(name);
  }
  for (int i = 0; i < 10; i++) {
    sct[i].join();
  }
  EXPECT_EQ("couchbase", dbPtr->get_typename());
}
