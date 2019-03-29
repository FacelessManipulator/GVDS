#include "context.h"
#include "datastore/datastore.h"
#include "gtest/gtest.h"

class DatastoreTest : public ::testing::Test {
 protected:
  void SetUp() override {
    hvs::init_context();
    dbPtr = hvs::DatastoreFactory::create_datastore(
        "test", hvs::DatastoreType::couchbase);
    EXPECT_EQ(dbPtr->init(), 0);
  }
  void TearDown() override { hvs::destroy_context(); }

 public:
  std::shared_ptr<hvs::Datastore> dbPtr;
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
  tie(vp, std::ignore) = dbPtr->get(key);
  EXPECT_EQ("", *vp);
}