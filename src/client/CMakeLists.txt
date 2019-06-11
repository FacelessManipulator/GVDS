GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

# add_subdirectory(fuse)
ADD_EXECUTABLE(hvs_client main.cc fuse_mod.cc client.cc graph_mod.cc hvs_fuse_impl.cc rpc_mod.cc zone_mod.cc)
TARGET_LINK_LIBRARIES(hvs_client fuse3 hvscommon ${HVSCOMMOM_DEPENDS_ON})