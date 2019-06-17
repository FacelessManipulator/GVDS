GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(zonerename_ipc zonerename_ipc.cc)
target_link_libraries(zonerename_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(mapadd_ipc mapadd_ipc.cc)
target_link_libraries(mapadd_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(mapdeduct_ipc mapdeduct_ipc.cc)
target_link_libraries(mapdeduct_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zoneshare_ipc zoneshare_ipc.cc)
target_link_libraries(zoneshare_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zonesharecancel_ipc zonesharecancel_ipc.cc)
target_link_libraries(zonesharecancel_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zonecancel_ipc zonecancel_ipc.cc)
target_link_libraries(zonecancel_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zoneadd_ipc zoneadd_ipc.cc)
target_link_libraries(zoneadd_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(zoneregister_ipc zoneregister_ipc.cc)
target_link_libraries(zoneregister_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})