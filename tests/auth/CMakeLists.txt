GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_auth_addmodel auth_addmodel.cc)
TARGET_LINK_LIBRARIES(unittest_auth_addmodel gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_auth_addmodel COMMAND unittest_auth_addmodel)

ADD_EXECUTABLE(unittest_auth_search auth_search.cc)
TARGET_LINK_LIBRARIES(unittest_auth_search gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_auth_search COMMAND unittest_auth_search)

ADD_EXECUTABLE(unittest_auth_modify auth_modify.cc)
TARGET_LINK_LIBRARIES(unittest_auth_modify gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_auth_modify COMMAND unittest_auth_modify)