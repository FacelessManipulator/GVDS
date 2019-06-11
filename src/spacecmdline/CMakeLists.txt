GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(spacerename spacerename.cc)
target_link_libraries(spacerename hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(spacesizechange spacesizechange.cc)
target_link_libraries(spacesizechange hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(spacerename_ipc spacerename_ipc.cc)
target_link_libraries(spacerename_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)