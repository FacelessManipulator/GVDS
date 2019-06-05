GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_rpc test_rpc.cc)
TARGET_LINK_LIBRARIES(unittest_rpc gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_rpc COMMAND unittest_rpc)

ADD_EXECUTABLE(unittest_pack test_pack.cc)
TARGET_LINK_LIBRARIES(unittest_pack gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_pack COMMAND unittest_pack)

ADD_EXECUTABLE(unittest_udt test_udt.cc)
TARGET_LINK_LIBRARIES(unittest_udt gtest gtest_main io_proxy hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_udt COMMAND unittest_udt)