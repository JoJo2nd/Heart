cmake_minimum_required(VERSION 2.8)

set(ENTITYDEF_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( ENTITYDEF_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    ${PROTO_LUA_SRC_DIR}
    "${ENTITYDEF_BUILDER_SRC_DIR}/include"
    "${MINFS_INCLUDE_DIR}"
	${GETOPT_PORT_INCLUDE_DIR}
)
set( ENTITYDEF_BUILDER_SRC_FILES
    "${ENTITYDEF_BUILDER_SRC_DIR}/src/entitydefloader.cpp"
)

source_group(include FILES ${ENTITYDEF_BUILDER_HDR_FILES})
source_group(source  FILES ${ENTITYDEF_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${ENTITYDEF_BUILDER_INCLUDE_DIRS})
link_directories(${ENTITYDEF_BUILDER_LIB_DIRS})
add_executable(entitydef_builder ${ENTITYDEF_BUILDER_SRC_FILES} ${ENTITYDEF_BUILDER_HDR_FILES})
set(EDB_LIBS proto proto_lua lua52 crypto minfs getopt_port)
target_link_libraries(entitydef_builder debug ${PROTOBUF_LIBRARY_DEBUG} ${EDB_LIBS})
target_link_libraries(entitydef_builder optimized ${PROTOBUF_LIBRARY} ${EDB_LIBS})

if (PLATFORM_LINUX)
    ADD_CUSTOM_COMMAND(
        TARGET entitydef_builder
        POST_BUILD
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND "ln"
        ARGS -fs "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/entitydef.so" entitydef.so
    )
endif()
