cmake_minimum_required(VERSION 2.8)

project(HEART)

#TODO: execute_process -> to build files? 

find_package(Protobuf REQUIRED)

# set project directories
set(heart_BINARY_DIR "${HEART_SOURCE_DIR}/../bin/heart")

# heart include directorys
set( HEART_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    "${ENET_INCLUDE_DIR}"
    "${HEART_SRC_ROOT}/include"
    "${EXTERNAL_SRC_ROOT}/zlib/include"
    "${EXTERNAL_SRC_ROOT}/lua/src"
    "${EXTERNAL_SRC_ROOT}/crypto/include"
    "${EXTERNAL_SRC_ROOT}/rapidxml-1.13/include"
    "${EXTERNAL_SRC_ROOT}/openal/include"
    "${EXTERNAL_SRC_ROOT}/minizip/include"
    "${EXTERNAL_SRC_ROOT}/libogg/include"
    "${EXTERNAL_SRC_ROOT}/libvorbis/include"
    "${PROTOBUF_INCLUDE_DIR}"
    "${GENERATED_PROTO_INC_DIRS}"
)
set(HEART_LIB_DIRS)
if (WIN32)
	if (BUILD_64_BIT)
		set(HEART_LIB_DIRS
			${HEART_LIB_DIRS}
			"${EXTERNAL_SRC_ROOT}/openal/libs/Win64"
		)
	else()
		set(HEART_LIB_DIRS
			${HEART_LIB_DIRS}
			"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
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

file(GLOB HEART_BASE_FILES 
    "${HEART_SOURCE_DIR}/include/base/*.h"
    "${HEART_SOURCE_DIR}/src/base/*.cpp"
)
file(GLOB HEART_COMPONENTS_FILES 
    "${HEART_SOURCE_DIR}/include/components/*.h"
    "${HEART_SOURCE_DIR}/src/components/*.cpp"
)
file(GLOB HEART_CORE_FILES 
    "${HEART_SOURCE_DIR}/include/core/*.h"
    "${HEART_SOURCE_DIR}/src/core/*.cpp"
)

file(GLOB HEART_COMPUTE_FILES 
    "${HEART_SOURCE_DIR}/include/compute/*.h"
    "${HEART_SOURCE_DIR}/src/compute/*.cpp"
)
file(GLOB HEART_NETWORK_FILES 
    "${HEART_SOURCE_DIR}/src/network/*.h"
    "${HEART_SOURCE_DIR}/src/network/*.cpp"
)
set(HEART_DEBUG_FILES
    "${HEART_SOURCE_DIR}/include/debug/hDebugMenuManager.h"
    "${HEART_SOURCE_DIR}/include/debug/hDebugInfo.h"
    "${HEART_SOURCE_DIR}/src/debug/hDebugMenuManager.cpp"
	"${HEART_SOURCE_DIR}/src/debug/hDebugInfo.cpp"
)
set(HEART_EVENTS_FILES
    "${HEART_SOURCE_DIR}/include/events/hEventManager.h"
    
    "${HEART_SOURCE_DIR}/src/events/hEventManager.cpp"
)
set(HEART_INPUT_FILES
    "${HEART_SOURCE_DIR}/include/input/hController.h"
    "${HEART_SOURCE_DIR}/include/input/hControllerManager.h"
    
    "${HEART_SOURCE_DIR}/src/input/hController.cpp"
    "${HEART_SOURCE_DIR}/src/input/hControllerManager.cpp"
)
set(HEART_LUA_FILES
    "${HEART_SOURCE_DIR}/include/lua/hLuaHeartLib.h"
    "${HEART_SOURCE_DIR}/include/lua/hLuaStateManager.h"
    
    "${HEART_SOURCE_DIR}/src/lua/hLuaHeartLib.cpp"
    "${HEART_SOURCE_DIR}/src/lua/hLuaStateManager.cpp"
)
set(HEART_MATH_FILES
    "${HEART_SOURCE_DIR}/include/math/hAABB.h"
    "${HEART_SOURCE_DIR}/include/math/hMathUtil.h"
    "${HEART_SOURCE_DIR}/include/math/hPlane.h"
    "${HEART_SOURCE_DIR}/include/math/hPoly2.h"
    "${HEART_SOURCE_DIR}/include/math/hViewFrustum.h"
    
    "${HEART_SOURCE_DIR}/src/math/hCollision.cpp"
    "${HEART_SOURCE_DIR}/src/math/hViewFrustum.cpp"
)
file(GLOB HEART_RENDER_FILES
    "${HEART_SOURCE_DIR}/include/render/*.h"
    "${HEART_SOURCE_DIR}/src/render/*.cpp"
)
set(HEART_SCENE_FILES
    "${HEART_SOURCE_DIR}/include/scene/hSceneBundle.h"
    "${HEART_SOURCE_DIR}/include/scene/hSceneDefinition.h"
)
set(HEART_THREAD_FILES
    "${HEART_SOURCE_DIR}/include/threading/hJobManager.h"
    "${HEART_SOURCE_DIR}/include/threading/hLocklessComPipe.h"
    "${HEART_SOURCE_DIR}/include/threading/hMutexAutoScope.h"
    
    "${HEART_SOURCE_DIR}/src/threading/hJobManager.cpp"
)
set(HEART_UTILS_FILES
    "${HEART_SOURCE_DIR}/include/utils/hBase64.h"
    "${HEART_SOURCE_DIR}/include/utils/hMemoryViewMenu.h"
    "${HEART_SOURCE_DIR}/include/utils/hRapidXML.h"
    "${HEART_SOURCE_DIR}/include/utils/hRealTimeProfilerMenu.h"
    "${HEART_SOURCE_DIR}/include/utils/hTextFormatting.h"
    
    "${HEART_SOURCE_DIR}/src/utils/hBase64.cpp"
    "${HEART_SOURCE_DIR}/src/utils/hMemoryViewMenu.cpp"
    "${HEART_SOURCE_DIR}/src/utils/hRapidXML.cpp"
    "${HEART_SOURCE_DIR}/src/utils/hRealTimeProfilerMenu.cpp"
    "${HEART_SOURCE_DIR}/src/utils/hTextFormatting.cpp"
)
set( HEART_GEN_FILES
    "${HEART_SOURCE_DIR}/include/events/generated/hPuslisher_gen.h"
)
set(HEART_RESOURCE_FILES
    "${HEART_SOURCE_DIR}/include/resourcedata/hTextureResourceData.h"
	"${HEART_SOURCE_DIR}/include/resourcedata/hShaderResourceData.h"
	"${HEART_SOURCE_DIR}/include/resourcedata/hMaterialResourceData.h"
	"${HEART_SOURCE_DIR}/include/resourcedata/hMeshResourceData.h"
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
    file(GLOB HEART_DEVICE_MATH_FILES
        "${HEART_SOURCE_DIR}/include/pal/win32/device/math/*.h"
	)
    set( HEART_SRC_FILES 
        ${HEART_SRC_FILES}
        "${HEART_SOURCE_DIR}/src/pal/win32/heartWin32Unity.cpp"
    )
    file(GLOB HEART_PLATFORM_NON_UNITY_FILES 
        "${HEART_SOURCE_DIR}/include/pal/win32/device/*.h"
        "${HEART_SOURCE_DIR}/src/pal/win32/device/*.h"
        "${HEART_SOURCE_DIR}/src/pal/win32/device/*.cpp"
    )
endif (WIN32)

set( HEART_NON_UNITY_FILES
    ${HEART_BASE_FILES} 
    ${HEART_COMPONENTS_FILES}
    ${HEART_COMPUTE_FILES}
    ${HEART_CORE_FILES}
    ${HEART_DEBUG_FILES}
    ${HEART_EVENTS_FILES}
    ${HEART_INPUT_FILES}
    ${HEART_LUA_FILES}
    ${HEART_MATH_FILES}
    ${HEART_RENDER_FILES}
    ${HEART_SCENE_FILES}
    ${HEART_THREAD_FILES}
    ${HEART_UTILS_FILES}
    ${HEART_DEVICE_MATH_FILES}
	${HEART_PLATFORM_NON_UNITY_FILES}
	${HEART_RESOURCE_FILES}
    ${HEART_NETWORK_FILES}
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

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

source_group(unity FILES ${HEART_SRC_FILES})
source_group(base FILES ${HEART_BASE_FILES})
source_group(math FILES ${HEART_DEVICE_MATH_FILES})
source_group(components FILES ${HEART_COMPONENTS_FILES})
source_group(compute FILES ${HEART_COMPUTE_FILES})
source_group(core FILES ${HEART_CORE_FILES})
source_group(debug FILES ${HEART_DEBUG_FILES})
source_group(events FILES ${HEART_EVENTS_FILES})
source_group(input FILES ${HEART_INPUT_FILES})
source_group(lua FILES ${HEART_LUA_FILES})
source_group(math FILES ${HEART_MATH_FILES})
source_group(render FILES ${HEART_RENDER_FILES})
source_group(scene FILES ${HEART_SCENE_FILES})
source_group(thread FILES ${HEART_THREAD_FILES})
source_group(utils FILES ${HEART_UTILS_FILES})
source_group(resource_data FILES ${HEART_RESOURCE_FILES})
source_group(platform FILES ${HEART_PLATFORM_NON_UNITY_FILES})
source_group(network FILES ${HEART_NETWORK_FILES})

set_source_files_properties(${HEART_NON_UNITY_FILES} PROPERTIES HEADER_FILE_ONLY 1)
set_source_files_properties("${HEART_SOURCE_DIR}/include/events/generated/hPuslisher_gen.h" PROPERTIES GENERATED 1)

include_directories(${HEART_INCLUDE_DIRS})
link_directories(${HEART_LIB_DIRS})
add_library(heart ${HEART_HDR_FILES} ${HEART_SRC_FILES} ${HEART_NON_UNITY_FILES})
target_link_libraries(heart zlib crypto lua minizip libogg libvorbis proto enet ${PLATFORM_LIBS})
