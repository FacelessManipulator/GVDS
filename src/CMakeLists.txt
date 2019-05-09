# ADD_SUBDIRECTORY(hello)
SET(CommonSRCS
        common/Thread.cc
        log/Log.cc
        config/ConfigureSettings.cc
        datastore/datastore.cc
        datastore/couchbase_helper.cc
        common/context.cc
        usermodel/Account.cc
        usermodel/MD5.cc
        usermodel/UserModelServer.cc
        common/RestServer.cc
        common/JsonSerializer.cc
        space/Space.cc
        space/SpaceServer.cc
        zone/Zone.cc
        zone/ZoneServer.cc
        aggregation/AggregationServer.cc
        aggregation/StorageResBicInfo.cc
        msg/rpc.cc
)
SET_PROPERTY(GLOBAL PROPERTY HVSCOMMOM_DEPENDS_ON
        config++
        couchbase
        pthread
        pistache_shared
        rpc
        boost_system
)

ADD_LIBRARY(hvscommon SHARED ${CommonSRCS})
ADD_EXECUTABLE(hvsd main.cc)
install(TARGETS hvscommon
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(TARGETS hvsd
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

add_subdirectory(io_proxy)
add_subdirectory(manager)
#TARGET_LINK_LIBRARIES(hvsd hello)
