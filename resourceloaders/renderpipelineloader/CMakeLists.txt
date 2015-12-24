cmake_minimum_required(VERSION 2.8)

set(RENDER_PIPELINE_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( RENDER_PIPELINE_BUILDER_INCLUDE_DIRS
    ${PROTOBUF_INCLUDE_DIR}
    ${GENERATED_PROTO_FULL_INC_DIRS}
    "${RENDER_PIPELINE_BUILDER_SRC_DIR}/include"
    ${MINFS_INCLUDE_DIR}
	${GETOPT_PORT_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)
set( RENDER_PIPELINE_BUILDER_SRC_FILES
    "${RENDER_PIPELINE_BUILDER_SRC_DIR}/src/renderpipelineloader.cpp"
)

source_group(include FILES ${RENDER_PIPELINE_BUILDER_HDR_FILES})
source_group(source  FILES ${RENDER_PIPELINE_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${RENDER_PIPELINE_BUILDER_INCLUDE_DIRS})
link_directories(${RENDER_PIPELINE_BUILDER_LIB_DIRS})
add_executable(renderpipeline_builder ${RENDER_PIPELINE_BUILDER_SRC_FILES} ${RENDER_PIPELINE_BUILDER_HDR_FILES})
set(EDB_LIBS proto_full proto_lua lua52 crypto minfs getopt_port)
target_link_libraries(renderpipeline_builder debug ${PROTOBUF_LIBRARY_DEBUG} ${EDB_LIBS})
target_link_libraries(renderpipeline_builder optimized ${PROTOBUF_LIBRARY} ${EDB_LIBS})
