GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(hvsfs hvs_fuse.cc)
target_link_libraries(hvsfs fuse3 pthread io_proxy boost_thread hvscommon ${HVSCOMMOM_DEPENDS_ON} )