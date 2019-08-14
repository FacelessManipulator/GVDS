GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(zrename zonerename_ipc.cc)
target_link_libraries(zrename hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adzrename zonerename_admin.cc)
target_link_libraries(adzrename hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(spadd mapadd_ipc.cc)
target_link_libraries(spadd hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(spdeduct mapdeduct_ipc.cc)
target_link_libraries(spdeduct hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adspdeduct mapdeduct_admin.cc)
target_link_libraries(adspdeduct hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zshare zoneshare_ipc.cc)
target_link_libraries(zshare hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adzshare zoneshare_admin.cc)
target_link_libraries(adzshare hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zsharecancel zonesharecancel_ipc.cc)
target_link_libraries(zsharecancel hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adzsharecancel zonesharecancel_admin.cc)
target_link_libraries(adzsharecancel hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zcancel zonecancel_ipc.cc)
target_link_libraries(zcancel hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adzcancel zonecancel_admin.cc)
target_link_libraries(adzcancel hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adzadd zoneadd_admin.cc)
target_link_libraries(adzadd hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zregister zoneregister_ipc.cc)
target_link_libraries(zregister hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zlist zonelist_ipc.cc)
target_link_libraries(zlist hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adzlist zonelist_admin.cc)
target_link_libraries(adzlist hvscommon ${HVSCOMMOM_DEPENDS_ON})

install(TARGETS zrename spadd spdeduct zshare zcancel zregister zlist adspdeduct adzcancel adzadd adzlist adzrename adzshare adzsharecancel
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
