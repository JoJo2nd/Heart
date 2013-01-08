@echo off

echo Building external libs...it may take some time
echo Check %loggingroot%/ext_build_log_*.txt for details

echo Building Boost
call build_boost_libs.bat > %loggingroot%/ext_build_log_boost.txt
echo Building assimp
call build_assimp.bat > %loggingroot%/ext_build_log_assimp.txt
echo Building nvidia texture tools
call build_nvtt.bat > %loggingroot%/ext_build_log_nvidia_texture_tools.txt