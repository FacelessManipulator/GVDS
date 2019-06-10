GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

ADD_LIBRARY(managerO OBJECT 
        manager.cc 
        ioproxy_mgr.cc
        space/Space.cc
        space/SpaceServer.cc
        zone/Zone.cc
        zone/ZoneServer.cc
        resaggregation_mgr.cc
        rpc_mod.cc
)

ADD_EXECUTABLE(hvs_manager main.cc)
TARGET_LINK_LIBRARIES(hvs_manager hvscommon ${HVSCOMMOM_DEPENDS_ON} boost_thread boost_system managerO)