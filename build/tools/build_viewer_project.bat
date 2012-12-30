@echo off

call ..\init_vis_studio_env.bat
call ..\..\external\premake4.4\bin\premake4.exe --file="viewer.sln.lua" %PREMAKEVSVER%