cmake_minimum_required(VERSION 2.8)

find_package(Protobuf REQUIRED)

if (MSVC)
    add_definitions(/wd"4244") # proto buffers spits out a lot of uint64 to int conversion warnings
    add_definitions(/wd"4267") # proto buffers spits out a lot of size_t to int conversion warnings
    add_definitions(/wd"4996") # iterator copy rubbish
    add_definitions(-D_SCL_SECURE_NO_WARNINGS) # A secure MS future
endif()

include_directories(${PROTOBUF_INCLUDE_DIR})

set(PROTO_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set( PROTO_INCLUDE_DIRS
    "${PROTO_SRC_DIR}/src"
)

file(GLOB PROTO_SRC_FILES "${PROTO_SRC_DIR}/*.proto")

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${PROTO_SRC_FILES})

source_group(proto_files FILES ${PROTO_SRC_FILES})
source_group(autogen FILES ${ProtoSources} ${ProtoHeaders})

include_directories(${PROTO_INCLUDE_DIRS})
add_library(proto ${PROTO_SRC_FILES} ${ProtoSources} ${ProtoHeaders})
target_link_libraries(proto debug ${PROTOBUF_LIBRARY_DEBUG})
target_link_libraries(proto optimized ${PROTOBUF_LIBRARY})