@echo off
echo XShade Build Environment Setup
echo =============================
echo.

REM Check if running as administrator
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Not running as administrator
    echo Some installation methods may require admin privileges
    echo.
)

REM Check if CMake is already installed
cmake --version >nul 2>&1
if %errorlevel% equ 0 (
    for /f "tokens=3" %%i in ('cmake --version ^| findstr /C:"cmake version"') do set CMAKE_VERSION=%%i
    echo CMake is already installed: %CMAKE_VERSION%
    echo.
    goto :check_vs
)

echo CMake is not installed or not in PATH
echo.
echo Choose installation method:
echo 1. Install using winget (recommended)
echo 2. Install using chocolatey
echo 3. Manual download instructions
echo 4. Skip CMake installation
echo.
set /p choice="Enter your choice (1-4): "

if "%choice%"=="1" goto :install_winget
if "%choice%"=="2" goto :install_choco
if "%choice%"=="3" goto :manual_instructions
if "%choice%"=="4" goto :check_vs
echo Invalid choice, defaulting to manual instructions
goto :manual_instructions

:install_winget
echo.
echo Installing CMake using winget...
winget install Kitware.CMake
if %errorlevel% neq 0 (
    echo Failed to install CMake using winget
    echo You may need to install winget first or try another method
    goto :manual_instructions
) else (
    echo CMake installed successfully!
    echo Please restart your command prompt for PATH changes to take effect
)
goto :check_vs

:install_choco
echo.
echo Installing CMake using chocolatey...
choco install cmake -y
if %errorlevel% neq 0 (
    echo Failed to install CMake using chocolatey
    echo You may need to install chocolatey first or try another method
    goto :manual_instructions
) else (
    echo CMake installed successfully!
    echo Please restart your command prompt for PATH changes to take effect
)
goto :check_vs

:manual_instructions
echo.
echo Manual CMake Installation:
echo.
echo 1. Go to: https://cmake.org/download/
echo 2. Download "Windows x64 Installer"
echo 3. Run the installer
echo 4. During installation, check "Add CMake to system PATH for all users"
echo 5. Complete the installation
echo 6. Restart your command prompt
echo.
goto :check_vs

:check_vs
echo.
echo Checking for Visual Studio 2022...
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo Visual Studio 2022 C++ compiler not found in PATH
    echo.
    echo You need Visual Studio 2022 with C++ development tools:
    echo 1. Download Visual Studio Community 2022 (free)
    echo 2. During installation, select "Desktop development with C++"
    echo 3. Make sure Windows SDK is included
    echo.
    echo Download from: https://visualstudio.microsoft.com/downloads/
    echo.
) else (
    echo Visual Studio C++ compiler found!
)

echo.
echo Setup complete! You can now run build.bat to build XShade.
echo.
pause
