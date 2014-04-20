cmake_minimum_required(VERSION 2.8)

ENABLE_PROFILE_IF_SET()

if (MSVC)
    add_definitions(/wd"4244") # proto buffers spits out a lot of uint64 to int conversion warnings
    add_definitions(/wd"4267") # proto buffers spits out a lot of size_t to int conversion warnings
    add_definitions(/wd"4996") # iterator copy rubbish
    add_definitions(-D_SCL_SECURE_NO_WARNINGS) # A secure MS future
endif()

set(PROTO_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set( PROTO_INCLUDE_DIRS
    "${PROTO_SRC_DIR}"
)

file(GLOB PROTO_SRC_FILES "${PROTO_SRC_DIR}/*.proto")

set(PROTO_LITE_SRC_FILES ${PROTO_SRC_FILES})

include_directories(${PROTOBUF_INCLUDE_DIR} ${GENERATED_PROTO_INC_DIRS})

PROTOBUF_GENERATE_LITE_CPP_BINDINGS(LiteProtoSources LiteProtoHeaders ${PROTO_LITE_SRC_FILES})

source_group(proto_files FILES ${PROTO_LITE_SRC_FILES})
source_group(autogen FILES ${LiteProtoSources} ${LiteProtoHeaders})

add_library(proto ${PROTO_LITE_SRC_FILES} ${LiteProtoSources} ${LiteProtoHeaders})
target_link_libraries(proto debug ${PROTOBUF_LITE_LIBRARY_DEBUG} ${PROFILE_LIBS})
target_link_libraries(proto optimized ${PROTOBUF_LITE_LIBRARY} ${PROFILE_LIBS})
set_property(TARGET proto PROPERTY DEBUG_POSTFIX)

