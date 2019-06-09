GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(userlogin userlogin.cc)
target_link_libraries(userlogin hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)