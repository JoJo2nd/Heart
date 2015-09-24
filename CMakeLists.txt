cmake_minimum_required(VERSION 2.8.12.2)

project(HEART)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(DEPLOY_TOOLS_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/build/deploy_scripts"
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(BUILD_PLATFORM "windows")
  set(PLATFORM_WINDOWS true)
  set(PROTOBUF_LUA_PLUGIN protoc-gen-lua.exe)
  set(PROTOBUF_CPPDIFF_PLUGIN protoc-gen-cppdiff.exe)
  add_definitions(-DPLATFORM_WINDOWS)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(ROBOCOPY_OPTIONS /XO /XX /njh /njs /ndl /nc /ns /np /W:2)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG=1")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1")
  set(BUILD_PLATFORM "linux")
  set(PLATFORM_LINUX true)
  set(PROTOBUF_LUA_PLUGIN protoc-gen-lua)
  set(PROTOBUF_CPPDIFF_PLUGIN protoc-gen-cppdiff)
  add_definitions(-DPLATFORM_LINUX)
  add_definitions(-DGLEW_NO_GLU)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -msse -msse2") # add -v for verbose
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -msse -msse2") # add -v for verbose
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}") # add --verbose for verbose

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
    list(APPEND ${SRCS} "${PROTO_IM_DIR}/${FIL_WE}.pb.diff.cc")
    list(APPEND ${HDRS} "${PROTO_IM_DIR}/${FIL_WE}.pb.diff.h")

	set_source_files_properties(${ABS_FIL} PROPERTIES HEADER_FILE_ONLY TRUE)
	get_filename_component(ABS_ROOT ${ABS_FIL} DIRECTORY)

    add_custom_command(
      OUTPUT "${PROTO_IM_DIR}/${FIL_WE}.pb.cc"
             "${PROTO_IM_DIR}/${FIL_WE}.pb.diff.cc"
             "${PROTO_IM_DIR}/${FIL_WE}.pb.h"
             "${PROTO_IM_DIR}/${FIL_WE}.pb.diff.h"
	    COMMAND python ARGS "${DEPLOY_TOOLS_DIR}/copy_create_proto_files.py" "-s" "${ABS_FIL}" "-d" "${PROTO_IM_DIR}"
	    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ARGS --proto_path=${PROTO_IM_DIR} --cpp_out=${PROTO_IM_DIR} --cppdiff_out=${PROTO_IM_DIR} ${PROTO_IM_FILE} --plugin=protoc-gen-cppdiff=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${PROTOBUF_CPPDIFF_PLUGIN}
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

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pre_game.txt")
  include("${CMAKE_CURRENT_SOURCE_DIR}/pre_game.txt")
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

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4091") #MP
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4091")
endif()

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
add_definitions(-DHEART_SHARED_LIB_EXPORTS=0)

if (FORCE_DISABLE_MEMORY_TRACKING)
	add_definitions(-DHEART_FORCE_DISABLE_TRACK_MEMORY_ALLOCS)
elseif (FORCE_ENABLE_MEMORY_TRACKING)
	add_definitions(-DHEART_FORCE_TRACK_MEMORY_ALLOCS)
endif(FORCE_DISABLE_MEMORY_TRACKING)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/modules/")

## Common code for finding libs
find_package(OpenGL REQUIRED)

## Platform Handling
if (PLATFORM_WINDOWS)
  #Setup from prebuild libraries
  ## Protobuf ##
  set(PROTOBUF_PROTOC_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/vs2015/bin/Release/protoc.exe")
  set(PROTOBUF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/vs2015/include")
  set(PROTOBUF_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/vs2015/lib/Release/libprotobuf.lib")
  set(PROTOBUF_LITE_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/vs2015/lib/Release/libprotobuf-lite.lib")
  set(PROTOBUF_PROTOC_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/vs2015/lib/Release/libprotoc.lib")
  ## Protobuf Debug ##
  set(PROTOBUF_LIBRARY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/vs2015/lib/Debug/libprotobuf.lib")
  set(PROTOBUF_LITE_LIBRARY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/vs2015/lib/Debug/libprotobuf-lite.lib")
  set(PROTOBUF_PROTOC_LIBRARY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/vs2015/lib/Debug/libprotoc.lib")
  ## SDL2 ##
  set(SDL2_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/lib/x64/SDL2.lib")
  set(SDL2_FOUND TRUE)
  set(SDL2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/include")
  set(SDL2_BINARY "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/lib/x64/SDL2.dll")
  ## FreeType ##
  set(FREETYPE_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/vs2015/lib/Release/freetype253.lib")
  set(FREETYPE_FOUND TRUE)
  set(FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/vs2015/include")
  set(FREETYPE_INCLUDE_DIR_ft2build "${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/vs2015/include/config")
  set(FREETYPE_INCLUDE_DIR_freetype2 "${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/vs2015/include/config")
  ## GLEW ##
  set(GLEW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include") #- include directories for GLEW
  set(GLEW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/lib/Release/x64/glew32.lib") #- libraries to link against GLEW
  set(GLEW_FOUND TRUE) #- true if GLEW has been found and can be used
  set(GLEW_BIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/bin/Release/x64/glew32.dll") # - Non-standard
  ## NVTT ##
  set(NVTT_LIBRARIES 
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Release/bc6h.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Release/bc7.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Release/nvcore.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Release/nvimage.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Release/nvmath.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Release/nvthread.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Release/nvtt.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Release/squish.lib")
  set(NVTT_LIBRARIES_DEBUG
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Debug/bc6h.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Debug/bc7.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Debug/nvcore.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Debug/nvimage.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Debug/nvmath.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Debug/nvthread.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Debug/nvtt.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/lib/Debug/squish.lib")
  set(NVTT_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/external/nvtt/src")

else()
  find_package(Protobuf REQUIRED)
  find_package(SDL2 REQUIRED)
  find_package(Freetype REQUIRED)
  find_package(FreeImage REQUIRED)
  find_package(GLEW REQUIRED)
endif(PLATFORM_WINDOWS)

#Common var
set(DEPLOY_TOOLS_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/build/deploy_scripts"
)
set(EXTERNAL_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/external"
)
set(TOOLS_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/tools"
)
set(HEART_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/heart/src"
)
set(GENERATED_PROTO_INC_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}/tools/proto/pb_lite/"
    #add more as needed
)
set(GENERATED_PROTO_FULL_INC_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}/tools/proto_full"
    #add more as needed
)
set(ENET_INCLUDE_DIR
    "${EXTERNAL_SRC_ROOT}/enet/include"
)
set(LUA_INCLUDE_DIR
    "${EXTERNAL_SRC_ROOT}/lua/src"
)
set(MINFS_INCLUDE_DIR
    "${EXTERNAL_SRC_ROOT}/minfs/src"
)
set(GETOPT_PORT_INCLUDE_DIR
    "${EXTERNAL_SRC_ROOT}/getopt_port"
)
set(CRYPTO_INCLUDE_DIR
    "${EXTERNAL_SRC_ROOT}/crypto/src"
)
set(IMGUI_INCLUDE_DIR
    "${EXTERNAL_SRC_ROOT}/imgui/src"
)
set(PROTO_SRC_DIR
    "${TOOLS_SRC_ROOT}/proto/src"
)
set(PROTO_LUA_SRC_DIR
  ${PROTO_SRC_DIR}
  ${GENERATED_PROTO_FULL_INC_DIRS}
)
set(MEMTRACKERLIB_SRC_DIR
    "${TOOLS_SRC_ROOT}/memtrackerlib/src"
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

# Add testbed proto buffer files
set(ADDITIONAL_PROTO_SRC_DIRS 
  ${ADDITIONAL_PROTO_SRC_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}/testbed/proto"
)

add_subdirectory ("external/getopt_port")
add_subdirectory ("external/zlib")
add_subdirectory ("external/crypto")
add_subdirectory ("external/lua")
add_subdirectory ("external/minizip")
add_subdirectory ("external/libvorbis")
add_subdirectory ("external/libogg")
add_subdirectory ("external/enet")
add_subdirectory ("external/minfs")
#add_subdirectory ("external/nvidia-texture-tools")
add_subdirectory ("external/imgui")
if (PLATFORM_WINDOWS)
  add_subdirectory ("external/libprof")
endif()
# !!JM Disabled. May reimplement in the future
#add_subdirectory ("external/luadb")
add_subdirectory ("external/protobuf/cppdiff")
add_subdirectory ("heart")

add_subdirectory ("testbed")

add_subdirectory("resourceloaders/shaderloader")
add_subdirectory("resourceloaders/textureloader")
add_subdirectory("resourceloaders/materialloader")
add_subdirectory("resourceloaders/meshloader")
add_subdirectory("resourceloaders/fontloader")
add_subdirectory("resourceloaders/entitydefloader")

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
add_subdirectory("tools/memtrackerlib")
add_subdirectory("tools/builder")
add_subdirectory("tools/pbscan")
add_subdirectory("tools/pkginspector")
if (PLATFORM_WINDOWS)
  add_subdirectory("tools/profiler")
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/post_game.txt")
  include("${CMAKE_CURRENT_SOURCE_DIR}/post_game.txt")
endif()
