GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(authsearch authsearch_ipc.cc ../client/clientuser/ClientAuth_struct.cc ../client/clientuser/ClientAuth_struct.cc)
target_link_libraries(authsearch hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(authmodify authmodify_ipc.cc ../client/clientuser/ClientAuth_struct.cc ../client/clientuser/ClientAuth_struct.cc)
target_link_libraries(authmodify hvscommon ${HVSCOMMOM_DEPENDS_ON})

install(TARGETS authsearch authmodify
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
