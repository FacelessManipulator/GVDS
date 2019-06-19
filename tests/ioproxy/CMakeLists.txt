GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_ioproxy test_ioproxy.cc)
TARGET_LINK_LIBRARIES(unittest_ioproxy io_proxy boost_thread gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_ioproxy COMMAND unittest_ioproxy)

ADD_EXECUTABLE(unittest_ioproxy_rpc test_ioproxy_rpc.cc)
TARGET_LINK_LIBRARIES(unittest_ioproxy_rpc io_proxy boost_thread gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON})
ADD_TEST(NAME unittest_ioproxy_rpc COMMAND unittest_ioproxy_rpc)