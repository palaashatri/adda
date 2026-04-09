@echo off
setlocal EnableDelayedExpansion

cd /d "%~dp0"

set BUILD_DIR=build
set BUILD_TYPE=Release
set RUN_DEMO=
set INSTALL_DEPS=0
set CLEAN=0

:parse_args
if "%~1"=="" goto args_done

if /I "%~1"=="--build-type" (
    if "%~2"=="" (
        echo --build-type requires a value
        exit /b 1
    )
    set BUILD_TYPE=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="--run" (
    if "%~2"=="" (
        echo --run requires a demo name
        exit /b 1
    )
    set RUN_DEMO=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="--install-deps" (
    set INSTALL_DEPS=1
    shift
    goto parse_args
)

if /I "%~1"=="--clean" (
    set CLEAN=1
    shift
    goto parse_args
)

if /I "%~1"=="--help" goto help

echo Unknown option: %~1
goto help

:args_done
if not "%RUN_DEMO%"=="" (
    if /I not "%RUN_DEMO:~0,5%"=="demo_" (
        echo Unsupported demo target name: %RUN_DEMO%
        echo Demo target must start with demo_
        exit /b 1
    )
)

call :ensure_cmake || exit /b 1
call :ensure_compiler || exit /b 1

if "%CLEAN%"=="1" (
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

cmake -S . -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 exit /b %errorlevel%

cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%
if errorlevel 1 exit /b %errorlevel%

if not "%RUN_DEMO%"=="" (
    call :run_demo "%RUN_DEMO%" || exit /b 1
)

echo Build completed successfully.
exit /b 0

:ensure_cmake
where cmake >nul 2>nul
if not errorlevel 1 exit /b 0

if "%INSTALL_DEPS%"=="1" (
    where winget >nul 2>nul
    if errorlevel 1 (
        echo cmake missing and winget not found. Install CMake manually.
        exit /b 1
    )

    winget install --id Kitware.CMake -e --accept-package-agreements --accept-source-agreements
    where cmake >nul 2>nul
    if not errorlevel 1 exit /b 0
)

echo cmake is required but was not found.
echo Install CMake or rerun with --install-deps.
exit /b 1

:ensure_compiler
where cl >nul 2>nul
if not errorlevel 1 exit /b 0

where g++ >nul 2>nul
if not errorlevel 1 exit /b 0

if "%INSTALL_DEPS%"=="1" (
    where winget >nul 2>nul
    if errorlevel 1 (
        echo No MSVC/G++ compiler found and winget unavailable.
        exit /b 1
    )

    winget install --id Microsoft.VisualStudio.2022.BuildTools -e --accept-package-agreements --accept-source-agreements --override "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools"
    where cl >nul 2>nul
    if not errorlevel 1 exit /b 0
)

echo No supported C++ compiler found.
echo Install Visual Studio Build Tools (C++ workload) or MinGW.
exit /b 1

:run_demo
set DEMO_NAME=%~1
set DEMO_PATH=%BUILD_DIR%\examples\%DEMO_NAME%.exe

if exist "%DEMO_PATH%" (
    "%DEMO_PATH%"
    exit /b %errorlevel%
)

set DEMO_PATH=%BUILD_DIR%\examples\%BUILD_TYPE%\%DEMO_NAME%.exe
if exist "%DEMO_PATH%" (
    "%DEMO_PATH%"
    exit /b %errorlevel%
)

echo Could not find executable for demo: %DEMO_NAME%
exit /b 1

:help
echo Usage: build.bat [options]
echo.
echo Options:
echo   --build-type ^<type^>    Build type (Debug, Release). Default: Release
echo   --run ^<demo^>           Run a demo target after build (demo_* target name)
echo   --install-deps         Attempt to install missing dependencies with winget
echo   --clean                Remove build directory before configuring
echo   --help                 Show this help
echo.
echo Examples:
echo   build.bat
echo   build.bat --build-type Debug
echo   build.bat --run demo_text_editor
echo   build.bat --install-deps --run demo_basic
exit /b 0
