
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  endif()
  set(_GRPC_GRPCPP_UNSECURE grpc++_unsecure)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()

# Proto file
get_filename_component(gvds_proto "op.proto" ABSOLUTE)
get_filename_component(gvds_proto_path "${gvds_proto}" PATH)

# Generated sources
set(gvds_proto_srcs "${CMAKE_BINARY_DIR}/pb/op.pb.cc")
set(gvds_proto_hdrs "${CMAKE_BINARY_DIR}/pb/op.pb.h")
set(gvds_grpc_srcs "${CMAKE_BINARY_DIR}/pb/op.grpc.pb.cc")
set(gvds_grpc_hdrs "${CMAKE_BINARY_DIR}/pb/op.grpc.pb.h")
add_custom_command(
      OUTPUT ${gvds_proto_srcs} ${gvds_proto_hdrs} ${gvds_grpc_srcs} ${gvds_grpc_hdrs}
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS
        --grpc_out "${CMAKE_BINARY_DIR}/pb"
        --cpp_out "${CMAKE_BINARY_DIR}/pb"
        -I "${gvds_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${gvds_proto}"
      DEPENDS "${gvds_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_BINARY_DIR}/pb")

# Targets greeter_[async_](client|server)
foreach(_target
  test_client test_server)
  add_executable(${_target} "${_target}.cc"
    ${gvds_proto_srcs}
    ${gvds_grpc_srcs})
  target_link_libraries(${_target}
    ${_GRPC_GRPCPP_UNSECURE}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()
