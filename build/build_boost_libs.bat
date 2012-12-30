@echo off

cd ../external/boost

call bootstrap.bat

b2.exe -a

cd ../../build