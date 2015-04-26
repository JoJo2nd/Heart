cmake_minimum_required(VERSION 2.8)

project(HEART)

ENABLE_PROFILE_IF_SET()

# heart include directorys
set( HEART_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    "${ENET_INCLUDE_DIR}"
    "${HEART_SRC_ROOT}/include"
    "${EXTERNAL_SRC_ROOT}/zlib/include"
    "${EXTERNAL_SRC_ROOT}/lua/src"
    "${EXTERNAL_SRC_ROOT}/crypto/src"
    "${EXTERNAL_SRC_ROOT}/rapidxml-1.13/include"
    "${EXTERNAL_SRC_ROOT}/openal/include"
    "${EXTERNAL_SRC_ROOT}/minizip/include"
    "${EXTERNAL_SRC_ROOT}/libogg/include"
    "${EXTERNAL_SRC_ROOT}/libvorbis/include"
    "${EXTERNAL_SRC_ROOT}/sce_vectormath/include"
    "${PROTOBUF_INCLUDE_DIR}"
    "${GENERATED_PROTO_INC_DIRS}"
    "${SDL2_INCLUDE_DIR}"
    "${GLEW_INCLUDE_DIRS}"
    "${FREETYPE_INCLUDE_DIRS}"
    "${IMGUI_INCLUDE_DIR}"
    "${MEMTRACKERLIB_SRC_DIR}"
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

if (USE_SDL2)
	set(HEART_LIB_DIRS
		${HEART_LIB_DIRS}
		${SDL2_LIB_PATH})
endif()

# heart include directorys
set( HEART_INCLUDE_DIRS
    ${HEART_INCLUDE_DIRS}
    "${PROJECT_SOURCE_DIR}/src"
)
set( HEART_SRC_FILES
	"${HEART_SOURCE_DIR}/src/Heart.h"
	"${HEART_SOURCE_DIR}/src/HeartBase.h"
)
file(GLOB HEART_UNITY_SRC
    "${HEART_SOURCE_DIR}/src/heart.unity.cpp"
)
set(HEART_UNITY_TARGET "${HEART_SOURCE_DIR}/src/unity.target")
file(GLOB HEART_BASE_FILES 
    "${HEART_SOURCE_DIR}/src/base/*.h"
    "${HEART_SOURCE_DIR}/src/base/*.cpp"
)
file(GLOB HEART_AUDIO_FILES 
    "${HEART_SOURCE_DIR}/src/audio/*.h"
    "${HEART_SOURCE_DIR}/src/audio/*.cpp"
)
file(GLOB HEART_COMPONENTS_FILES 
    "${HEART_SOURCE_DIR}/src/components/*.h"
    "${HEART_SOURCE_DIR}/src/components/*.cpp"
)
file(GLOB HEART_CORE_FILES 
    "${HEART_SOURCE_DIR}/src/core/*.h"
    "${HEART_SOURCE_DIR}/src/core/*.cpp"
)
file(GLOB HEART_LFDS_FILES 
    "${HEART_SOURCE_DIR}/src/lfds/*.h"
    "${HEART_SOURCE_DIR}/src/lfds/*.c"
    "${HEART_SOURCE_DIR}/src/lfds/*.cpp"
)
file(GLOB HEART_COMPUTE_FILES 
    "${HEART_SOURCE_DIR}/src/compute/*.h"
    "${HEART_SOURCE_DIR}/src/compute/*.cpp"
)
file(GLOB HEART_NETWORK_FILES 
    "${HEART_SOURCE_DIR}/src/network/*.h"
    "${HEART_SOURCE_DIR}/src/network/*.cpp"
)
file(GLOB HEART_DEBUG_FILES
    "${HEART_SOURCE_DIR}/src/debug/*.h"
    "${HEART_SOURCE_DIR}/src/debug/*.cpp"
)
file(GLOB HEART_EVENTS_FILES
    "${HEART_SOURCE_DIR}/src/events/*.h"
    "${HEART_SOURCE_DIR}/src/events/*.cpp"
	"${HEART_SOURCE_DIR}/src/events/*.py"
)
file(GLOB HEART_INPUT_FILES
    "${HEART_SOURCE_DIR}/src/input/*.h"
    "${HEART_SOURCE_DIR}/src/input/*.cpp"
)
file(GLOB HEART_LUA_FILES
    "${HEART_SOURCE_DIR}/src/lua/*.h"
    "${HEART_SOURCE_DIR}/src/lua/*.cpp"
)
file(GLOB HEART_MATH_FILES
    "${HEART_SOURCE_DIR}/src/math/*.h"
    "${HEART_SOURCE_DIR}/src/math/*.cpp"
)
file(GLOB HEART_RENDER_FILES
    "${HEART_SOURCE_DIR}/src/render/*.h"
    "${HEART_SOURCE_DIR}/src/render/*.cpp"
)
file(GLOB HEART_SCENE_FILES
    "${HEART_SOURCE_DIR}/src/scene/*.h"
    "${HEART_SOURCE_DIR}/src/scene/*.cpp"
)
file(GLOB HEART_THREAD_FILES
    "${HEART_SOURCE_DIR}/src/threading/*.h"
    "${HEART_SOURCE_DIR}/src/threading/*.cpp"
)
file(GLOB HEART_UTILS_FILES
    "${HEART_SOURCE_DIR}/src/utils/*.h"
    "${HEART_SOURCE_DIR}/src/utils/*.cpp"
)
file(GLOB HEART_GEN_FILES
    "${HEART_SOURCE_DIR}/src/events/generated/*.h"
)
file(GLOB HEART_RESOURCE_FILES
    "${HEART_SOURCE_DIR}/src/resourcedata/*.h"
	"${HEART_SOURCE_DIR}/src/resourcedata/*.cpp"
)

#platform headers
if (PLATFORM_WINDOWS)
    set( HEART_INCLUDE_DIRS
        ${HEART_INCLUDE_DIRS}
		"${HEART_SOURCE_DIR}/src/win32/pal"
        "${HEART_SOURCE_DIR}/src/openal"
    )
    file(GLOB HEART_PLATFORM_BASE_FILES
        "${HEART_SOURCE_DIR}/src/win32/pal/base/*.h"
        "${HEART_SOURCE_DIR}/src/win32/pal/base/*.cpp")
    file(GLOB HEART_PLATFORM_NON_UNITY_FILES 
        "${HEART_SOURCE_DIR}/src/win32/pal/*.h"
        "${HEART_SOURCE_DIR}/src/win32/pal/*.cpp"
        "${HEART_SOURCE_DIR}/src/win32/pal/device/*.h"
        "${HEART_SOURCE_DIR}/src/win32/pal/device/*.cpp"
        "${HEART_SOURCE_DIR}/src/opengl/*.h"
        "${HEART_SOURCE_DIR}/src/opengl/*.cpp"
        "${HEART_SOURCE_DIR}/src/openal/pal/*.h"
        "${HEART_SOURCE_DIR}/src/openal/pal/*.cpp"
    )
endif()
if (PLATFORM_LINUX)
    set( HEART_INCLUDE_DIRS
        ${HEART_INCLUDE_DIRS}
        "${HEART_SOURCE_DIR}/src/posix/pal"
        "${HEART_SOURCE_DIR}/src/openal"
    )
    file(GLOB HEART_PLATFORM_BASE_FILES
        "${HEART_SOURCE_DIR}/src/posix/pal/base/*.h"
        "${HEART_SOURCE_DIR}/src/posix/pal/base/*.cpp")
    file(GLOB HEART_PLATFORM_NON_UNITY_FILES 
        "${HEART_SOURCE_DIR}/src/posix/pal/*.h"
        "${HEART_SOURCE_DIR}/src/posix/pal/*.cpp"
        "${HEART_SOURCE_DIR}/src/posix/pal/device/*.h"
        "${HEART_SOURCE_DIR}/src/posix/pal/device/*.cpp"
        "${HEART_SOURCE_DIR}/src/opengl/*.h"
        "${HEART_SOURCE_DIR}/src/opengl/*.cpp"
        "${HEART_SOURCE_DIR}/src/openal/pal/*.h"
        "${HEART_SOURCE_DIR}/src/openal/pal/*.cpp"        
    )
endif()

set( HEART_BASE_FILES
    ${HEART_BASE_FILES} ${HEART_PLATFORM_BASE_FILES})

set( HEART_NON_UNITY_FILES
    ${HEART_PLATFORM_NON_UNITY_FILES}
    ${HEART_RESOURCE_FILES}
    ${HEART_AUDIO_FILES} 
    ${HEART_COMPONENTS_FILES}
    ${HEART_COMPUTE_FILES}
    ${HEART_CORE_FILES}
    ${HEART_LFDS_FILES}
    ${HEART_DEBUG_FILES}
    ${HEART_DEVICE_MATH_FILES}
    ${HEART_EVENTS_FILES}
    ${HEART_INPUT_FILES}
    ${HEART_LUA_FILES}
    ${HEART_MATH_FILES}
    ${HEART_NETWORK_FILES}
    ${HEART_RENDER_FILES}
    ${HEART_SCENE_FILES}
    ${HEART_THREAD_FILES}
    ${HEART_UTILS_FILES}
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

source_group(audio FILES ${HEART_AUDIO_FILES})
source_group(base FILES ${HEART_BASE_FILES})
source_group(components FILES ${HEART_COMPONENTS_FILES})
source_group(compute FILES ${HEART_COMPUTE_FILES})
source_group(core FILES ${HEART_CORE_FILES})
source_group(lfds FILES ${HEART_LFDS_FILES})
source_group(debug FILES ${HEART_DEBUG_FILES})
source_group(events FILES ${HEART_EVENTS_FILES})
source_group(input FILES ${HEART_INPUT_FILES})
source_group(lua FILES ${HEART_LUA_FILES})
source_group(math FILES ${HEART_MATH_FILES})
source_group(network FILES ${HEART_NETWORK_FILES})
source_group(platform FILES ${HEART_PLATFORM_NON_UNITY_FILES})
source_group(render FILES ${HEART_RENDER_FILES})
source_group(resource_data FILES ${HEART_RESOURCE_FILES})
source_group(scene FILES ${HEART_SCENE_FILES})
source_group(thread FILES ${HEART_THREAD_FILES})
source_group(unity FILES ${HEART_UNITY_SRC})
source_group(utils FILES ${HEART_UTILS_FILES})

if (UNITY_BUILD)
    message("Heart unity build.")
    set (HEART_SRC_FILES 
        ${HEART_SRC_FILES} ${HEART_UNITY_SRC} ${HEART_UNITY_TARGET})
    set_source_files_properties(${HEART_NON_UNITY_FILES} PROPERTIES HEADER_FILE_ONLY 1)
    set(REGEX1 ".+\\.(h|hpp|gitignore|py|lua|txt|inl)")
    set(UNITY_ARGS "-d \"${HEART_SOURCE_DIR}/src\" -e \".+\\base\\.\" -e \".+\\.(h|hpp|gitignore|py|lua|txt|inl)\" -e .+(.unity[\\d*].cpp) -e \".+([U|u]nity.cpp)\" -o \"${HEART_SOURCE_DIR}/src\" -n heart")
    set(UNITY_ARGS "${UNITY_ARGS} -x unix -x posix")
    message(${HEART_UNITY_SRC} ${UNITY_ARGS})
    add_custom_command(
        OUTPUT "${HEART_SOURCE_DIR}/src/unity.target"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
        COMMAND python 
        ARGS c_cpp_unity_creator.py ${UNITY_ARGS}
        DEPENDS ${HEART_NON_UNITY_FILES}
        COMMENT "Updating Unity Build...")
endif()
set_source_files_properties(${HEART_UNITY_TARGET} PROPERTIES HEADER_FILE_ONLY TRUE)

set(HEART_GEN_FILES ${HEART_GEN_FILES} 
    "${HEART_SOURCE_DIR}/src/base/hFunctor.h"
    "${HEART_SOURCE_DIR}/src/events/hPublisher_gen.inl"
    )
add_custom_command(
      OUTPUT "${HEART_SOURCE_DIR}/src/base/hFunctor.h"
      WORKING_DIRECTORY "${HEART_SOURCE_DIR}/src/base/"
      COMMAND python
      ARGS "${HEART_SOURCE_DIR}/src/base/hFunctor_gen.py"
      DEPENDS "${HEART_SOURCE_DIR}/src/base/hFunctor_gen.py"
      COMMENT "Generating hFunctor.h"
      VERBATIM )
set_source_files_properties("${HEART_SOURCE_DIR}/src/base/hFunctor_gen.py" PROPERTIES HEADER_FILE_ONLY TRUE)
add_custom_command(
      OUTPUT "${HEART_SOURCE_DIR}/src/events/hPublisher_gen.inl"
      WORKING_DIRECTORY "${HEART_SOURCE_DIR}/src/events/"
      COMMAND python
      ARGS "${HEART_SOURCE_DIR}/src/events/hEvent_gen.py" -i 10 -o "hPublisher_gen.inl"
      DEPENDS "${HEART_SOURCE_DIR}/src/events/hEvent_gen.py"
      COMMENT "Generating Events Code"
      VERBATIM )
set_source_files_properties("${HEART_SOURCE_DIR}/src/events/hEvent_gen.py" PROPERTIES HEADER_FILE_ONLY TRUE)    

set_source_files_properties(${HEART_GEN_FILES} PROPERTIES GENERATED 1)

set(HEART_LIBRARIES_DEBUG 
        heart_base zlib crypto lua52 minizip libogg libvorbis proto enet imgui memtrackerlib ${SDL2_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_gl_LIBRARY} ${FREETYPE_LIBRARIES})
set(HEART_LIBRARIES_RELEASE
         heart_base zlib crypto lua52 minizip libogg libvorbis proto enet ${SDL2_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_gl_LIBRARY} ${FREETYPE_LIBRARIES})

include_directories(${HEART_INCLUDE_DIRS})
link_directories(${HEART_LIB_DIRS})

#Create the shared base lib
add_definitions(-DHEART_SHARED_LIB_EXPORTS=1)
add_library(heart_base SHARED ${HEART_BASE_FILES})
target_link_libraries(heart_base debug crypto memtrackerlib ${SDL2_LIBRARY} ${PLATFORM_LIBS})
target_link_libraries(heart_base optimized crypto ${SDL2_LIBRARY} ${PLATFORM_LIBS})

#Create the main engine lib
add_definitions(-DHEART_SHARED_LIB_EXPORTS=0)
add_library(heart ${HEART_HDR_FILES} ${HEART_SRC_FILES} ${HEART_NON_UNITY_FILES} ${HEART_GEN_FILES} "${HEART_SOURCE_DIR}/src/base/hFunctor_gen.py")
target_link_libraries(heart debug ${HEART_LIBRARIES_DEBUG} ${PLATFORM_LIBS} ${PROFILE_LIBS})
target_link_libraries(heart optimized ${HEART_LIBRARIES_DEBUG} ${PLATFORM_LIBS} ${PROFILE_LIBS})

if (PLATFORM_WINDOWS)
    ADD_CUSTOM_COMMAND(
        TARGET heart
        POST_BUILD
        COMMAND python "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l glew32 -s "${GLEW_BIN_PATH}" -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
        COMMAND python "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l SDL2 -s "${SDL2_LIB_PATH}" -d "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
    )
endif()

# Add Code Components
add_subdirectory ("src/codecomponents/filesystem")