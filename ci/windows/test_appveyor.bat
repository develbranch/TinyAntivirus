@ECHO OFF
set APP_PATH="%2"\Unittests.exe
if "%1" == "x64" (set APP_PATH=x64\"%2"\Unittests.exe)
%APP_PATH% "%SAMPLE_DIR%"

