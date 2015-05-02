cmake_minimum_required(VERSION 2.8)

set(SHADER_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( SHADER_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${GENERATED_PROTO_INC_DIRS}"
    "${SHADER_BUILDER_SRC_DIR}/src"
    ${MINFS_INCLUDE_DIR}
)
file(GLOB SHADER_BUILDER_FILES 
    "${SHADER_BUILDER_SRC_DIR}/src/*.h"
    "${SHADER_BUILDER_SRC_DIR}/src/*.cpp"
)
set(SHADER_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
    ${GLEW_LIB_PATH}
    ${SDL2_LIB_PATH}
)
set(SB_LIBS 
    "lua52"
    "crypto"
    "proto"
    ${GLEW_LIBRARIES}
    ${SDL2_LIBRARY}
    "minfs"
)

source_group(source  FILES ${SHADER_BUILDER_FILES})

include_directories(${SHADER_BUILDER_INCLUDE_DIRS})
link_directories(${SHADER_BUILDER_LIB_DIRS})
add_library(shader_builder SHARED ${SHADER_BUILDER_FILES} )
target_link_libraries(shader_builder debug ${PROTOBUF_LIBRARY_DEBUG} ${SB_LIBS} ${PLATFORM_LIBS})
target_link_libraries(shader_builder optimized ${PROTOBUF_LIBRARY} ${SB_LIBS} ${PLATFORM_LIBS})

set_target_properties(shader_builder PROPERTIES PREFIX "")
set_target_properties(shader_builder PROPERTIES OUTPUT_NAME gpuprogram)

if (MSVC)
	add_definitions(/WX) # Warnings as errors

#    if (NOT DEFINED ENV{WindowsSDKDir})
#    	#d3dcompiler need this dll deployed with it, it is not standard so copy it from WindowsSDKDir
#    	message(FATAL_ERROR "\"WindowsSDKDir\" Environment variable not set..unable to continue build")
#    endif()
endif()

if (NOT ${BUILD_64_BIT} MATCHES 1)
	set(PLATTYPE x86)
else()
	set(PLATTYPE x64)
endif()

#ADD_CUSTOM_COMMAND(
#	TARGET shader_builder
#    POST_BUILD
#    COMMAND python
#    ARGS "${DEPLOY_TOOLS_DIR}/deploy_lib.py" -l gpuprogram -s "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
#)

if (PLATFORM_WINDOWS)
ADD_CUSTOM_COMMAND(
    TARGET shader_builder
    POST_BUILD 
    COMMAND python
    ARGS "${DEPLOY_TOOLS_DIR}/deploy_lib.py" -l d3dcompiler -s "${WindowsSDKDir}/redist/d3d/${PLATTYPE}/" -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
    COMMAND python
    ARGS "${DEPLOY_TOOLS_DIR}/deploy_lib.py" -l SDL2 -s "${SDL2_LIB_PATH}" -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
    COMMAND python
    ARGS "${DEPLOY_TOOLS_DIR}/deploy_files.py" -r glew32.dll -s "${GLEW_BIN_PATH}" -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
)
endif()
if (PLATFORM_LINUX)
    ADD_CUSTOM_COMMAND(
        TARGET shader_builder
        POST_BUILD
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND "ln"
        ARGS -fs "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/gpuprogram.so" gpuprogram.so
    )
endif()