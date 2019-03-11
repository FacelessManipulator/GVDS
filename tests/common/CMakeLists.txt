GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_log test_log.cc)
TARGET_LINK_LIBRARIES(unittest_log gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_log COMMAND unittest_log)
ADD_EXECUTABLE(unittest_json test_json.cc)
TARGET_LINK_LIBRARIES(unittest_json gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_json COMMAND unittest_json)
