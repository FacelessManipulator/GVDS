GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(spacerename spacerename.cc)
target_link_libraries(spacerename hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)