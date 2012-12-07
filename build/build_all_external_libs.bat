@echo off

echo Building external libs...it may take some time
echo Check logs/ext_build_log_*.txt for details

if not exist logs mkdir logs

echo Building Boost
call build_boost_libs.bat > logs/ext_build_log_boost.txt
echo Building wxWidgets (2.9.2)
call build_wx_2_9_2.bat > logs/ext_build_log_wx.txt
echo Building assimp
call build_assimp.bat > logs/ext_build_log_assimp.txt
echo Building nvidia texture tools
call build_nvtt.bat > logs/ext_build_log_nvidia_texture_tools.txt