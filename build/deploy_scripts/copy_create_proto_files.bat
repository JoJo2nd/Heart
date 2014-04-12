
set WORKING_DIR=%1
set DEST_DIR=%2

pushd "%WORKING_DIR%"
for %%f in ("*.proto") do (
    echo //Auto generated file. DO NOT EDIT! > "%DEST_DIR%/%%f"
    echo option optimize_for=LITE_RUNTIME; >> "%DEST_DIR%/%%f"
    type "%WORKING_DIR%/%%f" >> "%DEST_DIR%/%%f"
)
popd