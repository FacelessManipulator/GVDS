GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_aggregate test_aggregate.cc)
TARGET_LINK_LIBRARIES(unittest_aggregate gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_aggregate COMMAND unittest_aggregate)
