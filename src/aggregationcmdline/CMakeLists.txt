GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(resourceregister_ipc resourceregister_ipc.cc)
target_link_libraries(resourceregister_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(resourcequery_ipc resourcequery_ipc.cc)
target_link_libraries(resourcequery_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(resourcedelete_ipc resourcedelete_ipc.cc)
target_link_libraries(resourcedelete_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)