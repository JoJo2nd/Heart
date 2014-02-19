cmake_minimum_required(VERSION 2.8)

#macros
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${SourcesVar})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledBasename}.h\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledBinary}\" /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource} ${PrecompiledHeader})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

MACRO(GET_BASE_FILENAMES destVar sourceVar)
	set(destVar)
	foreach(FILEPATH ${sourceVar})
		get_filename_component(FILENAME ${FILEPATH} NAME)
		set(descVar ${destVar} ${FILENAME})
	endforeach(FILEPATH)
ENDMACRO(GET_BASE_FILENAMES)

MACRO(SET_USE_PRCOMPILED_HEADER files headername)
	if(MSVC)
		foreach( file ${files} )
				message("${file} COMPILE_FLAGS /Yu${headername}")
				set_source_files_properties(
					${file}
					PROPERTIES
					COMPILE_FLAGS "/Yu${headername}"
					)
		endforeach()
	endif(MSVC)
ENDMACRO(SET_USE_PRCOMPILED_HEADER)

function(PROTOBUF_GENERATE_LUA_CPP_BINDINGS SRCS HDRS luaSRCS luaHDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_LUA_CPP_BINDINGS() called without any proto files")
    return()
  endif(NOT ARGN)
  
  get_filename_component(PROTOBUF_PROTOC_EXECUTABLE_DIRECTORY ${PROTOBUF_PROTOC_EXECUTABLE} DIRECTORY)

  message("${PROTOBUF_PROTOC_EXECUTABLE_DIRECTORY}")
    
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
    message("${ABS_FIL}")
    message("${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.lua.cc")

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
			 "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.lua.cc"
			 "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.lua.h"
	  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --cpp_out  ${CMAKE_CURRENT_BINARY_DIR} --lua_out  ${CMAKE_CURRENT_BINARY_DIR} --proto_path ${ABS_ROOT} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL} --plugin=protoc-gen-lua=${PROTOBUF_PROTOC_EXECUTABLE_DIRECTORY}/../plugin/protoc-gen-lua.exe
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

#end macros

if( ${CMAKE_SIZEOF_VOID_P} MATCHES 4 )
	set( BUILD_64_BIT 0 )
else()
	set( BUILD_64_BIT 1 )
	add_definitions(-DBUILD_64_BIT)
endif()

#config vars
set(CMAKE_USE_RELATIVE_PATHS ON)
set(CMAKE_DEBUG_POSTFIX "_d")
set(ROBOCOPY_OPTIONS /XO /XX /njh /njs /ndl /nc /ns /np /W:2)

#build options
option(USE_XNAMATH "Use old XNA math libraries" OFF)
option(USE_OLD_DXSDK "Use old DirectX SDK. Will cause issues with Windows 8 SDK" OFF)
option(BUILD_HEART_TESTBED "Build testbed for heart" OFF)
option(BUILD_BUILDER_PROJECT "Build builder lib and stand alone" ON)
option(USE_ASSIMP_DEBUG "Use debug assimp libraries" OFF)
option(DO_PROFILE "Enable profiling in exe" OFF)
option(FORCE_DISABLE_MEMORY_TRACKING "Disable all memory tracking, including debug builds" OFF)
option(FORCE_ENABLE_MEMORY_TRACKING "Enable all memory tracking in all builds" OFF)

MACRO(ENABLE_PROFILE_IF_SET)
	if (DO_PROFILE)
		add_definitions(-DUSING_LIB_PROF)
		include_directories("${EXTERNAL_SRC_ROOT}/libprof/src")
		set(PROFILE_LIBS libprof dbghelp)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gh /GH")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Gh /GH")
	endif (DO_PROFILE)
ENDMACRO(ENABLE_PROFILE_IF_SET)

if (DO_PROFILE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /PROFILE")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /PROFILE")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /PROFILE")
endif(DO_PROFILE)

if (FORCE_DISABLE_MEMORY_TRACKING)
	add_definitions(-DHEART_FORCE_DISABLE_TRACK_MEMORY_ALLOCS)
elseif (FORCE_ENABLE_MEMORY_TRACKING)
	add_definitions(-DHEART_FORCE_TRACK_MEMORY_ALLOCS)
endif(FORCE_DISABLE_MEMORY_TRACKING)

#cache vars
set(assimp_BIN_DIR "C:/assimp_sdk_folder/bin" CACHE PATH "Directory for assimp binaries")
set(assimp_LIB_DIR "C:/assimp_sdk_folder/lib" CACHE PATH "Directory for assimp libraries")
set(assimp_INCLUDE_DIR "C:/assimp_sdk_folder/include" CACHE PATH "Directory for assimp includes")
set(wxWidgets_CUSTOM_SEARCH_DIR "C:/wxWidgets_sdk_folder/include" CACHE PATH "Directory for wxWidget librarys")
set(nvtt_INCLUDE_DIR "C:/nvtt_sdk_folder/include" CACHE PATH "Directory for nVidia Texture Tools includes")
set(nvtt_LIB_DIR "C:/nvtt_sdk_folder/project/xxx/lib" CACHE PATH "Directory for nVidia Texture Tools libraries")
set(nvtt_BIN_DIR "C:/nvtt_sdk_folder/project/xxx/bin" CACHE PATH "Directory for nVidia Texture Tools binaries")
set(nvtt_DEPENDENT_BIN_DIR "C:/nvtt_sdk_folder/gnumake32/bin" CACHE PATH "Directory for nVidia Texture Tools external binaries")
set(wxLua_BIN_DIR_ROOT "C:/wxLua/bin" CACHE PATH "Root directory for wxLua .dlls (Release and Debug)") 

#Set wxWidgets search path
set(wxWidgets_ROOT_DIR ${wxWidgets_CUSTOM_SEARCH_DIR})
set(wxWidgets_CONFIGURATION CACHE STRING mswdll)
#set(wxWidgets_CONFIGURATION mswdlld)# for debug

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
    "${CMAKE_BINARY_DIR}/tools/proto"
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
	if (BUILD_64_BIT)
		set(assimp_BUILD_EXT "assimp_debug-dll_x64")
	else()
		set(assimp_BUILD_EXT "assimp_debug-dll_win32")
	endif()
else()
	if (BUILD_64_BIT)
		set(assimp_BUILD_EXT "assimp_release-dll_x64")
	else()
		set(assimp_BUILD_EXT "assimp_release-dll_win32")
	endif()
endif()

set(assimp_BIN_DIR_FULL "${assimp_BIN_DIR}/${assimp_BUILD_EXT}")
set(assimp_LIB_DIR_FULL "${assimp_LIB_DIR}/${assimp_BUILD_EXT}")

#Add boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_DEBUG OFF)
#turn off auto link by boost, we manage this ourselves
add_definitions(-DBOOST_FILESYSTEM_NO_LIB)
if (DEFINED ENV{BOOST_ROOT})
	message("Boost Env Var found. Using boost path $ENV{BOOST_ROOT}")
else()
	message("Boost Env Var NOT found. Using boost path ${EXTERNAL_SRC_ROOT}/boost")
	set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${EXTERNAL_SRC_ROOT}/boost")
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${EXTERNAL_SRC_ROOT}/boost/stage/lib")
endif()

# Add proto buffers
set(PROTOBUF_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/include"
)
set(PROTOBUF_LIBRARY
    "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/lib/x64/release/libprotobuf-lite.lib"
)
set(PROTOBUF_LIBRARY_DEBUG
    "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/lib/x64/debug/libprotobuf-lite.lib"
)
set(PROTOBUF_LIBRARY_FULL
    "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/lib/x64/release/libprotobuf.lib"
)
set(PROTOBUF_LIBRARY_FULL_DEBUG
    "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/lib/x64/debug/libprotobuf.lib"
)
set(PROTOBUF_PROTOC_EXECUTABLE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/bin/protoc.exe"
)

project(HEART)

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
add_definitions(-DHEART_LUA_LIBS -DLUA_COMPAT_MODULE -DLUA_BUILD_AS_DLL)

add_subdirectory ("external/zlib")
add_subdirectory ("external/crypto")
add_subdirectory ("external/lua")
add_subdirectory ("external/minizip")
add_subdirectory ("external/libvorbis")
add_subdirectory ("external/libogg")
add_subdirectory ("external/enet")
add_subdirectory ("external/libprof")
add_subdirectory ("external/luadb")
add_subdirectory ("heart")
if (BUILD_HEART_TESTBED)
	add_subdirectory ("testbed")
endif()

add_subdirectory("resourceloaders/shaderloader")
add_subdirectory("resourceloaders/textureloader")
add_subdirectory("resourceloaders/materialloader")
add_subdirectory("resourceloaders/meshloader")
add_subdirectory("resourceloaders/fontloader")

add_subdirectory("tools/lua")
add_subdirectory("tools/lualibs/filesystem")
add_subdirectory("tools/lualibs/luaxml")
add_subdirectory("tools/lualibs/enet_lua")

add_subdirectory("tools/proto")
add_subdirectory("tools/memtrack")
add_subdirectory("tools/viewer")
add_subdirectory("tools/profiler")

message(${CMAKE_BINARY_DIR})
