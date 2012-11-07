@echo off

set BIN_FOLDER=../../bin/
set GAME_BIN_FOLDER=%BIN_FOLDER%game/
set TOOLS_BIN_FOLDER=%BIN_FOLDER%tools/
set NVIDIA_TEX_TOOLS_FOLDER=../../nvidia-texture-tools/gnuwin32/bin/
set NVIDIA_TEX_TOOLS_FOLDER2=../../nvidia-texture-tools/project/vc8/Release (no cuda).Win32/bin/
set ASSIMP_FOLDER_R=../../external/assimp--3.0.1270-sdk/bin/assimp_release-dll_win32
set ASSIMP_FOLDER_D=../../external/assimp--3.0.1270-sdk/bin/assimp_debug-dll_win32
set HEART_BIN_FOLDER_R=../../build/built_projects/vs2008/lib/Release
set HEART_BIN_FOLDER_D=../../build/built_projects/vs2008/lib/Debug
set ROBOCOPY_OPTIONS=/XO /njh /njs /ndl /nc /ns /np /nfl

IF NOT exist "%BIN_FOLDER%" mkdir "%BIN_FOLDER%"
IF NOT exist "%GAME_BIN_FOLDER%" mkdir "%GAME_BIN_FOLDER%"
IF NOT exist "%TOOLS_BIN_FOLDER%" mkdir "%TOOLS_BIN_FOLDER%"

ROBOCOPY "%NVIDIA_TEX_TOOLS_FOLDER%" "%GAME_BIN_FOLDER%" "*.dll" %ROBOCOPY_OPTIONS%
ROBOCOPY "%NVIDIA_TEX_TOOLS_FOLDER2%" "%GAME_BIN_FOLDER%" "*.dll" %ROBOCOPY_OPTIONS%
ROBOCOPY "%ASSIMP_FOLDER_R%" "%GAME_BIN_FOLDER%" "*.dll" %ROBOCOPY_OPTIONS%
ROBOCOPY "%ASSIMP_FOLDER_D%" "%GAME_BIN_FOLDER%" "*.dll" %ROBOCOPY_OPTIONS%
ROBOCOPY "%HEART_BIN_FOLDER_D%" "%GAME_BIN_FOLDER%" "*.dll" "*.exe" %ROBOCOPY_OPTIONS%
ROBOCOPY "%HEART_BIN_FOLDER_R%" "%GAME_BIN_FOLDER%" "*.dll" "*.exe" %ROBOCOPY_OPTIONS%

IF %ERRORLEVEL% GTR 2 GOTO :REPORT_ERRORLEVEL

EXIT /B 0

:REPORT_ERRORLEVEL
echo.
if ERRORLEVEL 16 echo ***FATAL ERROR*** & goto :FATAL_ERROR
if ERRORLEVEL 8 echo **FAILED COPIES** & goto :FATAL_ERROR
if ERRORLEVEL 4 echo *MISMATCHES* & goto :FATAL_ERROR
if ERRORLEVEL 2 echo EXTRA FILES & goto :FATAL_ERROR

:FATAL_ERROR
echo ERROR EXIT
REM EXIT