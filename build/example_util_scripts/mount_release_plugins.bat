call mount_common.bat

cd plug_ins

MKLINK /D plugins ..\..\HeartGit\bin\tools\WIN32_RELEASE_PLUGINS
REM MKLINK /D plugins ..\..\HeartGit\_outdir\HeartPacker\RELEASE\WIN32

pause