GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_EXECUTABLE(unittest_zone test_zone.cc)
TARGET_LINK_LIBRARIES(unittest_zone gtest gtest_main hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)
ADD_TEST(NAME unittest_zone COMMAND unittest_zone)