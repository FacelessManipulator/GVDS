# ADD_SUBDIRECTORY(hello)
SET(CommonSRCS
        common/Thread.cc
        log/Log.cc
        datastore/datastore.cc
        datastore/couchbase_helper.cc
        common/context.cc
        usermodel/Account.cc
        usermodel/MD5.cc
        usermodel/UserModelServer.cc
        common/RestServer.cc
        common/JsonSerializer.cc
)
SET_PROPERTY(GLOBAL PROPERTY HVSCOMMOM_DEPENDS_ON
        couchbase
        pthread
        pistache_shared
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
#TARGET_LINK_LIBRARIES(hvsd hello)
