cmake_minimum_required(VERSION 2.8)

set(BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( BUILDER_INCLUDE_DIRS
  "${BUILDER_SRC_DIR}/src"
	"${TOOLS_SRC_ROOT}/proto/src"
	"${PROTOBUF_INCLUDE_DIR}"
	"${GENERATED_PROTO_FULL_INC_DIRS}"
	"${GETOPT_PORT_INCLUDE_DIR}"
)

set(TDB_SRC_FILES
    "${BUILDER_SRC_DIR}/src/type_database.cpp"
)

file(GLOB BUILDER_SRC_FILES
    "${BUILDER_SRC_DIR}/src/*.h"
    "${BUILDER_SRC_DIR}/src/*.cpp"
)

file(GLOB PROTO_FILES
    "${PROTO_SRC_DIR}/*.proto"
)

set(ALL_SRC_FILES
    ${BUILDER_SRC_FILES}
    "${TDB_SRC_FILES}"
)

message("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/pbscan -o ${PROTO_FILES}")
add_custom_command(
    OUTPUT ${TDB_SRC_FILES}
    WORKING_DIRECTORY "${BUILDER_SRC_DIR}/src"
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/pbscan"
    ARGS "-o" ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    VERBATIM
)
set_source_files_properties(${TDB_SRC_FILES} PROPERTIES GENERATED 1)

source_group(include FILES ${BUILDER_HDR_FILES})
source_group(source  FILES ${BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

include_directories(${BUILDER_INCLUDE_DIRS})
link_directories(
	${LIBHEART_LIB_DIRS}
)

add_executable(pkginspector ${ALL_SRC_FILES})
target_link_libraries(pkginspector crypto lua52 getopt_port proto_full)
add_dependencies(pkginspector pbscan)
