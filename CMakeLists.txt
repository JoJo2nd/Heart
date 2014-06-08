cmake_minimum_required(VERSION 2.8)
project(HEART)

if (MSVC)
  set(BUILD_PLATFORM "windows")
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

#MACRO(SET_USE_PRCOMPILED_HEADER files headername)
	#if(MSVC)
		#foreach( file ${files} )
				#message("${file} COMPILE_FLAGS /Yu${headername}")
				#set_source_files_properties(
					#${file}
					#PROPERTIES
					#COMPILE_FLAGS "/Yu${headername}"
					#)
		#endforeach()
	#endif(MSVC)
#ENDMACRO(SET_USE_PRCOMPILED_HEADER)

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
      ARGS --cpp_out  ${CMAKE_CURRENT_BINARY_DIR} --lua_out  ${CMAKE_CURRENT_BINARY_DIR} --proto_path ${ABS_ROOT} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL} --plugin=protoc-gen-lua=${EXTERNAL_SRC_ROOT}/protobuf/plugin/${BUILD_PLATFORM}/protoc-gen-lua.exe
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
	    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/copy_create_proto_files.py" ARGS "-s" ${ABS_FIL} "-d" ${PROTO_IM_DIR}
	    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ARGS --proto_path=${PROTO_IM_DIR} --cpp_out=${PROTO_IM_DIR} ${PROTO_IM_FILE}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ lite protocol buffer compiler on ${FIL} [ ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${PROTO_IM_DIR} --cpp_out=${PROTO_IM_DIR} ${PROTO_IM_FILE} ]")
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
set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_DEBUG_POSTFIX "_d")
set(ROBOCOPY_OPTIONS /XO /XX /njh /njs /ndl /nc /ns /np /W:2)

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

if (DO_PROFILE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /PROFILE")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /PROFILE")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /PROFILE")
endif(DO_PROFILE)

#add_definitions(-DHEART_DLL)

if (FORCE_DISABLE_MEMORY_TRACKING)
	add_definitions(-DHEART_FORCE_DISABLE_TRACK_MEMORY_ALLOCS)
elseif (FORCE_ENABLE_MEMORY_TRACKING)
	add_definitions(-DHEART_FORCE_TRACK_MEMORY_ALLOCS)
endif(FORCE_DISABLE_MEMORY_TRACKING)

# Helper setup vars
set(Boost_NO_SYSTEM_PATHS ON)
set(BOOST_ROOT_HINT "C:/set/me/to/something/valid" CACHE PATH "Root path for boost 1.53.0")
set(BOOST_LIBRARYDIR_HINT "C:/set/me/to/something/valid" CACHE PATH "Root library path for boost 1.53.0")
set(BOOST_ROOT ${BOOST_ROOT_HINT})
set(BOOST_LIBRARYDIR ${BOOST_LIBRARYDIR_HINT})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/cmake/modules/")

#Boost setup
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_DEBUG OFF)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system regex serialization)
find_package(Protobuf REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS base core adv aui html media xrc gl qa richtext propgrid)
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
#set(wxWidgets_CUSTOM_SEARCH_DIR "C:/wxWidgets_sdk_folder/include" CACHE PATH "Directory for wxWidget librarys")
set(nvtt_INCLUDE_DIR CACHE PATH "Directory for nVidia Texture Tools includes")
set(nvtt_LIB_DIR CACHE PATH "Directory for nVidia Texture Tools libraries")
set(nvtt_BIN_DIR CACHE PATH "Directory for nVidia Texture Tools binaries")
set(nvtt_DEPENDENT_BIN_DIR CACHE PATH "Directory for nVidia Texture Tools external binaries")
set(wxLua_BIN_DIR_ROOT CACHE PATH "Root directory for wxLua .dlls (Release and Debug)") 

#Set wxWidgets search path
#set(wxWidgets_ROOT_DIR ${wxWidgets_CUSTOM_SEARCH_DIR})
#set(wxWidgets_CONFIGURATION CACHE STRING mswdll)
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

#SDL2
add_definitions(-DHEART_USE_SDL2)
get_filename_component(SDL2_LIB_PATH ${SDL2_LIBRARY} DIRECTORY)
#	if (BUILD_64_BIT)
#		set(SDL2_LIB_PATH "${SDL2_LIBPATH}/x64/Release")
#	else()
#		set(SDL2_LIB_PATH "${SDL2_LIBPATH}/Win32/Release")
#	endif()
#	set(SDL2_LIBRARIES SDL2)

#turn off auto link by boost, we manage this ourselves
#add_definitions(-DBOOST_FILESYSTEM_NO_LIB)
#if (DEFINED ENV{BOOST_ROOT})
#	message("Boost Env Var found. Using boost path $ENV{BOOST_ROOT}")
#else()
#	message("Boost Env Var NOT found. Using boost path ${EXTERNAL_SRC_ROOT}/boost")
#	set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${EXTERNAL_SRC_ROOT}/boost")
#	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${EXTERNAL_SRC_ROOT}/boost/stage/lib")
#endif()

# Add proto buffers
# set(PROTOBUF_INCLUDE_DIR
    # "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/include"
# )
# set(PROTOBUF_LIBRARY
    # "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/lib/x64/release/libprotobuf-lite.lib"
# )
# set(PROTOBUF_LIBRARY_DEBUG
    # "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/lib/x64/debug/libprotobuf-lite.lib"
# )
# set(PROTOBUF_LIBRARY_FULL
    # "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/lib/x64/release/libprotobuf.lib"
# )
# set(PROTOBUF_LIBRARY_FULL_DEBUG
    # "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/lib/x64/debug/libprotobuf.lib"
# )
# set(PROTOBUF_PROTOC_EXECUTABLE
    # "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/bin/protoc.exe"
# )

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

add_definitions(-DHEART_LUA_LIBS -DLUA_BUILD_AS_DLL)

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
add_subdirectory("tools/memtrack")
add_subdirectory("tools/viewer")
add_subdirectory("tools/profiler")
