GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

# add_subdirectory(fuse)

ADD_EXECUTABLE(hvs_client main.cc fuse_mod.cc client.cc graph_mod.cc hvs_fuse_impl.cc msg_mod.cc zone_mod.cc ipc_mod.cc
                queue.cc client_worker.cc
                ./OPTNode/myping.cc 
                ./OPTNode/opt_node.cc
                ./clientuser/ClientUser_struct.cc
                ./clientuser/ClientUser.cc
                ./clientuser/ClientAuth_struct.cc
        readahead.cc readahead.h cache_mod.cc cache_mod.h)
TARGET_LINK_LIBRARIES(hvs_client fuse3 hvscommon ${HVSCOMMOM_DEPENDS_ON})

install(TARGETS hvs_client
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)