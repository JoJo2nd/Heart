cmake_minimum_required(VERSION 2.8)

set(TEXTURE_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( TEXTURE_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${TEXTURE_BUILDER_SRC_DIR}/src"
	"${EXTERNAL_SRC_ROOT}/freeimage/Dist"
if (PLATFORM_WINDOWS)
	"${EXTERNAL_SRC_ROOT}/nvidia-texture-tools/src"
endif()
)
file(GLOB TEXTURE_BUILDER_SRC_FILES 
    "${TEXTURE_BUILDER_SRC_DIR}/src/*.h"
    "${TEXTURE_BUILDER_SRC_DIR}/src/*.cpp"
)

source_group(source  FILES ${TEXTURE_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

set(nvtt_BIN_DIR "${EXTERNAL_SRC_ROOT}nvidia-texture-tools/gnuwin32/bin")
set(nvtt_DEPENDENT_BIN_DIR "${EXTERNAL_SRC_ROOT}nvidia-texture-tools/gnuwin32/bin")

if(PLATFORM_WINDOWS)
	if (BUILD_64_BIT)
		set(freeimage_LIB_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x64" )
		set(freeimage_BIN_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x64")
	else()
		set(freeimage_LIB_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x86" )
		set(freeimage_BIN_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x86")
	endif()
	set(FreeImage_LIBRARIES freeimage)
endif()

include_directories(${TEXTURE_BUILDER_INCLUDE_DIRS})
link_directories(
	${TEXTURE_BUILDER_LIB_DIRS}
	${nvtt_LIB_DIR}
	${freeimage_LIB_DIR})
add_library(texture_builder SHARED ${TEXTURE_BUILDER_SRC_FILES})
target_link_libraries(texture_builder debug ${PROTOBUF_LIBRARY_DEBUG} lua52 crypto proto nvtt ${FreeImage_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
target_link_libraries(texture_builder optimized ${PROTOBUF_LIBRARY} lua52 crypto proto nvtt ${FreeImage_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES})

set_target_properties(texture_builder PROPERTIES PREFIX "")
set_target_properties(texture_builder PROPERTIES OUTPUT_NAME texture)

if (PLATFORM_LINUX)
    ADD_CUSTOM_COMMAND(
        TARGET texture_builder
        POST_BUILD
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND "ln"
        ARGS -fs "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/texture.so" texture.so
    )
endif()

if (PLATFORM_WINDOWS)
	ADD_CUSTOM_COMMAND(
		TARGET texture_builder
	    POST_BUILD
		COMMAND python
		ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_files.py"
		ARGS -r "(.*\\.(dll))$"
		ARGS -s "${freeimage_BIN_DIR}"
		ARGS -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
		COMMAND python
		ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_files.py" -r "(.*\\.(dll))$" -s "${nvtt_BIN_DIR}" -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
		COMMAND python
		ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_files.py" -r "(.*\\.(dll))$" -s "${nvtt_DEPENDENT_BIN_DIR}" -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
	)
endif()
