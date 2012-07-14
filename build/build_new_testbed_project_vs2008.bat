@echo off
cd project_scripts
call ..\..\external\premake4.4\bin\premake4.exe --file="../testbed.sln.lua" vs2008
cd ..