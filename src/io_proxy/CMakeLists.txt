GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_LIBRARY(io_proxy STATIC proxy_op.cc io_proxy.cc io_worker.cc sync_io.cc)

ADD_EXECUTABLE(hvs_ioproxy hvs_ioproxy_main.cc)

TARGET_LINK_LIBRARIES(hvs_ioproxy io_proxy boost_thread hvscommon ${HVSCOMMOM_DEPENDS_ON})

# TARGET_LINK_LIBRARIES(io_proxy  hvscommon ${HVSCOMMOM_DEPENDS_ON} boost_thread boost_system)