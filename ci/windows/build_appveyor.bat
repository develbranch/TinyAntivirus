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
md libs\googletest\googletest\build
pushd libs\googletest\googletest\build

if /I "%2" == "Debug" (
cmake -G "%generator%" -DCMAKE_CONFIGURATION_TYPES="%2" -DBUILD_SHARED_LIBS=ON ..
) ELSE (
cmake -G "%generator%" -DCMAKE_CONFIGURATION_TYPES="%2" ..
)
cmake --build . --config "%2"
popd

:: build zlib library
md libs\zlib\build
pushd libs\zlib\build
set ZLIB_C_FLAGS=
if /I "%2" == "Release" (set ZLIB_C_FLAGS=/MT)
cmake -G "%generator%" -DCMAKE_CONFIGURATION_TYPES="%2" -DCMAKE_C_FLAGS_RELEASE="%ZLIB_C_FLAGS%" ..
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