rem @echo off

echo //Auto generated file. DO NOT EDIT! > "%3"
echo option optimize_for=LITE_RUNTIME; >> "%3"
pushd "%1"
type "%2" >> "%3"
popd