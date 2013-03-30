IF NOT exist "..\heart\include\events\generated" mkdir "..\heart\include\events\generated"
call %PYTHON_ROOT%python ..\heart\include\events\hEvent_gen.py -i 11 -o "..\heart\include\events\generated\hPuslisher_gen.h"
cd ..\heart\include\base
call %PYTHON_ROOT%python hFunctor_gen.py
cd ..\..\..\build\project_scripts