GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(authsearch_ipc authsearch_ipc.cc ../client/clientuser/ClientAuth_struct.cc ../client/clientuser/ClientAuth_struct.cc)
target_link_libraries(authsearch_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(authmodify_ipc authmodify_ipc.cc ../client/clientuser/ClientAuth_struct.cc ../client/clientuser/ClientAuth_struct.cc)
target_link_libraries(authmodify_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON})