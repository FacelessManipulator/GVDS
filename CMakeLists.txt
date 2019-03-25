
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(HVSONE CXX C)
set(VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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
option(BUILD_TESTING "Build test" ON)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  if (GTEST_FOUND)
    enable_testing()
    ADD_SUBDIRECTORY(tests)
  else (GTEST_FOUND)
    message("please install googletest first")
  endif(GTEST_FOUND)
endif(BUILD_TESTING)

# 是否需要构建文档
option(BUILD_DOC "Build document" ON)
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