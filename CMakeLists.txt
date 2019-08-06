
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(HVSONE CXX C)
set(VERSION 0.1.0)

# include(CTest)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 是否需要构建文档
option(BUILD_DOC "Build document" OFF)
# The tests are not built by default. To build them, set the 
# build_tests option to ON. You can do it by running ccmake
# or specifying the -Dbuild_tests=ON flag when running cmake.
option(BUILD_TESTING "Build test" ON)

option(BUILD_THIRD_PARTY "Build with third party rather than depends on system" ON)

if(BUILD_THIRD_PARTY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE FILEPATH "archive output path")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE FILEPATH "library output path")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE FILEPATH "executable output path")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(THIRD_PATRY_INCLUDE_DIRECTORIES
      "${PROJECT_SOURCE_DIR}/third-party/googletest/googletest/include"
      "${PROJECT_SOURCE_DIR}/third-party/pistache/include"
      "${PROJECT_SOURCE_DIR}/third-party/rapidjson/include"
      "${PROJECT_SOURCE_DIR}/third-party/libcouchbase/include"
      "${PROJECT_BINARY_DIR}/third-party/libcouchbase/generated"
      "${PROJECT_SOURCE_DIR}/third-party/rpclib/include"
      "${PROJECT_SOURCE_DIR}/third-party/boost"
      "${PROJECT_SOURCE_DIR}/third-party/libconfig/lib"
      "${PROJECT_SOURCE_DIR}/third-party/udt4/udt4/src"
  )

  # download and update submodules
  find_package(Git QUIET)
  if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  # Update submodules as needed
      option(GIT_SUBMODULE "Check submodules during build" ON)
      if(GIT_SUBMODULE)
          message(STATUS "Submodule update")
          execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
          if(NOT GIT_SUBMOD_RESULT EQUAL "0")
              message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
          endif()
      endif()
  endif()

  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/third-party/pistache/CMakeLists.txt" AND
                "${PROJECT_SOURCE_DIR}/third-party/googletest/CMakeLists.txt" AND
                "${PROJECT_SOURCE_DIR}/third-party/rapidjson/CMakeLists.txt" AND
		"${PROJECT_SOURCE_DIR}/third-party/libcouchbase/CMakeLists.txt" AND
                "${PROJECT_SOURCE_DIR}/third-party/rpclib/CMakeLists.txt"
  )
      message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
  endif()
  add_subdirectory(third-party)
  include_directories(${THIRD_PATRY_INCLUDE_DIRECTORIES})
endif()

# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/src/include

# we should add the top level directory to support the
# scene that .h and .cc in a subdirectory
  ${PROJECT_SOURCE_DIR}/src
)

# lib directories
LINK_DIRECTORIES(
  ${PROJECT_BINARY_DIR}/lib

  /usr/local/lib
  /use/lib
)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_SUBDIRECTORY(src)

if(BUILD_TESTING)
    enable_testing()
    ADD_SUBDIRECTORY(tests)
endif(BUILD_TESTING)

if(BUILD_DOC)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${PROJECT_BINARY_DIR}/doc/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")
    add_custom_target( doc_doxygen ALL
	    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
 endif (DOXYGEN_FOUND)
endif(BUILD_DOC)

SET(CONFIG_IN ${PROJECT_SOURCE_DIR}/hvs.conf.example)
SET(CONFIG_OUT ${PROJECT_BINARY_DIR}/hvs.conf)
configure_file(${CONFIG_IN} ${CONFIG_OUT} @ONLY)
install(FILES ${PROJECT_BINARY_DIR}/hvs.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/)
install (FILES ${PROJECT_SOURCE_DIR}/README.md DESTINATION ${CMAKE_INSTALL_PREFIX}/var/data)
install (FILES ${PROJECT_SOURCE_DIR}/README.md DESTINATION ${CMAKE_INSTALL_PREFIX}/var/log)