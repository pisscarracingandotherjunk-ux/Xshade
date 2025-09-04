@echo off
echo Building XShade RTX Enhancement Tool...
pause

REM Visual Studio detection
set "VS_FOUND="
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" (
    set "VS_FOUND=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe"
)
if not defined VS_FOUND (
    echo ERROR: No Visual Studio found.
    pause
    exit /b 1
) else (
    echo Visual Studio found at: !VS_FOUND!
    pause
)

REM CMake detection
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found.
    pause
    exit /b 1
) else (
    echo CMake found.
    pause
)

REM Check CMake version
for /f "tokens=3" %%i in ('cmake --version ^| findstr /C:"cmake version"') do set CMAKE_VERSION=%%i
echo Found CMake version: %CMAKE_VERSION%

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64
if %errorlevel% neq 0 (
    echo.
    echo Failed to configure project with CMake
    echo.
    echo This might be caused by:
    echo - Visual Studio 2022 not installed
    echo - Missing Windows SDK
    echo - Missing required dependencies
    echo.
    echo Please ensure you have:
    echo - Visual Studio 2022 (Community Edition or higher)
    echo - Windows 10/11 SDK
    echo - CMake 3.20+
    echo.
    pause
    exit /b %errorlevel%
)

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo.
    echo Failed to build project
    echo.
    echo Common causes:
    echo - Missing Visual Studio 2022 C++ build tools
    echo - Missing Windows SDK components
    echo - Source code compilation errors
    echo.
    echo Check the output above for specific error details.
    echo.
    pause
    exit /b %errorlevel%
)

echo.
echo Build completed successfully!
echo Executable: bin\Release\XShade.exe
echo DLL: bin\Release\XShadeCore.dll
echo.
echo Note: This application requires administrator privileges to inject into Roblox.
echo Make sure to run as administrator when testing.
echo.

if exist "bin\Release\XShade.exe" (
    echo XShade.exe was built successfully!
) else (
    echo ERROR: XShade.exe was not found after build.
)
pause
