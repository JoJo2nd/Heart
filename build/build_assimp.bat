@echo off

set ASSIMP_VERSION=--3.0.1270-sdk

call init_vis_studio_env.bat

devenv ..\external\assimp%ASSIMP_VERSION%\workspaces\%VSTYPE%\assimp.sln /Build "debug-noboost-st|Win32"
devenv ..\external\assimp%ASSIMP_VERSION%\workspaces\%VSTYPE%\assimp.sln /Build "release-noboost-st|Win32"