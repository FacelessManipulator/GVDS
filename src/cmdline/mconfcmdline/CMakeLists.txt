GET_PROPERTY(GVDSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "GVDSCOMMOM_DEPENDS_ON")

add_executable(centermodify modifycenter.cc 
${PROJECT_SOURCE_DIR}/src/client/clientuser/ClientUser_struct.cc 
${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc 
${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc 
${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc)
target_link_libraries(centermodify boost_system pthread boost_program_options)

add_executable(centersearch searchcenter.cc 
    ${PROJECT_SOURCE_DIR}/src/client/clientuser/ClientUser_struct.cc
    ${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc
    ${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc
    ${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc
)
target_link_libraries(centersearch boost_system pthread boost_program_options)

add_executable(centerdelete deletecenter.cc 
${PROJECT_SOURCE_DIR}/src/client/clientuser/ClientUser_struct.cc 
${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc 
${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc 
${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc)
target_link_libraries(centerdelete boost_system pthread boost_program_options)

install(TARGETS centermodify centersearch centerdelete
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
