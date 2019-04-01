
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(third-party CXX C)
set(VERSION 0.1.0)

option(HVS_CPPCHECK "use cpp check to check the syntax. Need /usr/bin/cppcheck program" OFF)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

# global variable override
# set(CMAKE_CXX_CPPCHECK "" CACHE STRING "the path of cppcheck program.")

# rapidjson variable override
SET(SOME_EXPAT_OPTION OFF CACHE BOOL "Use some expat option")
SET(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "Build rapidjson documentation.")
SET(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "Build rapidjson examples.")
SET(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "Build rapidjson perftests and unittests.")
SET(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE BOOL 
    "Use gtest installation in `thirdparty/gtest` by default if available")
SET(RAPIDJSON_HAS_STDSTRING ON CACHE BOOL "Use std::string in rapidjson.")

# gtest variable override
SET(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
SET(INSTALL_GTEST OFF CACHE BOOL
    "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)")

# pistache variable override
SET(PISTACHE_INSTALL ON CACHE BOOL "add pistache as install target (recommended)")

# libcouchbase variable override
SET(LCB_NO_TESTS ON CACHE BOOL "Disable building of tests")
SET(LCB_NO_TOOLS ON CACHE BOOL "Disable building of additional tools")
SET(LCB_NO_MOCK ON CACHE BOOL "Don't run tests which depend on the mock")
SET(LCB_BUILD_DTRACE OFF CACHE BOOL  "Build DTrace instrumentation, if available on platform")
SET(LCB_EMBED_PLUGIN_LIBEVENT ON CACHE BOOL "Embed the libevent plugin")
SET(LCB_STATIC_LIBEVENT ON CACHE BOOL "Link static libevent (only applicable if EMBED_PLUGIN_LIBEVENT is ON")
SET(LCB_BUILD_STATIC OFF CACHE BOOL "Build libcouchbase static library")

if(HVS_CPPCHECK)
    find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    if (CMAKE_CXX_CPPCHECK)
        message("-- Cppcheck found " ${CMAKE_CXX_CPPCHECK})
        list(
            APPEND CMAKE_CXX_CPPCHECK 
                "--enable=all"
                "--suppress=*:${PROJECT_SOURCE_DIR}/googletest*"
                "--suppress=*:${PROJECT_SOURCE_DIR}/pistache*"
                "--suppress=*:${PROJECT_SOURCE_DIR}/rapidjson*"
        )
    else()
        message("-- Cppcheck not found")
        set(CMAKE_CXX_CPPCHECK "")
        set(CMAKE_CXX_CPPCHECK "" CACHE FILEPATH "the path of cppcheck program.")
    endif()
else()
    set(CMAKE_CXX_CPPCHECK "")
    set(CMAKE_CXX_CPPCHECK "" CACHE FILEPATH "the path of cppcheck program.")
endif()

ADD_SUBDIRECTORY(googletest)
ADD_SUBDIRECTORY(pistache)
ADD_SUBDIRECTORY(rapidjson)
ADD_SUBDIRECTORY(libcouchbase)