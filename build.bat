@echo off
echo Building XShade RTX Enhancement Tool...

setlocal enabledelayedexpansion

REM --- Search for Visual Studio installations on all drives ---
set "VS_FOUND="
for %%d in (C D E F G H) do (
    for /r %%d:\ %%f in (devenv.exe) do (
        set "VS_FOUND=%%f"
        goto :VS_FOUND
    )
)

REM Also check for vswhere.exe (recommended way)
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
        set "VS_FOUND=%%i\Common7\IDE\devenv.exe"
        goto :VS_FOUND
    )
)

:VS_FOUND
if not defined VS_FOUND (
    echo.
    echo ERROR: No Visual Studio installation found on any drive.
    echo Please install Visual Studio 2022 (Community Edition or higher).
    echo.
    pause
    exit /b 1
) else (
    echo Visual Studio found at: !VS_FOUND!
    pause
)

REM Continue with the rest of your batch file here...
REM (Do not use endlocal until the very end, if at all)

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
