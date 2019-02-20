#include "datastore.h"
#include "gtest/gtest.h"

TEST(DatastoreTest, CouchbaseInit) {
    hvs::DatastorePtr dbPtr = hvs::DatastoreFactory::create_datastore(
        "test", hvs::DatastoreType::couchbase);
    EXPECT_EQ("couchbase", dbPtr->get_typename());
}

TEST(DatastoreTest, Connect) {
    hvs::DatastorePtr dbPtr;
    dbPtr = hvs::DatastoreFactory::create_datastore(
        "test", hvs::DatastoreType::couchbase);
    EXPECT_EQ(0, dbPtr->init());
    dbPtr = hvs::DatastoreFactory::create_datastore(
        "test2", hvs::DatastoreType::couchbase);
    EXPECT_EQ(2, dbPtr->init());
}

TEST(DatastoreTest, CouchbaseCURD) {
    std::string key = "test_key";
    std::string value = "test_value";
    hvs::DatastorePtr dbPtr = hvs::DatastoreFactory::create_datastore(
        "test", hvs::DatastoreType::couchbase);
    dbPtr->init();
    EXPECT_EQ(0, dbPtr->set(key, value));
    EXPECT_EQ(value, *(dbPtr->get(key)));
    EXPECT_EQ(0, dbPtr->remove(key));
    EXPECT_EQ("", *(dbPtr->get(key)));
}
