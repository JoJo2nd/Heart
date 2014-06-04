cmake_minimum_required(VERSION 2.8)

set(TEXTURE_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( TEXTURE_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${TEXTURE_BUILDER_SRC_DIR}/src"
	"${EXTERNAL_SRC_ROOT}/freeimage/Dist"
	${nvtt_INCLUDE_DIR}
)
file(GLOB TEXTURE_BUILDER_SRC_FILES 
    "${TEXTURE_BUILDER_SRC_DIR}/src/*.h"
    "${TEXTURE_BUILDER_SRC_DIR}/src/*.cpp"
)
set(TEXTURE_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)

source_group(source  FILES ${TEXTURE_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

if (BUILD_64_BIT)
	set(freeimage_LIB_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x64" )
	set(freeimage_BIN_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x64")
else()
	set(freeimage_LIB_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x86" )
	set(freeimage_BIN_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x86")
endif()

include_directories(${TEXTURE_BUILDER_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(
	${TEXTURE_BUILDER_LIB_DIRS}
	${nvtt_LIB_DIR}
	${freeimage_LIB_DIR})
add_library(texture_builder SHARED ${TEXTURE_BUILDER_SRC_FILES})
target_link_libraries(texture_builder debug ${PROTOBUF_LIBRARY_DEBUG} lua52 crypto proto nvtt freeimage ${Boost_LIBRARIES})
target_link_libraries(texture_builder optimized ${PROTOBUF_LIBRARY} lua52 crypto proto nvtt freeimage ${Boost_LIBRARIES})

set_target_properties(texture_builder PROPERTIES OUTPUT_NAME texture)
set_property(TARGET texture_builder PROPERTY DEBUG_POSTFIX)

ADD_CUSTOM_COMMAND(
	TARGET texture_builder
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l texture -s "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
	COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -s "${freeimage_BIN_DIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
	COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -s "${nvtt_BIN_DIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
	COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -s "${nvtt_DEPENDENT_BIN_DIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
)