GET_PROPERTY(GVDSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "GVDSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_manager test_manager.cc)
TARGET_LINK_LIBRARIES(unittest_manager gtest gtest_main gvdscommon ${GVDSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_manager COMMAND unittest_manager)

ADD_EXECUTABLE(unittest_ioproxy_mgr test_ioproxy_mgr.cc)
TARGET_LINK_LIBRARIES(unittest_ioproxy_mgr gtest gtest_main gvdscommon ${GVDSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_ioproxy_mgr COMMAND unittest_ioproxy_mgr)

ADD_EXECUTABLE(unittest_resaggr_mgr test_resaggr_mgr.cc)
TARGET_LINK_LIBRARIES(unittest_resaggr_mgr gtest gtest_main gvdscommon ${GVDSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_resaggr_mgr COMMAND unittest_resaggr_mgr)