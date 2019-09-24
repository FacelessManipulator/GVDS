GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(rg resourceregister.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(rg boost_system pthread boost_program_options)

add_executable(rq resourcequery.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(rq boost_system pthread boost_program_options)

add_executable(rd resourcedelete.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(rd boost_system pthread boost_program_options)

add_executable(ru resourceupdate.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(ru boost_system pthread boost_program_options)

install(TARGETS rg rq rd ru
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
