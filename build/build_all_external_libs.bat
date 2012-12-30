@echo off

echo Building external libs...it may take some time
echo Check logs/ext_build_log_*.txt for details

if not exist logs mkdir logs

echo Building assimp
call build_assimp.bat > logs/ext_build_log_assimp.txt
echo Building nvidia texture tools
call build_nvtt.bat > logs/ext_build_log_nvidia_texture_tools.txt
echo Building Boost
call build_boost_libs.bat > logs/ext_build_log_boost.txt

cd deploy_scripts
call deploy_external_libs.bat