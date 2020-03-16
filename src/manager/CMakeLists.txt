GET_PROPERTY(GVDSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "GVDSCOMMOM_DEPENDS_ON")

ADD_LIBRARY(managerO OBJECT 
        ioproxy_mgr.cc 
        manager.cc
        usermodel/Account.cc
        usermodel/MD5.cc
        usermodel/UserModelServer.cc
        space/SpaceServer.cc
        zone/ZoneServer.cc
        resaggregation_mgr.cc
        rpc_mod.cc
        mconf/mconf.cc
)

ADD_EXECUTABLE(gvds_manager main.cc)
TARGET_LINK_LIBRARIES(gvds_manager gvdscommon ${GVDSCOMMOM_DEPENDS_ON}
 boost_thread boost_system boost_program_options managerO)

install(TARGETS gvds_manager
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)