@echo off

call init_vis_studio_env.bat

devenv ..\external\nvidia-texture-tools\project\%VSTYPE%\nvtt.sln /Rebuild "Debug (no cuda)|Win32"
devenv ..\external\nvidia-texture-tools\project\%VSTYPE%\nvtt.sln /Rebuild "Release (no cuda)|Win32"