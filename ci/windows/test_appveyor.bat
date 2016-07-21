@ECHO OFF
set APP_PATH="%2\Unittests.exe"
if "%1" == "x64" (set APP_PATH="x64\%2\Unittests.exe")
echo  "%1"
echo  "%2"
echo  %APP_PATH%
%APP_PATH% "%SAMPLE_DIR%"

