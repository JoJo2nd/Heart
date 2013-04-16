@echo off

if not exist platform_win32 mkdir platform_win32
pushd platform_win32
    set CMAKE_EXE="C:\Program Files (x86)\CMake 2.8\bin\cmake.exe"
    set CMAKE_OPTIONS= -G "Visual Studio 11" -DBUILD_HEART_TESTBED=ON ./../
    ""%CMAKE_EXE% %CMAKE_OPTIONS%""
popd
if not exist platform_win64 mkdir platform_win64
pushd platform_win64
    set CMAKE_EXE="C:\Program Files (x86)\CMake 2.8\bin\cmake.exe"
    set CMAKE_OPTIONS= -G "Visual Studio 11 Win64" -DBUILD_HEART_TESTBED=ON ./../
    ""%CMAKE_EXE% %CMAKE_OPTIONS%""
popd