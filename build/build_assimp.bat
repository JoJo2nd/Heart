@echo off

set ASSIMP_VERSION=--3.0.1270-sdk

call init_vis_studio_env.bat

set BUILD_FLR=%cd%
cd ..\external\boost
set BOOST_DIR=%cd%

cd %BUILD_FLR%
devenv ..\external\assimp%ASSIMP_VERSION%\workspaces\%VSTYPE%\assimp.sln /Rebuild "debug-dll|Win32" /Project "assimp"
cd %BUILD_FLR%
devenv ..\external\assimp%ASSIMP_VERSION%\workspaces\%VSTYPE%\assimp.sln /Rebuild "release-dll|Win32" /Project "assimp"