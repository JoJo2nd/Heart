cmake_minimum_required(VERSION 2.8)

set(FONT_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( FONT_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${FONT_BUILDER_SRC_DIR}/include"
    "${MINFS_INCLUDE_DIR}"
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
add_library(font_builder SHARED ${FONT_BUILDER_SRC_FILES} ${FONT_BUILDER_HDR_FILES})
set(FB_LIBS proto lua52 crypto minfs ${PLATFORM_LIBS})
target_link_libraries(font_builder debug ${PROTOBUF_LIBRARY_DEBUG} ${FB_LIBS})
target_link_libraries(font_builder optimized ${PROTOBUF_LIBRARY} ${FB_LIBS})

set_target_properties(font_builder PROPERTIES PREFIX "")
set_target_properties(font_builder PROPERTIES OUTPUT_NAME font)

# ADD_CUSTOM_COMMAND(
# 	TARGET font_builder
#     POST_BUILD
#     COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l font -s "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/builder/font"
# )
