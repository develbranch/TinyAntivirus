@ECHO OFF
set APP_PATH="%2\Unittests.exe"
if /I "%1" == "x64" (set APP_PATH="%1\%2\Unittests.exe")
%APP_PATH% "%SAMPLE_DIR%"

