
# ADD_SUBDIRECTORY(hello)
SET(CommonSRCS
        common/Thread.cc
        log/Log.cc
)

ADD_LIBRARY(hvscommon SHARED ${CommonSRCS})
ADD_EXECUTABLE(hvsd main.cc)
#TARGET_LINK_LIBRARIES(hvsd hello)