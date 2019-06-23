GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(sprename spacerename_ipc.cc)
target_link_libraries(sprename hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(spsizechange spacesizechange_ipc.cc)
target_link_libraries(spsizechange hvscommon ${HVSCOMMOM_DEPENDS_ON})