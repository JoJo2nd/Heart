@echo off
rem usage deploy_lua_libs [source folder] [dest folder]

set SRC_FOLDER=%~1
set DEST_FOLDER=%~2
set ROBOCOPY_OPTIONS=/XO /XX /njh /njs /ndl /nc /ns /np /W:2

if exist "%SRC_FOLDER%/*.lua" ( 
  echo Copying *.lua files
  set CPYEXT="*.lua"
) else (
  echo Nothing to copy
  goto :EXIT
)

ROBOCOPY "%SRC_FOLDER%" "%DEST_FOLDER%/lua/" %CPYEXT% %ROBOCOPY_OPTIONS%

IF %ERRORLEVEL% GTR 2 GOTO :REPORT_ERRORLEVEL

:EXIT
EXIT /B 0

:REPORT_ERRORLEVEL
if ERRORLEVEL 16 echo ***FATAL ERROR*** & goto :FATAL_ERROR
if ERRORLEVEL 8 echo **FAILED COPIES** & goto :FATAL_ERROR
if ERRORLEVEL 4 echo *MISMATCHES* & goto :FATAL_ERROR
if ERRORLEVEL 2 echo EXTRA FILES & goto :EXIT

:FATAL_ERROR
echo ERROR EXIT
REM EXIT