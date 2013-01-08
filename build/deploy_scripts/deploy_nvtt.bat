@echo off
call ../init_vis_studio_env.bat

rem usage deploy_nvtt [destfolder]

set NVIDIA_TEX_TOOLS_FOLDER=../../external/nvidia-texture-tools/gnuwin32/bin/
set NVIDIA_TEX_TOOLS_FOLDER2=../../external/nvidia-texture-tools/project/%VSTYPE%/Release (no cuda).Win32/bin/
set NVTT_COPY="nvtt.dll" "jpeg62.dll" "libpng12.dll" "libtiff3.dll" "zlib1.dll"
set ROBOCOPY_OPTIONS=/XO /XX /njh /njs /ndl /nc /ns /np

ROBOCOPY "%NVIDIA_TEX_TOOLS_FOLDER%" "%1" %NVTT_COPY% %ROBOCOPY_OPTIONS%
ROBOCOPY "%NVIDIA_TEX_TOOLS_FOLDER2%" "%1" %NVTT_COPY% %ROBOCOPY_OPTIONS%

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