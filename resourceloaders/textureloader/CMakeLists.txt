cmake_minimum_required(VERSION 2.8)

set(TEXTURE_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( TEXTURE_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${TEXTURE_BUILDER_SRC_DIR}/src"
	"${EXTERNAL_SRC_ROOT}/freeimage/Dist"
	"${GETOPT_PORT_INCLUDE_DIR}"
	"${NVTT_INCLUDE_DIRS}"
)
file(GLOB TEXTURE_BUILDER_SRC_FILES 
    "${TEXTURE_BUILDER_SRC_DIR}/src/*.h"
    "${TEXTURE_BUILDER_SRC_DIR}/src/*.cpp"
)

source_group(source  FILES ${TEXTURE_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()


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

set(TB_LIBS_OPT ${NVTT_LIBRARIES} ${PROTOBUF_LIBRARY})
set(TB_LIBS_DEBUG ${NVTT_LIBRARIES_DEBUG} ${PROTOBUF_LIBRARY_DEBUG})

message("TB_LIBS_OPT: ${TB_LIBS_OPT}")
message("TB_LIBS_DEBUG: ${TB_LIBS_DEBUG}")

include_directories(${TEXTURE_BUILDER_INCLUDE_DIRS})
link_directories(
	${TEXTURE_BUILDER_LIB_DIRS}
	${nvtt_LIB_DIR}
	${freeimage_LIB_DIR})
add_executable(texture_builder ${TEXTURE_BUILDER_SRC_FILES})
#target_link_libraries(texture_builder getopt_port crypto proto ${FreeImage_LIBRARIES} optimized ${TB_LIBS_OPT} debug ${TB_LIBS_DEBUG})
target_link_libraries(texture_builder getopt_port crypto proto ${FreeImage_LIBRARIES})
target_link_libraries(texture_builder optimized ${TB_LIBS_OPT})
#target_link_libraries(texture_builder debug ${TB_LIBS_DEBUG})

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
		ARGS "${DEPLOY_TOOLS_DIR}/deploy_files.py"
		ARGS -r "(.*\\.(dll))$"
		ARGS -s "${freeimage_BIN_DIR}"
		ARGS -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
		#COMMAND python
		#ARGS "${DEPLOY_TOOLS_DIR}/deploy_files.py" -r "(.*\\.(dll))" -s "${GLEW_BIN_DIR}" -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
	)
endif()
