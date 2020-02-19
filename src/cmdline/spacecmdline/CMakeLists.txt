GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(sprename spacerename_ipc.cc 
${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc 
${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc 
${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc)
target_link_libraries(sprename boost_system pthread boost_program_options)

add_executable(adsprename spacerename_admin.cc 
${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc 
${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc 
${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc)
target_link_libraries(adsprename boost_system pthread boost_program_options)

add_executable(spsizechange spacesizechange_ipc.cc 
${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc 
${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc 
${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc)
target_link_libraries(spsizechange boost_system pthread boost_program_options)

add_executable(spusage spaceusage_ipc.cc 
${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc 
${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc 
${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc)
target_link_libraries(spusage boost_system pthread boost_program_options)

add_executable(adspusage spaceusage_admin.cc 
${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc 
${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc 
${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc)
target_link_libraries(adspusage boost_system pthread boost_program_options)

add_executable(spreplica spacereplica.cc 
${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc 
${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc 
${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc)
target_link_libraries(spreplica boost_system pthread boost_program_options)

install(TARGETS sprename spsizechange spusage adsprename adspusage spreplica
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
