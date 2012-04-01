call mount_common.bat

cd plug_ins

MKLINK /D plugins ..\..\HeartGit\bin\tools\WIN32_DEBUG_PLUGINS
REM MKLINK /D plugins ..\..\HeartGit\_outdir\HeartPacker\DEBUG\WIN32

pause