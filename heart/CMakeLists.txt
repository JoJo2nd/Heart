cmake_minimum_required(VERSION 2.8)

project(HEART)

#TODO: execute_process -> to build files? 

# set project directories
set(heart_BINARY_DIR "${HEART_SOURCE_DIR}/../bin/heart")

# heart include directorys
set( HEART_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    "${HEART_SRC_ROOT}/include"
    "${EXTERNAL_SRC_ROOT}/zlib/include"
    "${EXTERNAL_SRC_ROOT}/lua/src"
    "${EXTERNAL_SRC_ROOT}/crypto/include"
    "${EXTERNAL_SRC_ROOT}/rapidxml-1.13/include"
    "${EXTERNAL_SRC_ROOT}/openal/include"
    "${EXTERNAL_SRC_ROOT}/minizip/include"
    "${EXTERNAL_SRC_ROOT}/libogg/include"
    "${EXTERNAL_SRC_ROOT}/libvorbis/include"
)
set(HEART_LIB_DIRS)
if (WIN32)
	set(HEART_LIB_DIRS
		${HEART_LIB_DIRS}
		"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
	)
endif()

# heart include directorys
set( HEART_INCLUDE_DIRS
    ${HEART_INCLUDE_DIRS}
    "${PROJECT_SOURCE_DIR}/src"
)
set( HEART_SRC_FILES
	"${HEART_SOURCE_DIR}/include/Heart.h"
	"${HEART_SOURCE_DIR}/include/HeartBase.h"
)
set( HEART_SRC_FILES
    ${HEART_SRC_FILES}
    "${HEART_SOURCE_DIR}/src/heartBaseUnity.cpp"
    "${HEART_SOURCE_DIR}/src/heartCoreUnity.cpp"
)
set(HEART_BASE_FILES
    "${HEART_SOURCE_DIR}/include/base/hAtomic.h"
    "${HEART_SOURCE_DIR}/include/base/hClock.h"    
    "${HEART_SOURCE_DIR}/include/base/hDebugMacros.h"    
    "${HEART_SOURCE_DIR}/include/base/hProfiler.h"
    "${HEART_SOURCE_DIR}/include/base/hSysCalls.h"
    "${HEART_SOURCE_DIR}/include/base/hUTF8.h"
	"${HEART_SOURCE_DIR}/src/base/hAtomic.cpp"
	"${HEART_SOURCE_DIR}/src/base/hClock.cpp"
	"${HEART_SOURCE_DIR}/src/base/hDebugMacros.cpp"
	"${HEART_SOURCE_DIR}/src/base/hProfiler.cpp"
	"${HEART_SOURCE_DIR}/src/base/hSysCalls.cpp"
    "${HEART_SOURCE_DIR}/src/base/hUTF8.cpp"
    "${HEART_SOURCE_DIR}/src/base/hWildcard.cpp"
)
set( HEART_GEN_FILES
    "${HEART_SOURCE_DIR}/include/events/generated/hPuslisher_gen.h"
)

#platform headers
if (WIN32)
    set( HEART_INCLUDE_DIRS
        ${HEART_INCLUDE_DIRS}
		"${HEART_SOURCE_DIR}/include/pal/win32"
        "${HEART_SOURCE_DIR}/src/pal/win32"
    )
	set( HEART_SRC_FILES
		${HEART_SRC_FILES}
		"${HEART_SOURCE_DIR}/include/pal/win32/device/hWin32.h"
        "${HEART_SOURCE_DIR}/include/pal/win32/device/dx11/hWin32DX11.h"
	)
    # Add platform math files.
    set( HEART_DEVICE_MATH_FILES
		${HEART_DEVICE_MATH_FILES}
		"${HEART_SOURCE_DIR}/include/pal/win32/device/math/hMatrix.h"
        "${HEART_SOURCE_DIR}/include/pal/win32/device/math/hQuaternion.h"
        "${HEART_SOURCE_DIR}/include/pal/win32/device/math/hVec2.h"
        "${HEART_SOURCE_DIR}/include/pal/win32/device/math/hVec3.h"
        "${HEART_SOURCE_DIR}/include/pal/win32/device/math/hVec4.h"
        "${HEART_SOURCE_DIR}/include/pal/win32/device/math/hVector.h"
	)
    set( HEART_SRC_FILES 
        ${HEART_SRC_FILES}
        "${HEART_SOURCE_DIR}/src/pal/win32/heartWin32Unity.cpp"
    )
endif (WIN32)

set( HEART_NON_UNITY_FILES
    ${HEART_BASE_FILES} 
    ${HEART_DEVICE_MATH_FILES}
)
#common vars to pass up to parent directory
set( LIBHEART_INCLUDES
    ${HEART_INCLUDE_DIRS}
    PARENT_SCOPE
)
set( LIBHEART_LIB_DIRS
	${HEART_LIB_DIRS}
	PARENT_SCOPE
)

source_group(unity FILES ${HEART_SRC_FILES})
source_group(base FILES ${HEART_BASE_FILES})
source_group(math FILES ${HEART_DEVICE_MATH_FILES})

set_source_files_properties(${HEART_NON_UNITY_FILES} PROPERTIES HEADER_FILE_ONLY 1)
set_source_files_properties("${HEART_SOURCE_DIR}/include/events/generated/hPuslisher_gen.h" PROPERTIES GENERATED 1)

include_directories(${HEART_INCLUDE_DIRS})
link_directories(${HEART_LIB_DIRS})
add_library(heart ${HEART_HDR_FILES} ${HEART_SRC_FILES} ${HEART_NON_UNITY_FILES})
target_link_libraries(heart zlib crypto lua minizip libogg libvorbis ${PLATFORM_LIBS})
