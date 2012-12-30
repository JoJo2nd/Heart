@echo off

call ../init_vis_studio_env.bat

set BIN_FOLDER=../../bin/
set GAME_BIN_FOLDER=%BIN_FOLDER%game/
set TOOLS_BIN_FOLDER=%BIN_FOLDER%tools/
set HEART_BIN_FOLDER_R=../../build/built_projects/%PREMAKEVSVER%/game/lib/Release/
set HEART_BIN_FOLDER_D=../../build/built_projects/%PREMAKEVSVER%/game/lib/Debug/
set ROBOCOPY_OPTIONS=/XO /XX /njh /njs /ndl /nc /ns /np

IF NOT exist "%BIN_FOLDER%" mkdir "%BIN_FOLDER%"
IF NOT exist "%GAME_BIN_FOLDER%" mkdir "%GAME_BIN_FOLDER%"
IF NOT exist "%TOOLS_BIN_FOLDER%" mkdir "%TOOLS_BIN_FOLDER%"

if exist "%HEART_BIN_FOLDER_R%%1.dll" ( 
  set CPYEXT="%1.dll" "%1.pdb"
) else if exist "%HEART_BIN_FOLDER_R%%1.exe" ( 
  set CPYEXT="%1.exe" "%1.pdb"
) else if exist "%HEART_BIN_FOLDER_D%%1.dll" ( 
  set CPYEXT="%1.dll" "%1.pdb"
) else if exist "%HEART_BIN_FOLDER_D%%1.exe" ( 
  set CPYEXT="%1.exe" "%1.pdb"
) else (
  goto :EXIT
)

IF exist "%HEART_BIN_FOLDER_D%" ROBOCOPY "%HEART_BIN_FOLDER_D%" "%GAME_BIN_FOLDER%" %CPYEXT% %ROBOCOPY_OPTIONS%
IF exist "%HEART_BIN_FOLDER_R%" ROBOCOPY "%HEART_BIN_FOLDER_R%" "%GAME_BIN_FOLDER%" %CPYEXT% %ROBOCOPY_OPTIONS%

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