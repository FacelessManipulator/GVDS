
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
)

ADD_LIBRARY(hvscommon SHARED ${CommonSRCS})
ADD_EXECUTABLE(hvsd main.cc)
#TARGET_LINK_LIBRARIES(hvsd hello)
