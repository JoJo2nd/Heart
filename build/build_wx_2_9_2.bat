@echo off

call init_vis_studio_env.bat

devenv ..\external\wxWidgets-2.9.2\build\msw\wx_%VSTYPE%.sln /Build "Debug|Win32"
devenv ..\external\wxWidgets-2.9.2\build\msw\wx_%VSTYPE%.sln /Build "Release|Win32"
rem devenv ..\external\wxWidgets-2.9.2\build\msw\wx_%VSTYPE%.sln /Build "Universal Debug|Win32"
rem devenv ..\external\wxWidgets-2.9.2\build\msw\wx_%VSTYPE%.sln /Build "Universal Release|Win32"