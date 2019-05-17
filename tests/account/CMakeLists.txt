GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_login_search test_login_search.cc)
TARGET_LINK_LIBRARIES(unittest_login_search gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_login_search COMMAND unittest_login_search)
