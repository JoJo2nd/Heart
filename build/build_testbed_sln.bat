@echo off
call init_vis_studio_env.bat
cd ..\heart\include\base
call %PYTHON_ROOT%python hFunctor_gen.py
cd ..\..\..\build\project_scripts
call ..\..\external\premake4.4\bin\premake4.exe --file="../testbed.sln.lua" %PREMAKEVSVER%
cd ..
call project_unity_fixer.bat built_projects\%PREMAKEVSVER%\projects\heart.%VS_EXT% %PREMAKEVSVER%