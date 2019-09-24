GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(centermodify modifycenter.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(centermodify boost_system pthread boost_program_options)

add_executable(centersearch searchcenter.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(centersearch boost_system pthread boost_program_options)

add_executable(centerdelete deletecenter.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(centerdelete boost_system pthread boost_program_options)

install(TARGETS centermodify centersearch centerdelete
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
