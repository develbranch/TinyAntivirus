@ECHO OFF
set BUILD_DIR=%2
if "%1" == "x64" (set BUILD_DIR=%1\%BUILD_DIR%)
pushd "%BUILD_DIR%"
Unittests.exe "%SAMPLE_DIR%"
popd
