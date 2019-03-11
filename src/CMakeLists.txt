
find_file(RAPIDJSON_INCLUDE
        NAMES rapidjson.h
        PATH_SUFFIXES rapidjson
)

if (NOT RAPIDJSON_INCLUDE)
        message(FATAL_ERROR "can't find rapidjson")
endif(NOT RAPIDJSON_INCLUDE)
# ADD_SUBDIRECTORY(hello)
SET(CommonSRCS
        common/Thread.cc
        log/Log.cc
        datastore/datastore.cc
        datastore/couchbase_helper.cc
)
SET_PROPERTY(GLOBAL PROPERTY HVSCOMMOM_DEPENDS_ON
        couchbase
        pthread
        )
ADD_LIBRARY(hvscommon SHARED ${CommonSRCS})
ADD_EXECUTABLE(hvsd main.cc)
#TARGET_LINK_LIBRARIES(hvsd hello)
