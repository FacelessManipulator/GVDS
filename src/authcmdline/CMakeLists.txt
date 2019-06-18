GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

#add_executable(authsearch_ipc authsearch_ipc.cc ../client/clientuser/ClientUser_struct.cc)
#target_link_libraries(authsearch_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})