cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system)
find_package(Protobuf REQUIRED)

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
    "${GENERATED_PROTO_INC_DIRS}"
    "${SHADER_BUILDER_SRC_DIR}/src"
)
file(GLOB SHADER_BUILDER_FILES 
    "${SHADER_BUILDER_SRC_DIR}/src/*.h"
    "${SHADER_BUILDER_SRC_DIR}/src/*.cpp"
)
set(SHADER_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)

source_group(source  FILES ${SHADER_BUILDER_FILES})

include_directories(${SHADER_BUILDER_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(${SHADER_BUILDER_LIB_DIRS})
add_library(shader_builder SHARED ${SHADER_BUILDER_FILES} )
target_link_libraries(shader_builder lua52 crypto proto ${Boost_LIBRARIES} ${PLATFORM_LIBS})

set_target_properties(shader_builder PROPERTIES OUTPUT_NAME gpuprogram)
set_property(TARGET shader_builder PROPERTY DEBUG_POSTFIX)

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

if (NOT DEFINED ENV{WindowsSDKDir})
	#d3dcompiler need this dll deployed with it, it is not standard so copy it from WindowsSDKDir
	message(FATAL_ERROR "\"WindowsSDKDir\" Environment variable not set..unable to continue build")
endif()
if (NOT ${BUILD_64_BIT} MATCHES 1)
	set(PLATTYPE x86)
else()
	set(PLATTYPE x64)
endif()

ADD_CUSTOM_COMMAND(
	TARGET shader_builder
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" gpuprogram "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
	COMMAND ROBOCOPY "$ENV{WindowsSDKDir}redist/d3d/${PLATTYPE}/" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}" D3DCompiler*.DLL ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)