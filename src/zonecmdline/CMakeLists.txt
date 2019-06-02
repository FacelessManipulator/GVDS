GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(zonerename zonerename.cc)
target_link_libraries(zonerename hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(mapadd mapadd.cc)
target_link_libraries(mapadd hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(mapdeduct mapdeduct.cc)
target_link_libraries(mapdeduct hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(zoneshare zoneshare.cc)
target_link_libraries(zoneshare hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(zonesharecancel zonesharecancel.cc)
target_link_libraries(zonesharecancel hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)