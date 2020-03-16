SET(CommonSRCS
        ${PROJECT_SOURCE_DIR}/src/common/Thread.cc
        ${PROJECT_SOURCE_DIR}/src/log/Log.cc
        ${PROJECT_SOURCE_DIR}/src/cmdline/CmdLineProxy.cc
        ${PROJECT_SOURCE_DIR}/src/config/ConfigureSettings.cc
        ${PROJECT_SOURCE_DIR}/src/common/context.cc
        ${PROJECT_SOURCE_DIR}/src/datastore/datastore.cc
        ${PROJECT_SOURCE_DIR}/src/datastore/couchbase_helper.cc
        ${PROJECT_SOURCE_DIR}/src/common/RestServer.cc
        ${PROJECT_SOURCE_DIR}/src/common/JsonSerializer.cc
        ${PROJECT_SOURCE_DIR}/src/msg/rpc.cc
        ${PROJECT_SOURCE_DIR}/src/msg/udt_client.cc
        ${PROJECT_SOURCE_DIR}/src/msg/udt_writer.cc
        ${PROJECT_SOURCE_DIR}/src/msg/client_session.cc
        ${PROJECT_SOURCE_DIR}/src/common/ipc/IPCClient.cc
        ${PROJECT_SOURCE_DIR}/src/common/ipc/IPCServer.cc
        ${PROJECT_SOURCE_DIR}/src/common/ipc/IPCMessage.hpp
)

SET_PROPERTY(GLOBAL PROPERTY GVDSCOMMOM_DEPENDS_ON
        config++
        couchbase
        pthread
        pistache_shared
        rpc
        boost_system
        boost_thread
        boost_filesystem
        boost_program_options
        udt
)

ADD_LIBRARY(gvdscommon SHARED ${CommonSRCS})
install(TARGETS gvdscommon
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

add_subdirectory(common/pb)
add_subdirectory(io_proxy)
add_subdirectory(client)
add_subdirectory(manager)
add_subdirectory(cmdline/spacecmdline)
add_subdirectory(cmdline/zonecmdline)
add_subdirectory(cmdline/usercmdline)
add_subdirectory(cmdline/aggregationcmdline)
add_subdirectory(cmdline/mconfcmdline)

#TARGET_LINK_LIBRARIES(gvdsd hello)
