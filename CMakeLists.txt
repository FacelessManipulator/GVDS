
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(playground CXX C)
set(VERSION 0.1.0)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include(CTest)
# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/src/include
# we should add the top level directory to support the 
# scene that .h and .cc in a subdirectory
  ${PROJECT_SOURCE_DIR}/src
  /usr/local/include
  /usr/include
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

# The tests are not built by default. To build them, set the 
# build_tests option to ON. You can do it by running ccmake
# or specifying the -Dbuild_tests=ON flag when running cmake.
if(BUILD_TESTING)
	ADD_SUBDIRECTORY(googletest)
	ADD_SUBDIRECTORY(tests)
endif(BUILD_TESTING)

