rem @echo off

set WORKING_DIR="%1"
set DEST_FILE=%3
set SRC_FILE=%2

echo //Auto generated file. DO NOT EDIT! > %DEST_FILE%
echo option optimize_for=LITE_RUNTIME; >> %DEST_FILE%
pushd %WORKING_DIR%
type %SRC_FILE% >> %DEST_FILE%
popd