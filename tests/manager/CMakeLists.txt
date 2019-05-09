GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_manager test_manager.cc)
TARGET_LINK_LIBRARIES(unittest_manager gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_manager COMMAND unittest_manager)

ADD_EXECUTABLE(unittest_ioproxy_mgr test_ioproxy_mgr.cc)
TARGET_LINK_LIBRARIES(unittest_ioproxy_mgr gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_ioproxy_mgr COMMAND unittest_ioproxy_mgr)