GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_LIBRARY(io_proxy STATIC proxy_op.cc io_proxy.cc io_worker.cc sync_io.cc ../msg/udt_server.cc ../msg/server_session.cc fd_mgr.cc io_mon.cc)

ADD_EXECUTABLE(hvs_ioproxy hvs_ioproxy_main.cc)

TARGET_LINK_LIBRARIES(hvs_ioproxy io_proxy hvscommon ${HVSCOMMOM_DEPENDS_ON} boost_program_options)

install(TARGETS hvs_ioproxy
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


# TARGET_LINK_LIBRARIES(io_proxy  hvscommon ${HVSCOMMOM_DEPENDS_ON} boost_thread boost_system)