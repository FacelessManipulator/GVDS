# ADD_SUBDIRECTORY(hello)
SET(CommonSRCS
        common/Thread.cc
        log/Log.cc
        config/ConfigureSettings.cc
        datastore/datastore.cc
        datastore/couchbase_helper.cc
        common/context.cc
        common/JsonSerializer.cc
        msg/rpc.cc
)
SET_PROPERTY(GLOBAL PROPERTY HVSCOMMOM_DEPENDS_ON
        config++
        couchbase
        pthread
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
#TARGET_LINK_LIBRARIES(hvsd hello)
