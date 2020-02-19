GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(rg resourceregister.cc
        ${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc
        ${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc
        ${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc
        )
target_link_libraries(rg boost_system pthread boost_program_options)

add_executable(rq resourcequery.cc
        ${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc
        ${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc
        ${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc
        )
target_link_libraries(rq boost_system pthread boost_program_options)

add_executable(rd resourcedelete.cc
        ${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc
        ${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc
        ${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc
        )
target_link_libraries(rd boost_system pthread boost_program_options)

add_executable(ru resourceupdate.cc
        ${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc
        ${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc
        ${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc
        )
target_link_libraries(ru boost_system pthread boost_program_options)

install(TARGETS rg rq rd ru
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
