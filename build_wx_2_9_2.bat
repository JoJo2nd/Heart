@echo off

call init_vis_studio_env.bat

devenv external\wxWidgets-2.9.2\build\msw\wx_vc9.sln /Rebuild "Debug|Win32"
devenv external\wxWidgets-2.9.2\build\msw\wx_vc9.sln /Rebuild "Release|Win32"