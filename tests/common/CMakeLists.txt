ADD_EXECUTABLE(unittest_log test_log.cc)
TARGET_LINK_LIBRARIES(unittest_log gtest gtest_main pthread hvscommon)
ADD_TEST(NAME unittest_log COMMAND unittest_log)

ADD_EXECUTABLE(unittest_json test_json.cc)
TARGET_LINK_LIBRARIES(unittest_json gtest gtest_main pthread)
ADD_TEST(NAME unittest_json COMMAND unittest_json)
