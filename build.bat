@echo off
echo Building XShade RTX Enhancement Tool...

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64
if %errorlevel% neq 0 (
    echo Failed to configure project with CMake
    pause
    exit /b %errorlevel%
)

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Failed to build project
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
