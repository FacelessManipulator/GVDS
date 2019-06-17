GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(spacerename_ipc spacerename_ipc.cc)
target_link_libraries(spacerename_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(spacesizechange_ipc spacesizechange_ipc.cc)
target_link_libraries(spacesizechange_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})