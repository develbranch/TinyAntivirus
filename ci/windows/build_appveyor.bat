:: Build script for TinyAntivirus and all submodules
:: compiler and linker: Visual Studio 2015
@ECHO OFF

:: verify parameters
if "%1" == "" ( 
	echo   [!] Configure Projects to Target Platform
	exit /b 1
)

if "%2" == "" ( 
	echo   [!] Configure projects configuration
	exit /b 1
)

set generator=Visual Studio 14 2015
if /I "%1" == "x64" (set generator=%generator% Win64)

:: build googletest library
set build_gtest_dir=libs\googletest\googletest\build
if exist "%build_gtest_dir%" (
    del /S /Q "%build_gtest_dir%"
) 
md "%build_gtest_dir%"
pushd "%build_gtest_dir%"
cmake -G "%generator%" -DCMAKE_CONFIGURATION_TYPES="%2" -DCMAKE_C_FLAGS_RELEASE="/MT" -DCMAKE_C_FLAGS_DEBUG="/MTd" ..
cmake --build . --config "%2"
popd

:: build zlib library
set build_zlib_dir=libs\zlib\build
if exist "%build_zlib_dir%" (
    del /S /Q "%build_zlib_dir%"
) 
md "%build_zlib_dir%"
pushd "%build_zlib_dir%"
cmake -G "%generator%" -DCMAKE_CONFIGURATION_TYPES="%2" -DCMAKE_C_FLAGS_RELEASE="/MT" -DCMAKE_C_FLAGS_DEBUG="/MTd" ..
cmake --build . --config "%2"
popd

:: setup build environment
IF EXIST "%PROGRAMFILES(X86)%" GOTO win64
call "%PROGRAMFILES%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %1	
goto Build

:win64
call "%PROGRAMFILES(X86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %1

:: build all projects
:Build
msbuild TinyAntivirus.sln /p:Configuration="%2" /p:Platform="%1"