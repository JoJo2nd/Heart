cmake_minimum_required(VERSION 2.8)
find_package(Protobuf REQUIRED)
set( GEN_CPPDIFF_INCLUDE_DIRS
  	"${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROTOBUF_INCLUDE_DIR}"
)

file(GLOB GEN_CPPDIFF_SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

set(ALL_SRC_FILES
    ${GEN_CPPDIFF_SRC_FILES}
)


include_directories(${GEN_CPPDIFF_INCLUDE_DIRS})
add_executable(protoc_gen_cppdiff ${ALL_SRC_FILES})
set_target_properties(protoc_gen_cppdiff PROPERTIES OUTPUT_NAME protoc-gen-cppdiff)

target_link_libraries(protoc_gen_cppdiff debug ${PROTOBUF_LIBRARY_DEBUG} debug ${PROTOBUF_PROTOC_LIBRARY_DEBUG})
target_link_libraries(protoc_gen_cppdiff optimized ${PROTOBUF_LIBRARY} optimized ${PROTOBUF_PROTOC_LIBRARY})