cmake_minimum_required(VERSION 2.8)

project(HEART_TESTBED)

option(USE_XNAMATH "Use old XNA math libraries" OFF)
option(USE_OLD_DXSDK "Use old DirectX SDK. Will cause issues with Windows 8 SDK" OFF)

set(EXTERNAL_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/external"
)
set(HEART_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/heart"
)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
if (USE_XNAMATH)
    add_definitions(-DHEART_USE_XNAMATH)
endif()
if (USE_OLD_DXSDK)
    add_definitions(-DHEART_USE_DXSDK)
endif()


add_subdirectory ("external/zlib")
add_subdirectory ("external/minizip")
add_subdirectory ("external/lua")
add_subdirectory ("heart")