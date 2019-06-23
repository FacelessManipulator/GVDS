GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(zrename zonerename_ipc.cc)
target_link_libraries(zrename hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(spadd mapadd_ipc.cc)
target_link_libraries(spadd hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(spdeduct mapdeduct_ipc.cc)
target_link_libraries(spdeduct hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zshare zoneshare_ipc.cc)
target_link_libraries(zshare hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zsharecancel zonesharecancel_ipc.cc)
target_link_libraries(zsharecancel hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zcancel zonecancel_ipc.cc)
target_link_libraries(zcancel hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zadd zoneadd_ipc.cc)
target_link_libraries(zadd hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zregister zoneregister_ipc.cc)
target_link_libraries(zregister hvscommon ${HVSCOMMOM_DEPENDS_ON})