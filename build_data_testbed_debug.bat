@echo off
pushd %~dp0bin\Debug
echo -j3 -s %~dp0data -d %~dp0deploy\gamedata
builder.exe -j3 -s %~dp0data -d %~dp0deploy\gamedata
popd