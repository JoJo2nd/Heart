@echo off

call init_vis_studio_env.bat

devenv external\wxWidgets-2.9.2\build\msw\wx_vc9.sln /Build "Debug|Win32"
devenv external\wxWidgets-2.9.2\build\msw\wx_vc9.sln /Build "Release|Win32"
devenv external\wxWidgets-2.9.2\build\msw\wx_vc9.sln /Build "Universal Debug|Win32"
devenv external\wxWidgets-2.9.2\build\msw\wx_vc9.sln /Build "Universal Release|Win32"