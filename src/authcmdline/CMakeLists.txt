GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(authsearch authsearch_ipc.cc ../client/clientuser/ClientAuth_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(authsearch boost_system pthread boost_program_options)

add_executable(authmodify authmodify_ipc.cc ../client/clientuser/ClientAuth_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(authmodify boost_system pthread boost_program_options)

add_executable(adauthsearch adauthsearch.cc ../client/clientuser/ClientAuth_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adauthsearch boost_system pthread boost_program_options)

add_executable(adauthmodify adauthmodify.cc ../client/clientuser/ClientAuth_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adauthmodify boost_system pthread boost_program_options)

install(TARGETS authsearch authmodify adauthsearch adauthmodify
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
