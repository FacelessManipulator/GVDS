GET_PROPERTY(GVDSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "GVDSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_space test_space.cc)
TARGET_LINK_LIBRARIES(unittest_space gtest gtest_main gvdscommon ${GVDSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_space COMMAND unittest_space)

ADD_EXECUTABLE(unittest_space_func test_space_func.cc)
TARGET_LINK_LIBRARIES(unittest_space_func gtest gtest_main gvdscommon ${GVDSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_space_func COMMAND unittest_space_func)

ADD_EXECUTABLE(unittest_space_size test_space_size.cc)
TARGET_LINK_LIBRARIES(unittest_space_size gtest gtest_main gvdscommon ${GVDSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_space_size COMMAND unittest_space_size)