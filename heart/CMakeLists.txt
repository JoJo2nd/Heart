cmake_minimum_required(VERSION 2.8)

project(HEART)

#config vars
set(CMAKE_USE_RELATIVE_PATHS ON)

#TODO: execute_process -> to build files? 

# set project directories
set(heart_BINARY_DIR "${HEART_SOURCE_DIR}/../bin/heart")

# heart include directorys
set( HEART_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    "${HEART_SOURCE_DIR}/include"
    "${HEART_SOURCE_DIR}/../external/zlib/include"
    "${HEART_SOURCE_DIR}/../external/lua/src"
    "${HEART_SOURCE_DIR}/../external/crypto/include"
    "${HEART_SOURCE_DIR}/../external/rapidxml-1.13/include"
    "${HEART_SOURCE_DIR}/../external/openal/include"
    "${HEART_SOURCE_DIR}/../external/minizip/include"
    "${HEART_SOURCE_DIR}/../external/libogg/include"
    "${HEART_SOURCE_DIR}/../external/libvorbis/include"
)

#platform headers
if (WIN32)
    set( HEART_INCLUDE_DIRS
        ${HEART_INCLUDE_DIRS}
        "${HEART_SOURCE_DIR}/include/pal/win32"
    )
    if (USE_OLD_DXSDK)
        set( HEART_INCLUDE_DIRS 
            ${HEART_INCLUDE_DIRS}
            "$ENV{DXSDK_DIR}/include"
        )
    endif()
    if (USE_XNAMATH)
        set( HEART_INCLUDE_DIRS 
            ${HEART_INCLUDE_DIRS}
            "$ENV{DXSDK_DIR}/include"
        )
    endif()
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

source_group(unity FILES ${HEART_SRC_FILES})
source_group(base FILES ${HEART_BASE_FILES})
source_group(math FILES ${HEART_DEVICE_MATH_FILES})

set_source_files_properties(${HEART_NON_UNITY_FILES} PROPERTIES HEADER_FILE_ONLY 1)
set_source_files_properties("${HEART_SOURCE_DIR}/include/events/generated/hPuslisher_gen.h" PROPERTIES GENERATED 1)

include_directories(${HEART_INCLUDE_DIRS})
add_library(heart ${HEART_HDR_FILES} ${HEART_SRC_FILES} ${HEART_NON_UNITY_FILES})
target_link_libraries(heart zlib lua minizip)
