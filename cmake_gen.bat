@echo off

if not exist cmake_build mkdir cmake_build
pushd cmake_build
set CMAKE_EXE="C:\Program Files (x86)\CMake 2.8\bin\cmake.exe"
set CMAKE_OPTIONS= -G "Visual Studio 11" -DBUILD_HEART_TESTBED=ON ./../
""%CMAKE_EXE% %CMAKE_OPTIONS%""
popd