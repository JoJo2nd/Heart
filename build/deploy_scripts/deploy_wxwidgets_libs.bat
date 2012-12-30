@echo off

REM arg 1 = dist type, so u (release) or ud(debug)
REM arg 2 = dest folder

call ../init_vis_studio_env.bat

set wxWIDGET_COPY="wxmsw294%1_*_%VSTYPE%0.*" "wxbase294%1_*_%VSTYPE%0.*" "wxbase294%1_%VSTYPE%0.*"
set wxWIDGET_LIB_FOLDER=../../external/wxWidgets-2.9.4/lib/%VSTYPE%0_dll
set ROBOCOPY_OPTIONS=/XO /XX /njh /njs /ndl /nc /ns /np

ROBOCOPY "%wxWIDGET_LIB_FOLDER%" "%2" %wxWIDGET_COPY% %ROBOCOPY_OPTIONS%

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