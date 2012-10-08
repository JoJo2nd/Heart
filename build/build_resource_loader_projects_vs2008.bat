@echo off
cd project_scripts
call ..\..\external\premake4.4\bin\premake4.exe --file="../ResourceLoaders.sln.lua" vs2008
cd ..
REM call project_unity_fixer.bat built_projects\vs2008\projects\heartbase.vcproj
REM call project_unity_fixer.bat built_projects\vs2008\projects\heartwin32.vcproj
REM call project_unity_fixer.bat built_projects\vs2008\projects\heartcore.vcproj