GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(resourceregister resourceregister.cc)
target_link_libraries(resourceregister hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(resourcequery resourcequery.cc)
target_link_libraries(resourcequery hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(resourcedelete resourcedelete.cc)
target_link_libraries(resourcedelete hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(resourceupdate resourceupdate.cc)
target_link_libraries(resourceupdate hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)