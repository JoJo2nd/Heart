@echo off

if "%~1" == "-d" goto debug
if "%~1" == "-r" goto release
goto default

:default
:debug
cd %~dp0\Debug
goto run

:release
cd %~dp0\Release
goto run

:run
start .\lua.exe .\..\ramuh.lua
cd %~dp0