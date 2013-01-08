@echo off
set CURDIR=%cd%
call init_vis_studio_env.bat
call build_all_external_libs.bat
cd %CURDIR%
echo Building testbed solution
call build_testbed_sln.bat > logs/testbed_sln.txt
cd tools
echo Building memtrack solution
call build_memtrack_project.bat > ../logs/memtrack_project.txt
echo Building viewer solution
call build_viewer_project.bat > ../logs/viewer_project.txt
cd ..
echo Building testbed
devenv built_projects\%PREMAKEVSVER%\game\NewTestBed\NewTestBed.sln /Rebuild "Debug|Win32" > logs/testbed_debug_build_log.txt
devenv built_projects\%PREMAKEVSVER%\game\NewTestBed\NewTestBed.sln /Rebuild "Release|Win32" > logs/testbed_release_build_log.txt
echo Building memtrack
devenv built_projects\%PREMAKEVSVER%\tools\memtrack\memtrack.sln /Rebuild "Debug|Win32" > logs/memtrack_debug_build_log.txt
devenv built_projects\%PREMAKEVSVER%\tools\memtrack\memtrack.sln /Rebuild "Release|Win32" > logs/memtrack_release_build_log.txt
echo Building viewer
devenv built_projects\%PREMAKEVSVER%\tools\viewer\viewer.sln /Rebuild "Debug|Win32" > logs/viewer_debug_build_log.txt
devenv built_projects\%PREMAKEVSVER%\tools\viewer\viewer.sln /Rebuild "Release|Win32" > logs/viewer_release_build_log.txt
echo ...Done
pause