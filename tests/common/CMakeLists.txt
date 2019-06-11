GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_log test_log.cc)
ADD_EXECUTABLE(unittest_config test_config.cc)

TARGET_LINK_LIBRARIES(unittest_log gtest gtest_main  hvscommon ${HVSCOMMOM_DEPENDS_ON})
TARGET_LINK_LIBRARIES(unittest_config gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})

ADD_TEST(NAME unittest_log COMMAND unittest_log)
ADD_TEST(NAME unittest_config COMMAND unittest_config)

ADD_EXECUTABLE(unittest_cmdline test_cmdline.cc)
TARGET_LINK_LIBRARIES(unittest_cmdline gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_cmdline COMMAND unittest_cmdline)

ADD_EXECUTABLE(unittest_json test_json.cc)
TARGET_LINK_LIBRARIES(unittest_json gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_json COMMAND unittest_json)


ADD_EXECUTABLE(unittest_lru test_lru.cc)
TARGET_LINK_LIBRARIES(unittest_lru gtest gtest_main)
ADD_TEST(NAME unittest_lru COMMAND unittest_lru)

ADD_EXECUTABLE(unittest_ipc test_ipc.cc)
TARGET_LINK_LIBRARIES(unittest_ipc gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_ipc COMMAND unittest_ipc)