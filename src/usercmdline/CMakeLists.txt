GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(userlogin userlogin_ipc.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(userlogin boost_system pthread boost_program_options)

add_executable(usersearch usersearch_ipc.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(usersearch boost_system pthread boost_program_options)

add_executable(usersignup usersignup_ipc.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(usersignup boost_system pthread boost_program_options)

add_executable(usermodify usermodify_ipc.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(usermodify boost_system pthread boost_program_options)

add_executable(userexit userexit_ipc.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(userexit boost_system pthread boost_program_options)

add_executable(usercancel usercancel_ipc.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(usercancel boost_system pthread boost_program_options)

add_executable(adminsignup adminsignup.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adminsignup boost_system pthread boost_program_options)

add_executable(listapply listapply.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(listapply boost_system pthread boost_program_options)

add_executable(suggestion suggestion.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(suggestion boost_system pthread boost_program_options)

add_executable(adcam adcreateaccountmap.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adcam boost_system pthread boost_program_options)

add_executable(aduam aduaccountmap.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(aduam boost_system pthread boost_program_options)

add_executable(adsearcham adsearchaccountmap.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adsearcham boost_system pthread boost_program_options)

add_executable(adseepool adseepool.cc ../client/clientuser/ClientUser_struct.cc ../ipc/IPCClient.cc ../common/JsonSerializer.cc ../cmdline/CmdLineProxy.cc)
target_link_libraries(adseepool boost_system pthread boost_program_options)

install(TARGETS userlogin usersearch usersignup usermodify userexit usercancel adminsignup suggestion adcam aduam adsearcham adseepool
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
