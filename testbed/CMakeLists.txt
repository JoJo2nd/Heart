cmake_minimum_required(VERSION 2.8)

if (PLATFORM_LINUX)
#    #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/deploy/game/${CMAKE_CFG_INTDIR}")
endif()

set(TESTBED_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# heart include directories
set( TESTBED_INCLUDE_DIRS
    ${LIBHEART_INCLUDES}
    "${TESTBED_SOURCE_DIR}/src"
)

ENABLE_PROFILE_IF_SET()

file(GLOB TESTBED_HDR_FILES
	"${TESTBED_SOURCE_DIR}/src/*.h"
)
file(GLOB TESTBED_SRC_FILES
    "${TESTBED_SOURCE_DIR}/src/*.cpp"
)
ADD_MSVC_PRECOMPILED_HEADER(
	"${TESTBED_SOURCE_DIR}/include/testbed_precompiled.h" 
	"${TESTBED_SOURCE_DIR}/src/testbed_precompiled.cpp" 
	TESTBED_PRECOMPILED_SRC_FILES
)

#if (NOT DEFINED ENV{WindowsSDKDir} AND MSVC)
#	#d3dcompiler need this dll deployed with it, it is not standard so copy it from WindowsSDKDir
#	message(FATAL_ERROR "\"WindowsSDKDir\" Environment variable not set..unable to continue build")
#endif()
if (NOT ${BUILD_64_BIT} MATCHES 1)
	set(PLATTYPE x86)
else()
	set(PLATTYPE x64)
endif()

include_directories(${TESTBED_INCLUDE_DIRS})
link_directories(${LIBHEART_LIB_DIRS})
add_executable(heart_testbed WIN32 ${TESTBED_HDR_FILES} ${TESTBED_SRC_FILES} ${TESTBED_PRECOMPILED_SRC_FILES})
target_link_libraries(heart_testbed heart)

if (PLATFORM_WINDOWS)
#    ADD_CUSTOM_COMMAND(
#        TARGET heart_testbed
#        POST_BUILD
#        COMMAND python
#        ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l heart_testbed -s "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/game"
#    )
    ADD_CUSTOM_COMMAND(
    	TARGET heart_testbed
        POST_BUILD
    	COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -s "$ENV{WindowsSDKDir}redist/d3d/${PLATTYPE}/" -d "${CMAKE_SOURCE_DIR}/bin/${CMAKE_CFG_INTDIR}" -l d3dcompiler
    )
endif()
