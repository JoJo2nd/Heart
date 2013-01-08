@echo off

set CURDIR=%cd%
set bp_cwd=%cd%
set heartbuild=%bp_cwd%
set gamebuild=%bp_cwd%\project_scripts
set loggingroot=%bp_cwd%/logs/
set HEART_BUILD_ROOT=%bp_cwd%/built_projects/
set HEART_REPO_ROOT=%bp_cwd%/../
set HEART_BIN_TOOL_ROOT=%bp_cwd%/../bin/tools/
set HEART_BIN_GAME_ROOT=%bp_cwd%/../bin/game/
set HEART_BIN_ROOT=%bp_cwd%/../bin/

echo Heart Build Path = %heartbuild%
echo Game Build Path = %gamebuild%
echo HEART_BUILD_ROOT = %HEART_BUILD_ROOT%
echo HEART_REPO_ROOT = %HEART_REPO_ROOT%
echo HEART_BIN_TOOL_ROOT = %HEART_BIN_TOOL_ROOT%
echo HEART_BIN_GAME_ROOT = %HEART_BIN_GAME_ROOT%
echo HEART_BIN_ROOT = %HEART_BIN_ROOT%

if not exist logs mkdir logs

call init_vis_studio_env.bat

cd %CURDIR%
echo Building testbed solution
call build_testbed_sln.bat > %loggingroot%testbed_sln.txt
cd tools
echo Building memtrack solution
call build_memtrack_project.bat > %loggingroot%memtrack_project.txt
echo Building viewer solution
call build_viewer_project.bat > %loggingroot%viewer_project.txt
cd ..

echo ...Done
pause