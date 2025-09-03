@echo off
echo Building XShade RTX Enhancement Tool...

REM Check if CMake is installed and accessible
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake is not installed or not found in PATH
    echo.
    echo CMake 3.20+ is required to build XShade.
    echo Please install CMake from one of these options:
    echo.
    echo 1. Download from: https://cmake.org/download/
    echo    - Choose "Windows x64 Installer"
    echo    - During installation, select "Add CMake to system PATH"
    echo.
    echo 2. Using winget: winget install Kitware.CMake
    echo.
    echo 3. Using chocolatey: choco install cmake
    echo.
    echo After installation, restart your command prompt and try again.
    echo.
    pause
    exit /b 1
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
echo Executable: build\bin\Release\XShade.exe
echo DLL: build\bin\Release\XShadeCore.dll
echo.
echo Note: This application requires administrator privileges to inject into Roblox.
echo Make sure to run as administrator when testing.
echo.
pause
