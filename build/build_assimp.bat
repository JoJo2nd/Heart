@echo off

call init_vis_studio_env.bat

devenv ..\external\assimp\workspaces\vc9\assimp.sln /Build "debug-noboost-st|Win32"
devenv ..\external\assimp\workspaces\vc9\assimp.sln /Build "release-noboost-st|Win32"