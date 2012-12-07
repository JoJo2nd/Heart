@echo off

call init_vis_studio_env.bat

devenv ..\external\nvidia-texture-tools\project\%VSTYPE%\nvtt.sln /Build "Debug (no cuda)|Win32"
devenv ..\external\nvidia-texture-tools\project\%VSTYPE%\nvtt.sln /Build "Release (no cuda)|Win32"