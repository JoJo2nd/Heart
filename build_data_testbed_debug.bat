@echo off
pushd %~dp0bin\Debug
builder.exe -j3 -s %~dp0data -d %~dp0deploy\gamedata
popd