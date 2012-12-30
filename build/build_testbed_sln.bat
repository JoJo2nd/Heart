@echo off
call init_vis_studio_env.bat
IF NOT exist "..\heart\include\events\generated" mkdir "..\heart\include\events\generated"
call %PYTHON_ROOT%python ..\heart\include\events\hEvent_gen.py -i 11 -o "..\heart\include\events\generated\hPuslisher_gen.h"
cd ..\heart\include\base
call %PYTHON_ROOT%python hFunctor_gen.py
cd ..\..\..\build\project_scripts
call ..\..\external\premake4.4\bin\premake4.exe --file="testbed.sln.lua" %PREMAKEVSVER%
cd ..
%PYTHON_ROOT%\python.exe scripts\project_unity_fixer.py -p "built_projects\%PREMAKEVSVER%\game\projects\heart.%VS_EXT%" -m %PREMAKEVSVER% -r "project_scripts\heart_unity_regex.txt" -b "project_scripts\heart_unity_build_platforms.txt"