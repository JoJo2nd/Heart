cmake_minimum_required(VERSION 2.8)

#macros
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

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

#config vars
set(CMAKE_USE_RELATIVE_PATHS ON)
set(CMAKE_DEBUG_POSTFIX "_d")
set(ROBOCOPY_OPTIONS /XO /XX /njh /njs /ndl /nc /ns /np)

#build options
option(USE_XNAMATH "Use old XNA math libraries" OFF)
option(USE_OLD_DXSDK "Use old DirectX SDK. Will cause issues with Windows 8 SDK" OFF)
option(BUILD_HEART_TESTBED "Build testbed for heart" OFF)
option(BUILD_BUILDER_PROJECT "Build builder lib and stand alone" ON)

#Common var
set(EXTERNAL_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/external"
)
set(HEART_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/heart"
)
if (WIN32)
	set(PLATFORM_LIBS dbghelp dxguid xinput d3d11 d3dcompiler openAL32)
endif()

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

if (BUILD_HEART_TESTBED)
	project(HEART_TESTBED)
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
add_definitions(-DHEART_LUA_LIBS -DLUA_COMPAT_MODULE -DLUA_BUILD_AS_DLL)

add_subdirectory ("external/zlib")
add_subdirectory ("external/crypto")
add_subdirectory ("external/lua")
add_subdirectory ("external/minizip")
add_subdirectory ("external/libvorbis")
add_subdirectory ("external/libogg")
add_subdirectory ("heart")
if (BUILD_HEART_TESTBED)
	add_subdirectory ("testbed")
endif()

add_subdirectory("tools/lua")
add_subdirectory("tools/lualibs/filesystem")
add_subdirectory("tools/lualibs/luasocket")
add_subdirectory("tools/lualibs/mime")
