GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_datastore test_datastore.cc)
TARGET_LINK_LIBRARIES(unittest_datastore gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_datastore COMMAND unittest_datastore)

ADD_EXECUTABLE(unittest_couchbase test_couchbase.cc)
TARGET_LINK_LIBRARIES(unittest_couchbase gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_couchbase COMMAND unittest_couchbase)
