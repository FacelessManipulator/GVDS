GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(zrename zonerename_ipc.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(zrename boost_system pthread boost_program_options)

add_executable(adzrename zonerename_admin.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adzrename boost_system pthread boost_program_options)

add_executable(spadd mapadd_ipc.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(spadd boost_system pthread boost_program_options)

add_executable(spdeduct mapdeduct_ipc.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(spdeduct boost_system pthread boost_program_options)

add_executable(adspdeduct mapdeduct_admin.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adspdeduct boost_system pthread boost_program_options)

add_executable(zshare zoneshare_ipc.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(zshare boost_system pthread boost_program_options)

add_executable(adzshare zoneshare_admin.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adzshare boost_system pthread boost_program_options)

add_executable(zsharecancel zonesharecancel_ipc.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(zsharecancel boost_system pthread boost_program_options)

add_executable(adzsharecancel zonesharecancel_admin.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adzsharecancel boost_system pthread boost_program_options)

add_executable(zcancel zonecancel_ipc.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(zcancel boost_system pthread boost_program_options)

add_executable(adzcancel zonecancel_admin.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adzcancel boost_system pthread boost_program_options)

add_executable(adzadd zoneadd_admin.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adzadd boost_system pthread boost_program_options)

add_executable(zregister zoneregister_ipc.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(zregister boost_system pthread boost_program_options)

add_executable(zlist zonelist_ipc.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(zlist boost_system pthread boost_program_options)

add_executable(adzlist zonelist_admin.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adzlist boost_system pthread boost_program_options)

install(TARGETS zrename spadd spdeduct zshare zcancel zregister zlist adspdeduct adzcancel adzadd adzlist adzrename adzshare adzsharecancel
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
