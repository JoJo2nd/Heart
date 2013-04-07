cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(SHADER_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( SHADER_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${SHADER_BUILDER_SRC_DIR}/include"
)
set( SHADER_BUILDER_HDR_FILES 
    "${SHADER_BUILDER_SRC_DIR}/include/shaderloader.h"
)
set( SHADER_BUILDER_SRC_FILES
    "${SHADER_BUILDER_SRC_DIR}/src/shaderloader.cpp"
)
set(SHADER_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)

source_group(include FILES ${SHADER_BUILDER_HDR_FILES})
source_group(source  FILES ${SHADER_BUILDER_SRC_FILES})

include_directories(${SHADER_BUILDER_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(${SHADER_BUILDER_LIB_DIRS})
add_library(shader_builder SHARED ${SHADER_BUILDER_SRC_FILES} ${SHADER_BUILDER_HDR_FILES})
target_link_libraries(shader_builder lua crypto ${Boost_LIBRARIES} ${PLATFORM_LIBS})

set_target_properties(shader_builder PROPERTIES OUTPUT_NAME gpuprogram)

if (NOT DEFINED ENV{WindowsSDKDir})
	#d3dcompiler need this dll deployed with it, it is not standard so copy it from WindowsSDKDir
	message(FATAL_ERROR "\"WindowsSDKDir\" Environment variable not set..unable to continue build")
endif()

ADD_CUSTOM_COMMAND(
	TARGET shader_builder
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" gpuprogram "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder"
	COMMAND ROBOCOPY "$ENV{WindowsSDKDir}redist/d3d/x86/" "${CMAKE_SOURCE_DIR}/deploy/builder" D3DCompiler*.DLL ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)