GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_space test_space.cc)
TARGET_LINK_LIBRARIES(unittest_space gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_space COMMAND unittest_space)

ADD_EXECUTABLE(unittest_space_func test_space_func.cc)
TARGET_LINK_LIBRARIES(unittest_space_func gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_space_func COMMAND unittest_space)