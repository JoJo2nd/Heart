@echo off

call ../init_vis_studio_env.bat

rem usage deploy_nvtt [dest folder]

set NVIDIA_TEX_TOOLS_FOLDER=../../external/nvidia-texture-tools/gnuwin32/bin/
set NVIDIA_TEX_TOOLS_FOLDER2=../../external/nvidia-texture-tools/project/%VSTYPE%/Release (no cuda).Win32/bin/
set ASSIMP_FOLDER_R=../../external/assimp--3.0.1270-sdk/bin/assimp_release-dll_win32
set ASSIMP_FOLDER_D=../../external/assimp--3.0.1270-sdk/bin/assimp_debug-dll_win32
set DEST_FOLDER=%~1
set ROBOCOPY_OPTIONS=/XO /XX /njh /njs /ndl /nc /ns /np /nfl /v

IF NOT exist "%DEST_FOLDER%" mkdir "%DEST_FOLDER%"

ROBOCOPY "%NVIDIA_TEX_TOOLS_FOLDER%" "%DEST_FOLDER%" "*.dll" %ROBOCOPY_OPTIONS%
ROBOCOPY "%NVIDIA_TEX_TOOLS_FOLDER2%" "%DEST_FOLDER%" "*.dll" %ROBOCOPY_OPTIONS%
IF exist "%ASSIMP_FOLDER_R%" ROBOCOPY "%ASSIMP_FOLDER_R%" "%DEST_FOLDER%" "*.dll" "*.pdb" %ROBOCOPY_OPTIONS%
IF exist "%ASSIMP_FOLDER_D%" ROBOCOPY "%ASSIMP_FOLDER_D%" "%DEST_FOLDER%" "*.dll" "*.pdb" %ROBOCOPY_OPTIONS%

IF %ERRORLEVEL% GTR 2 GOTO :REPORT_ERRORLEVEL

:EXIT
EXIT /B 0

:REPORT_ERRORLEVEL
echo.
if ERRORLEVEL 16 echo ***FATAL ERROR*** & goto :FATAL_ERROR
if ERRORLEVEL 8 echo **FAILED COPIES** & goto :FATAL_ERROR
if ERRORLEVEL 4 echo *MISMATCHES* & goto :FATAL_ERROR
if ERRORLEVEL 2 echo EXTRA FILES & goto :EXIT

:FATAL_ERROR
echo ERROR EXIT
REM EXIT