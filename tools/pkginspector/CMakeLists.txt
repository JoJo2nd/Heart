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

file(GLOB BUILDER_SRC_FILES
    "${BUILDER_SRC_DIR}/src/*.h"
    "${BUILDER_SRC_DIR}/src/*.cpp"
)


source_group(include FILES ${BUILDER_HDR_FILES})
source_group(source  FILES ${BUILDER_SRC_FILES})

set(ALL_SRC_FILES
        ${BUILDER_SRC_FILES}
)

if (MSVC)
	add_definitions(/WX) # Warnings as errors
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

include_directories(${BUILDER_INCLUDE_DIRS})
link_directories(
	${LIBHEART_LIB_DIRS}
)

add_executable(pkginspector ${ALL_SRC_FILES})
set_property(TARGET pkginspector PROPERTY DEBUG_POSTFIX)
#target_link_libraries(pkginspector ${PROTOBUF_LIBRARY} crypto lua52 getopt_port proto_full)
target_link_libraries(pkginspector crypto lua52 getopt_port proto_full)
