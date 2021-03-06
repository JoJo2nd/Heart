cmake_minimum_required(VERSION 2.8)

set(FONT_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( FONT_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${FONT_BUILDER_SRC_DIR}/include"
    "${MINFS_INCLUDE_DIR}"
	${GETOPT_PORT_INCLUDE_DIR}
)
set( FONT_BUILDER_SRC_FILES
    "${FONT_BUILDER_SRC_DIR}/src/fontloader.cpp"
)
set(FONT_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)
source_group(include FILES ${FONT_BUILDER_HDR_FILES})
source_group(source  FILES ${FONT_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${FONT_BUILDER_INCLUDE_DIRS})
link_directories(${FONT_BUILDER_LIB_DIRS})
add_executable(font_builder ${FONT_BUILDER_SRC_FILES} ${FONT_BUILDER_HDR_FILES})
set(FB_LIBS proto lua52 crypto minfs getopt_port ${PLATFORM_LIBS})
target_link_libraries(font_builder debug ${PROTOBUF_LIBRARY_DEBUG} ${FB_LIBS})
target_link_libraries(font_builder optimized ${PROTOBUF_LIBRARY} ${FB_LIBS})

if (PLATFORM_LINUX)
    ADD_CUSTOM_COMMAND(
        TARGET font_builder
        POST_BUILD
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND "ln"
        ARGS -fs "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/font.so" font.so
    )
endif()
