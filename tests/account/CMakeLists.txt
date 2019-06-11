GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_login_search test_login_search.cc)
TARGET_LINK_LIBRARIES(unittest_login_search gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_login_search COMMAND unittest_login_search)


ADD_EXECUTABLE(unittest_registration test_registration.cc)
TARGET_LINK_LIBRARIES(unittest_registration gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_registration COMMAND unittest_registration)

ADD_EXECUTABLE(unittest_login_modify test_login_modify.cc)
TARGET_LINK_LIBRARIES(unittest_login_modify gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_login_modify COMMAND unittest_login_modify)

ADD_EXECUTABLE(unittest_login_cancel test_login_cancel.cc)
TARGET_LINK_LIBRARIES(unittest_login_cancel gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_login_cancel COMMAND unittest_login_cancel)

ADD_EXECUTABLE(unittest_login_exit test_login_exit.cc)
TARGET_LINK_LIBRARIES(unittest_login_exit gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_login_exit COMMAND unittest_login_exit)