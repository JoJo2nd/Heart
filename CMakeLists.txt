cmake_minimum_required(VERSION 2.8.12.2)

project(HEART)

# Example of setting binary directories
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}")
#

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(BUILD_PLATFORM "windows")
  set(PLATFORM_WINDOWS true)
  set(PROTOBUF_LUA_PLUGIN protoc-gen-lua.exe)
  add_definitions(-DPLATFORM_WINDOWS)
  set(ROBOCOPY_OPTIONS /XO /XX /njh /njs /ndl /nc /ns /np /W:2)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(BUILD_PLATFORM "linux")
  set(PLATFORM_LINUX true)
  set(PROTOBUF_LUA_PLUGIN protoc-gen-lua)
  add_definitions(-DPLATFORM_LINUX)
  add_definitions(-DGLEW_NO_GLU)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -msse -msse2") # add -v for verbose
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -msse -msse2") # add -v for verbose
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --verbose") # add --verbose for verbose

  if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    message("Using Clang compiler")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -std=c++11 -stdlib=libc++ -lc++abi")
    # work around for clang missing std headers...? 
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/c++/4.8.1 -I/usr/include/x86_64-linux-gnu -I/usr/include/x86_64-linux-gnu/c++/4.8")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -std=c++11")
  endif()

  if (SSE3_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
  endif()
  if (SSSE3_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3")
  endif()
  if (SSE4_1_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
  endif()
  if (SSE4_2_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
  endif()
endif()

if (NOT BUILD_PLATFORM)
  message("Unknown platform ${CMAKE_SYSTEM_NAME} got ${BUILD_PLATFORM} - cannot build")
endif()

#macros
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources SourcesVar)

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledBasename}.h\" -Zm256 /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${SourcesVar}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledBinary}\" -Zm256 /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    SET(SourcesVar ${SourcesVar} ${PrecompiledSource} ${PrecompiledHeader})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

MACRO(GET_BASE_FILENAMES destVar sourceVar)
	set(destVar)
	foreach(FILEPATH ${sourceVar})
		get_filename_component(FILENAME ${FILEPATH} NAME)
		set(descVar ${destVar} ${FILENAME})
	endforeach(FILEPATH)
ENDMACRO(GET_BASE_FILENAMES)

function(PROTOBUF_GENERATE_LUA_CPP_BINDINGS SRCS HDRS luaSRCS luaHDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_LUA_CPP_BINDINGS() called without any proto files")
    return()
  endif(NOT ARGN)
  
  get_filename_component(PROTOBUF_PROTOC_EXECUTABLE_DIRECTORY ${PROTOBUF_PROTOC_EXECUTABLE} DIRECTORY)
    
  set(${SRCS})
  set(${HDRS})
  set(${luaSRCS})
  set(${luaHDRS})
  list(APPEND ${luaSRCS} "${CMAKE_CURRENT_BINARY_DIR}/lua-protobuf.h")
  list(APPEND ${luaHDRS} "${CMAKE_CURRENT_BINARY_DIR}/lua-protobuf.cc")
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    
    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")
    list(APPEND ${luaSRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.lua.cc")
    list(APPEND ${luaHDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.lua.h")

	  set_source_files_properties(${ABS_FIL} PROPERTIES HEADER_FILE_ONLY TRUE)
	  get_filename_component(ABS_ROOT ${ABS_FIL} DIRECTORY)

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
			 "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.lua.cc"
			 "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.lua.h"
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --cpp_out  ${CMAKE_CURRENT_BINARY_DIR} --lua_out  ${CMAKE_CURRENT_BINARY_DIR} --proto_path ${ABS_ROOT} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL} --plugin=protoc-gen-lua=${EXTERNAL_SRC_ROOT}/protobuf/plugin/${BUILD_PLATFORM}/${PROTOBUF_LUA_PLUGIN}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ & Lua protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set_source_files_properties(${${luaSRCS}} ${${luaHDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
  set(${luaSRCS} ${${luaSRCS}} PARENT_SCOPE)
  set(${luaHDRS} ${${luaHDRS}} PARENT_SCOPE)
endfunction()

function(PROTOBUF_GENERATE_LITE_CPP_BINDINGS SRCS HDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_LITE_CPP_BINDINGS() called without any proto files")
    return()
  endif(NOT ARGN)
  
  get_filename_component(PROTOBUF_PROTOC_EXECUTABLE_DIRECTORY ${PROTOBUF_PROTOC_EXECUTABLE} DIRECTORY)
    
  set(${SRCS})
  set(${HDRS})
  set(PROTO_IM_DIR "${CMAKE_CURRENT_BINARY_DIR}/pb_lite")

  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    
	set(PROTO_IM_FILE "${PROTO_IM_DIR}/${FIL_WE}.proto")

    list(APPEND ${SRCS} "${PROTO_IM_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${PROTO_IM_DIR}/${FIL_WE}.pb.h")

	set_source_files_properties(${ABS_FIL} PROPERTIES HEADER_FILE_ONLY TRUE)
	get_filename_component(ABS_ROOT ${ABS_FIL} DIRECTORY)

    add_custom_command(
      OUTPUT "${PROTO_IM_DIR}/${FIL_WE}.pb.cc"
             "${PROTO_IM_DIR}/${FIL_WE}.pb.h"
	    COMMAND python ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/copy_create_proto_files.py" "-s" "${ABS_FIL}" "-d" "${PROTO_IM_DIR}"
	    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ARGS --proto_path=${PROTO_IM_DIR} --cpp_out=${PROTO_IM_DIR} ${PROTO_IM_FILE}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ lite protocol buffer compiler on ${FIL_WE}")
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

#end macros

if( ${CMAKE_SIZEOF_VOID_P} MATCHES 4 )
	set( BUILD_64_BIT 0 )
else()
	set( BUILD_64_BIT 1 )
	add_definitions(-DBUILD_64_BIT)
endif()

#config vars
#set(CMAKE_USE_RELATIVE_PATHS OFF)
#set(CMAKE_DEBUG_POSTFIX "")

#build options
option(USE_XNAMATH "Use old XNA math libraries" OFF)
option(USE_OLD_DXSDK "Use old DirectX SDK. Will cause issues with Windows 8 SDK" OFF)
option(USE_ASSIMP_DEBUG "Use debug assimp libraries" OFF)
option(DO_PROFILE "Enable profiling in exe" OFF)
option(FORCE_DISABLE_MEMORY_TRACKING "Disable all memory tracking, including debug builds" OFF)
option(FORCE_ENABLE_MEMORY_TRACKING "Enable all memory tracking in all builds" OFF)
option(UNITY_BUILD "Unity build where possible" OFF)


MACRO(ENABLE_PROFILE_IF_SET)
	if (DO_PROFILE)
		add_definitions(-DUSING_LIB_PROF)
		include_directories("${EXTERNAL_SRC_ROOT}/libprof/src")
		set(PROFILE_LIBS libprof dbghelp)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gh /GH")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Gh /GH")
	endif (DO_PROFILE)
ENDMACRO(ENABLE_PROFILE_IF_SET)

if (DO_PROFILE AND MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /PROFILE")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /PROFILE")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /PROFILE")
endif()

#add_definitions(-DHEART_DLL)

if (FORCE_DISABLE_MEMORY_TRACKING)
	add_definitions(-DHEART_FORCE_DISABLE_TRACK_MEMORY_ALLOCS)
elseif (FORCE_ENABLE_MEMORY_TRACKING)
	add_definitions(-DHEART_FORCE_TRACK_MEMORY_ALLOCS)
endif(FORCE_DISABLE_MEMORY_TRACKING)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/cmake/modules/")

find_package(Protobuf REQUIRED)
find_package(SDL2 REQUIRED)

find_package(OpenGL REQUIRED)
set(GLEW_PATH_HINT "" CACHE PATH "Root path for GLEW install")
find_package(GLEW REQUIRED)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${GLEW_PATH_HINT}/include")
if (BUILD_64_BIT)
    set(GLEW_LIB_PATH "${GLEW_PATH_HINT}/lib/Release/x64")
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${GLEW_LIB_PATH})
else()
    set(GLEW_LIB_PATH "${GLEW_PATH_HINT}/lib/Release/Win32")
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${GLEW_LIB_PATH})
endif(BUILD_64_BIT)
# path to glew shared library
if (BUILD_64_BIT)
    if(WIN32)
        set(GLEW_BIN_PATH "${GLEW_INCLUDE_DIR}/../bin/Release/x64")
    endif()
else()
    if(WIN32)
        set(GLEW_BIN_PATH "${GLEW_INCLUDE_DIR}/../bin/Release/Win32")
    endif()
endif(BUILD_64_BIT)

#cache vars
set(assimp_BIN_DIR CACHE PATH "Directory for assimp binaries")
set(assimp_LIB_DIR CACHE PATH "Directory for assimp libraries")
set(assimp_INCLUDE_DIR CACHE PATH "Directory for assimp includes")

#Common var
set(EXTERNAL_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/external"
)
set(TOOLS_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/tools"
)
set(HEART_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/heart"
)
set(GENERATED_PROTO_INC_DIRS
    "${CMAKE_BINARY_DIR}/tools/proto/pb_lite/"
    #add more as needed
)
set(GENERATED_PROTO_FULL_INC_DIRS
    "${CMAKE_BINARY_DIR}/tools/proto_full"
    #add more as needed
)
set(ENET_INCLUDE_DIR
    "${EXTERNAL_SRC_ROOT}/enet/include"
)
set(LUA_INCLUDE_DIR
    "${EXTERNAL_SRC_ROOT}/lua/src"
)

if (WIN32)
  #OpenAL and ws2 are still called "32" 64bit build...
	set(PLATFORM_LIBS dbghelp dxguid dxgi XInput9_1_0 d3d11 d3dcompiler openAL32 ws2_32 Winmm)
	#set(PLATFORM_LIBS dbghelp dxguid xinput d3d11 d3dcompiler openAL32)
endif()

if (USE_ASSIMP_DEBUG)
  if (MSVC)
  	if (BUILD_64_BIT)
  		set(assimp_BUILD_EXT "assimp_debug-dll_x64")
  	else()
  		set(assimp_BUILD_EXT "assimp_debug-dll_win32")
  	endif()
  endif()
else()
  if (MSVC)
  	if (BUILD_64_BIT)
  		set(assimp_BUILD_EXT "assimp_release-dll_x64")
  	else()
  		set(assimp_BUILD_EXT "assimp_release-dll_win32")
  	endif()
  endif()
endif()

set(assimp_BIN_DIR_FULL "${assimp_BIN_DIR}/${assimp_BUILD_EXT}")
set(assimp_LIB_DIR_FULL "${assimp_LIB_DIR}/${assimp_BUILD_EXT}")

#SDL2
add_definitions(-DHEART_USE_SDL2)
if (PLATFORM_WINDOWS)
  get_filename_component(SDL2_LIB_PATH ${SDL2_LIBRARY} DIRECTORY)
endif()

#project defines
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
if (USE_XNAMATH)
    add_definitions(-DHEART_USE_XNAMATH)
endif()
if (USE_OLD_DXSDK)
    add_definitions(-DHEART_USE_DXSDK)
endif()

add_definitions(-DHEART_LUA_LIBS)
if (PLATFORM_WINDOWS)
  add_definitions(-DLUA_BUILD_AS_DLL)
endif()

message("protoc path: ${PROTOBUF_PROTOC_EXECUTABLE}")
message("protobuf include dir: ${PROTOBUF_INCLUDE_DIR}")
message("protobuf lib: ${PROTOBUF_LIBRARY}")
message("protobuf lite lib: ${PROTOBUF_LITE_LIBRARY}")

add_subdirectory ("external/zlib")
add_subdirectory ("external/crypto")
add_subdirectory ("external/lua")
add_subdirectory ("external/minizip")
add_subdirectory ("external/libvorbis")
add_subdirectory ("external/libogg")
add_subdirectory ("external/enet")
add_subdirectory ("external/nvidia-texture-tools")
add_subdirectory ("external/physfs")
if (MSVC)
  add_subdirectory ("external/libprof")
endif()
# !!JM Disabled. May reimplement in the future
#add_subdirectory ("external/luadb")
add_subdirectory ("heart")

add_subdirectory ("testbed")

add_subdirectory("resourceloaders/shaderloader")
add_subdirectory("resourceloaders/textureloader")
add_subdirectory("resourceloaders/materialloader")
add_subdirectory("resourceloaders/meshloader")
add_subdirectory("resourceloaders/fontloader")

add_subdirectory("tools/lua")
add_subdirectory("tools/lualibs/filesystem")
add_subdirectory("tools/lualibs/luaxml")
add_subdirectory("tools/lualibs/enet_lua")
add_subdirectory("tools/lualibs/lua_process")

add_subdirectory("tools/proto")
add_subdirectory("tools/proto_full")
if (PLATFORM_WINDOWS)
  add_subdirectory("tools/memtrack")
endif()
# !!JM Disable viewer
#add_subdirectory("tools/builder")
# !!JM Replace with builder
add_subdirectory("tools/builder")
# !!JM Disable viewer end
if (PLATFORM_WINDOWS)
  add_subdirectory("tools/profiler")
endif()
