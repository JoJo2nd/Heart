cmake_minimum_required(VERSION 2.8)

set(MATERIAL_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( MATERIAL_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
	${MINFS_INCLUDE_DIR}
    "${MATERIAL_BUILDER_SRC_DIR}/src"
	${GETOPT_PORT_INCLUDE_DIR}
)
file(GLOB MATERIAL_BUILDER_SRC_FILES 
    "${MATERIAL_BUILDER_SRC_DIR}/src/*.h"
    "${MATERIAL_BUILDER_SRC_DIR}/src/*.cpp"
)
set(MATERIAL_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)

source_group(source  FILES ${MATERIAL_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${MATERIAL_BUILDER_INCLUDE_DIRS})
link_directories(${MATERIAL_BUILDER_LIB_DIRS})
add_executable(material_builder ${MATERIAL_BUILDER_SRC_FILES})
target_link_libraries(material_builder debug ${PROTOBUF_LIBRARY_DEBUG} lua52 crypto proto minfs getopt_port)
target_link_libraries(material_builder optimized ${PROTOBUF_LIBRARY} lua52 crypto proto minfs getopt_port)

set_target_properties(material_builder PROPERTIES PREFIX "")

if (PLATFORM_LINUX)
    ADD_CUSTOM_COMMAND(
        TARGET material_builder
        POST_BUILD
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND "ln"
        ARGS -fs "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/material.so" material.so
    )
endif()