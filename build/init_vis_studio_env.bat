
if defined VS100COMNTOOLS goto SETUPVS10
if defined VS90COMNTOOLS goto SETUPVS9
goto ERRORMSG

:SETUPVS9
call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86
set VSTYPE=vc9
set PREMAKEVSVER=vs2008
set VS_EXT=vcproj
set VSUPGRADE=
goto FINISH

:SETUPVS10
call "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
set VSTYPE=vc10
set PREMAKEVSVER=vs2010
set VS_EXT=vcxproj
set VSUPGRADE=/Upgrade
goto FINISH

:ERRORMSG
echo "Visual Studio not found" 

:FINISH