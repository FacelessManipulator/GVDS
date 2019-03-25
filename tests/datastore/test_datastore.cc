#include "context.h"
#include "datastore/datastore.h"
#include "gtest/gtest.h"

class DatastoreTest : public ::testing::Test {
 protected:
  void SetUp() override { hvs::init_context(); }
  void TearDown() override { hvs::destroy_context(); }
};

TEST_F(DatastoreTest, CouchbaseInit) {
  hvs::DatastorePtr dbPtr = hvs::DatastoreFactory::create_datastore(
      "test", hvs::DatastoreType::couchbase);
  EXPECT_EQ("couchbase", dbPtr->get_typename());
}

TEST_F(DatastoreTest, Connect) {
  hvs::DatastorePtr dbPtr;
  dbPtr = hvs::DatastoreFactory::create_datastore(
      "test", hvs::DatastoreType::couchbase);
  EXPECT_EQ(0, dbPtr->init());
  dbPtr = hvs::DatastoreFactory::create_datastore(
      "test2", hvs::DatastoreType::couchbase);
  EXPECT_EQ(2, dbPtr->init());
}

TEST_F(DatastoreTest, CouchbaseCURD) {
  std::string key = "test_key";
  std::string value = "test_value";
  hvs::DatastorePtr dbPtr = hvs::DatastoreFactory::create_datastore(
      "test", hvs::DatastoreType::couchbase);
  dbPtr->init();
  EXPECT_EQ(0, dbPtr->set(key, value));
  EXPECT_EQ(value, dbPtr->get(key));
  EXPECT_EQ(0, dbPtr->remove(key));
  EXPECT_EQ("", dbPtr->get(key));
}

TEST_F(DatastoreTest, CouchbaseSubCommand) {
  std::string key = "21st_amendment_brewery_cafe";
  std::string value = "37.7825";
  hvs::DatastorePtr dbPtr = hvs::DatastoreFactory::create_datastore(
      "beer-sample", hvs::DatastoreType::couchbase);
  dbPtr->init();
  EXPECT_EQ(value, dbPtr->get(key, "geo.lat"));
}

