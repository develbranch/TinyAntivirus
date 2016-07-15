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

:: build googletest library
md libs\googletest\googletest\build
pushd libs\googletest\googletest\build
set generator=Visual Studio 14 2015
if "%1" == "x64" (set generator=%generator% Win64)
cmake -G "%generator%" -DCMAKE_CONFIGURATION_TYPES="%2" ..
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