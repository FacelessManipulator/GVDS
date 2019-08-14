GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(sprename spacerename_ipc.cc)
target_link_libraries(sprename hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adsprename spacerename_admin.cc)
target_link_libraries(adsprename hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(spsizechange spacesizechange_ipc.cc)
target_link_libraries(spsizechange hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(spusage spaceusage_ipc.cc)
target_link_libraries(spusage hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adspusage spaceusage_admin.cc)
target_link_libraries(adspusage hvscommon ${HVSCOMMOM_DEPENDS_ON})

install(TARGETS sprename spsizechange spusage adsprename adspusage
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
