@echo off
rem usage deploy_lib [lib name] [source folder] [dest folder]

REM call ../init_vis_studio_env.bat

set SRC_FOLDER=%~2
set DEST_FOLDER=%~3
set ROBOCOPY_OPTIONS=/XO /XX /njh /njs /ndl /nc /ns /np

if exist "%SRC_FOLDER%/%1*.dll" ( 
  echo Copying DLL
  set CPYEXT="%1*.dll" "%1*.pdb"
) else if exist "%SRC_FOLDER%/%1*.exe" ( 
  echo Copying EXE
  set CPYEXT="%1*.exe" "%1*.pdb"
) else (
  echo Nothing to copy
  goto :EXIT
)

ROBOCOPY "%SRC_FOLDER%" "%DEST_FOLDER%" %CPYEXT% %ROBOCOPY_OPTIONS%

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