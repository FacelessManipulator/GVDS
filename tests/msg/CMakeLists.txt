GET_PROPERTY(GVDSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "GVDSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_rpc test_rpc.cc)
TARGET_LINK_LIBRARIES(unittest_rpc gtest gtest_main gvdscommon ${GVDSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_rpc COMMAND unittest_rpc)

ADD_EXECUTABLE(unittest_pack test_pack.cc)
TARGET_LINK_LIBRARIES(unittest_pack gtest gtest_main gvdscommon ${GVDSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_pack COMMAND unittest_pack)

ADD_EXECUTABLE(unittest_udt test_udt.cc)
TARGET_LINK_LIBRARIES(unittest_udt gtest gtest_main io_proxy gvdscommon ${GVDSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_udt COMMAND unittest_udt)