cmake_minimum_required(VERSION 2.8)

set(UNIFORMBUFFER_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( UNIFORMBUFFER_BUILDER_INCLUDE_DIRS
    ${PROTOBUF_INCLUDE_DIR}
    ${GENERATED_PROTO_FULL_INC_DIRS}
    "${UNIFORMBUFFER_BUILDER_SRC_DIR}/src"
    ${MINFS_INCLUDE_DIR}
    ${LIB_UNIFORM_BUFFER_INCLUDE_DIR}
	${GETOPT_PORT_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)
set( UNIFORMBUFFER_BUILDER_SRC_FILES
    "${UNIFORMBUFFER_BUILDER_SRC_DIR}/src/uniformbufferloader.cpp"
)

source_group(include FILES ${UNIFORMBUFFER_BUILDER_HDR_FILES})
source_group(source  FILES ${UNIFORMBUFFER_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${UNIFORMBUFFER_BUILDER_INCLUDE_DIRS})
link_directories(${UNIFORMBUFFER_BUILDER_LIB_DIRS})
add_executable(uniformbuffer_builder ${UNIFORMBUFFER_BUILDER_SRC_FILES} ${UNIFORMBUFFER_BUILDER_HDR_FILES})
set(EDB_LIBS proto_full proto_lua lua52 crypto minfs getopt_port ub_exporter_lib)
target_link_libraries(uniformbuffer_builder debug ${PROTOBUF_LIBRARY_DEBUG} ${EDB_LIBS})
target_link_libraries(uniformbuffer_builder optimized ${PROTOBUF_LIBRARY} ${EDB_LIBS})
