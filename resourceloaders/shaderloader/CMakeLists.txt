cmake_minimum_required(VERSION 2.8)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}")

set(SHADER_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( SHADER_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${GENERATED_PROTO_INC_DIRS}"
    "${SHADER_BUILDER_SRC_DIR}/src"
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
)

source_group(source  FILES ${SHADER_BUILDER_FILES})

include_directories(${SHADER_BUILDER_INCLUDE_DIRS})
link_directories(${SHADER_BUILDER_LIB_DIRS})
add_library(shader_builder SHARED ${SHADER_BUILDER_FILES} )
target_link_libraries(shader_builder debug ${PROTOBUF_LIBRARY_DEBUG} ${SB_LIBS} ${PLATFORM_LIBS})
target_link_libraries(shader_builder optimized ${PROTOBUF_LIBRARY} ${SB_LIBS} ${PLATFORM_LIBS})

set_target_properties(shader_builder PROPERTIES PREFIX "")
set_target_properties(shader_builder PROPERTIES OUTPUT_NAME gpuprogram)
set_property(TARGET shader_builder PROPERTY DEBUG_POSTFIX)

if (MSVC)
	add_definitions(/WX) # Warnings as errors

    if (NOT DEFINED ENV{WindowsSDKDir})
    	#d3dcompiler need this dll deployed with it, it is not standard so copy it from WindowsSDKDir
    	message(FATAL_ERROR "\"WindowsSDKDir\" Environment variable not set..unable to continue build")
    endif()
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
#    ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l gpuprogram -s "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
#)

if (PLATFORM_WINDOWS)
ADD_CUSTOM_COMMAND(
    TARGET shader_builder
    POST_BUILD 
    COMMAND python
    ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l d3dcompiler -s "$ENV{WindowsSDKDir}redist/d3d/${PLATTYPE}/" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
    COMMAND python
    ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l SDL2 -s "${SDL2_LIB_PATH}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
    COMMAND python
    ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_files.py" -r glew32.dll -s "${GLEW_BIN_PATH}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
)
endif()
