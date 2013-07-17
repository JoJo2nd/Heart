cmake_minimum_required(VERSION 2.8)

set(TESTBED_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# heart include directories
set( TESTBED_INCLUDE_DIRS
    ${LIBHEART_INCLUDES}
    "${TESTBED_SOURCE_DIR}/include"
)

set( TESTBED_HDR_FILES
	"${TESTBED_SOURCE_DIR}/include/Base64Test.h"
#    "${TESTBED_SOURCE_DIR}/include/ComplexMesh1.h"
#    "${TESTBED_SOURCE_DIR}/include/ComplexMesh2.h"
#    "${TESTBED_SOURCE_DIR}/include/ComputeBlur.h"
#    "${TESTBED_SOURCE_DIR}/include/ComputeTest.h"
#    "${TESTBED_SOURCE_DIR}/include/CreateTextureTest.h"
#    "${TESTBED_SOURCE_DIR}/include/DebugPrimsTest.h"
#    "${TESTBED_SOURCE_DIR}/include/EventTest.h"
#    "${TESTBED_SOURCE_DIR}/include/InstanceRenderTest.h"
#    "${TESTBED_SOURCE_DIR}/include/JobManagerTest.h"
#    "${TESTBED_SOURCE_DIR}/include/ListTest.h"
#    "${TESTBED_SOURCE_DIR}/include/MapTest.h"
#    "${TESTBED_SOURCE_DIR}/include/ModelRenderTest.h"
#    "${TESTBED_SOURCE_DIR}/include/ResourceLoadTest.h"
#    "${TESTBED_SOURCE_DIR}/include/Sibenik.h"
    "${TESTBED_SOURCE_DIR}/include/TestBedCore.h"
    "${TESTBED_SOURCE_DIR}/include/TestUtils.h"
#    "${TESTBED_SOURCE_DIR}/include/TexturedPlane.h"
    "${TESTBED_SOURCE_DIR}/include/UnitTestFactory.h"
#	"${TESTBED_SOURCE_DIR}/include/LoadTextureTest.h"
)
set( TESTBED_SRC_FILES
#    "${TESTBED_SOURCE_DIR}/src/ComplexMesh1.cpp"
#    "${TESTBED_SOURCE_DIR}/src/ComplexMesh2.cpp"
#    "${TESTBED_SOURCE_DIR}/src/ComputeBlur.cpp"
#    "${TESTBED_SOURCE_DIR}/src/ComputeTest.cpp"
#    "${TESTBED_SOURCE_DIR}/src/CreateTextureTest.cpp"
#    "${TESTBED_SOURCE_DIR}/src/DebugPrimsTest.cpp"
#    "${TESTBED_SOURCE_DIR}/src/EventTest.cpp"
    "${TESTBED_SOURCE_DIR}/src/HeartTestBed.cpp"
#    "${TESTBED_SOURCE_DIR}/src/InstanceRenderTest.cpp"
#    "${TESTBED_SOURCE_DIR}/src/JobManagerTest.cpp"
    "${TESTBED_SOURCE_DIR}/src/main.cpp"
#    "${TESTBED_SOURCE_DIR}/src/ModelRenderTest.cpp"
#    "${TESTBED_SOURCE_DIR}/src/ResourceLoadTest.cpp"
    "${TESTBED_SOURCE_DIR}/src/Sibenik.cpp"
    "${TESTBED_SOURCE_DIR}/src/TestBedCore.cpp"
    "${TESTBED_SOURCE_DIR}/src/TestUtils.cpp"
#    "${TESTBED_SOURCE_DIR}/src/TexturedPlane.cpp"
    "${TESTBED_SOURCE_DIR}/src/UnitTestFactory.cpp"
#	"${TESTBED_SOURCE_DIR}/src/LoadTextureTest.cpp"
)
ADD_MSVC_PRECOMPILED_HEADER(
	"${TESTBED_SOURCE_DIR}/include/testbed_precompiled.h" 
	"${TESTBED_SOURCE_DIR}/src/testbed_precompiled.cpp" 
	TESTBED_PRECOMPILED_SRC_FILES
)

if (NOT DEFINED ENV{WindowsSDKDir})
	#d3dcompiler need this dll deployed with it, it is not standard so copy it from WindowsSDKDir
	message(FATAL_ERROR "\"WindowsSDKDir\" Environment variable not set..unable to continue build")
endif()
if (NOT ${BUILD_64_BIT} MATCHES 1)
	set(PLATTYPE x86)
else()
	set(PLATTYPE x64)
endif()

include_directories(${TESTBED_INCLUDE_DIRS})
link_directories(${LIBHEART_LIB_DIRS})
add_executable(heart_testbed WIN32 ${TESTBED_HDR_FILES} ${TESTBED_SRC_FILES} ${TESTBED_PRECOMPILED_SRC_FILES})
set_property(TARGET heart_testbed PROPERTY DEBUG_POSTFIX _d)
target_link_libraries(heart_testbed heart)

ADD_CUSTOM_COMMAND(
	TARGET heart_testbed
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" heart_testbed "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/game"
	COMMAND ROBOCOPY "$ENV{WindowsSDKDir}redist/d3d/${PLATTYPE}/" "${CMAKE_SOURCE_DIR}/deploy/game" D3DCompiler*.DLL ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)
