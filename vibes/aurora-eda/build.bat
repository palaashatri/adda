@echo off
setlocal enabledelayedexpansion

set "ROOT_DIR=%~dp0"
set "ROOT_DIR=%ROOT_DIR:~0,-1%"
set "BUILD_DIR=%ROOT_DIR%\build"
set "CONFIG=RelWithDebInfo"
set "GENERATOR="
set "BUILD_UI=ON"
set "BUILD_PYTHON=ON"
set "INSTALL_DEPS=0"
set "RUN_APP=0"
set "RUN_TESTS=1"
set "CLEAN=0"

:parse
if "%~1"=="" goto after_parse
if /I "%~1"=="--install-deps" set "INSTALL_DEPS=1" & shift & goto parse
if /I "%~1"=="--run" set "RUN_APP=1" & shift & goto parse
if /I "%~1"=="--no-test" set "RUN_TESTS=0" & shift & goto parse
if /I "%~1"=="--clean" set "CLEAN=1" & shift & goto parse
if /I "%~1"=="--debug" set "CONFIG=Debug" & shift & goto parse
if /I "%~1"=="--release" set "CONFIG=Release" & shift & goto parse
if /I "%~1"=="--config" set "CONFIG=%~2" & shift & shift & goto parse
if /I "%~1"=="--build-dir" set "BUILD_DIR=%~2" & shift & shift & goto parse
if /I "%~1"=="--generator" set "GENERATOR=%~2" & shift & shift & goto parse
if /I "%~1"=="--no-ui" set "BUILD_UI=OFF" & shift & goto parse
if /I "%~1"=="--no-python" set "BUILD_PYTHON=OFF" & shift & goto parse
if /I "%~1"=="-h" goto usage
if /I "%~1"=="--help" goto usage
echo error: unknown option: %~1
goto usage_error

:usage
echo Usage: build.bat [options]
echo.
echo Build aurora-eda and optionally install dependencies, run tests, and launch the app.
echo.
echo Options:
echo   --install-deps       Install common Windows build dependencies.
echo   --run                Run the aurora-eda application after building.
echo   --no-test            Skip ctest after building.
echo   --clean              Remove the build directory before configuring.
echo   --debug              Build Debug configuration.
echo   --release            Build Release configuration.
echo   --config ^<name^>      Build configuration ^(default: RelWithDebInfo^).
echo   --build-dir ^<path^>   Build directory ^(default: .\build^).
echo   --generator ^<name^>   CMake generator, such as Ninja or Visual Studio 17 2022.
echo   --no-ui              Configure without Qt UI support.
echo   --no-python          Configure without pybind11 Python bindings.
echo   -h, --help           Show this help.
echo.
echo Examples:
echo   build.bat --install-deps
echo   build.bat --run
echo   build.bat --config Debug --run
exit /b 0

:usage_error
exit /b 2

:after_parse
if "%INSTALL_DEPS%"=="1" call :install_deps || exit /b 1

if not "%BUILD_DIR:~1,1%"==":" (
  if not "%BUILD_DIR:~0,2%"=="\\" (
    if not "%BUILD_DIR:~0,1%"=="\" (
      set "BUILD_DIR=%ROOT_DIR%\%BUILD_DIR%"
    )
  )
)

where cmake >nul 2>nul
if errorlevel 1 (
  echo error: CMake was not found. Run build.bat --install-deps or install CMake manually.
  exit /b 1
)

set "TOOLCHAIN_FILE="
if defined VCPKG_ROOT (
  if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    set "TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
  )
) else (
  if exist "%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
    set "TOOLCHAIN_FILE=%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake"
  )
)

if "%CLEAN%"=="1" (
  if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

if "%GENERATOR%"=="" (
  if "%TOOLCHAIN_FILE%"=="" (
    cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" ^
      -DCMAKE_BUILD_TYPE="%CONFIG%" ^
      -DAURORA_BUILD_UI="%BUILD_UI%" ^
      -DAURORA_BUILD_PYTHON="%BUILD_PYTHON%" ^
      -DAURORA_BUILD_TESTS=ON
  ) else (
    cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" ^
      -DCMAKE_BUILD_TYPE="%CONFIG%" ^
      -DAURORA_BUILD_UI="%BUILD_UI%" ^
      -DAURORA_BUILD_PYTHON="%BUILD_PYTHON%" ^
      -DAURORA_BUILD_TESTS=ON ^
      "-DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%"
  )
) else (
  if "%TOOLCHAIN_FILE%"=="" (
    cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "%GENERATOR%" ^
      -DCMAKE_BUILD_TYPE="%CONFIG%" ^
      -DAURORA_BUILD_UI="%BUILD_UI%" ^
      -DAURORA_BUILD_PYTHON="%BUILD_PYTHON%" ^
      -DAURORA_BUILD_TESTS=ON
  ) else (
    cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "%GENERATOR%" ^
      -DCMAKE_BUILD_TYPE="%CONFIG%" ^
      -DAURORA_BUILD_UI="%BUILD_UI%" ^
      -DAURORA_BUILD_PYTHON="%BUILD_PYTHON%" ^
      -DAURORA_BUILD_TESTS=ON ^
      "-DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%"
  )
)
if errorlevel 1 exit /b 1

cmake --build "%BUILD_DIR%" --config "%CONFIG%"
if errorlevel 1 exit /b 1

if "%RUN_TESTS%"=="1" (
  ctest --test-dir "%BUILD_DIR%" --build-config "%CONFIG%" --output-on-failure
  if errorlevel 1 exit /b 1
)

if "%RUN_APP%"=="1" call :run_app || exit /b 1

exit /b 0

:install_deps
where winget >nul 2>nul
if not errorlevel 1 (
  winget install --id Kitware.CMake -e --accept-package-agreements --accept-source-agreements
  winget install --id Ninja-build.Ninja -e --accept-package-agreements --accept-source-agreements
  winget install --id Python.Python.3.12 -e --accept-package-agreements --accept-source-agreements
  winget install --id Microsoft.VisualStudio.2022.BuildTools -e --accept-package-agreements --accept-source-agreements --override "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
) else (
  echo warning: winget not found; skipping compiler/CMake installation.
)

call :ensure_msvc
if errorlevel 1 (
  echo info: Attempting to repair Visual Studio Build Tools C++ components...
  call :repair_msvc || exit /b 1
  call :ensure_msvc || exit /b 1
)

if not defined VCPKG_ROOT set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
  where git >nul 2>nul
  if errorlevel 1 (
    echo error: git is required to bootstrap vcpkg. Install Git or set VCPKG_ROOT to an existing vcpkg checkout.
    exit /b 1
  )
  git clone https://github.com/microsoft/vcpkg "%VCPKG_ROOT%"
  if errorlevel 1 exit /b 1
  call "%VCPKG_ROOT%\bootstrap-vcpkg.bat"
  if errorlevel 1 exit /b 1
)

"%VCPKG_ROOT%\vcpkg.exe" install qtbase pybind11 fmt spdlog nlohmann-json --triplet x64-windows
if errorlevel 1 exit /b 1
exit /b 0

:ensure_msvc
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo error: Unable to find vswhere.exe. Please install Visual Studio 2022 Build Tools.
  exit /b 1
)

set "VSINSTALLDIR="
for /f "usebackq tokens=* delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set "VSINSTALLDIR=%%I"
)

if not defined VSINSTALLDIR (
  echo error: Could not find a valid Visual Studio C++ toolchain.
  echo        Re-run with --install-deps, or install Visual Studio 2022 Build Tools with C++ workload.
  exit /b 1
)

if not exist "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvarsall.bat" (
  echo error: Found Visual Studio at "%VSINSTALLDIR%", but vcvarsall.bat is missing.
  echo        Repair the Build Tools installation and try again.
  exit /b 1
)

exit /b 0

:repair_msvc
set "VSSETUP=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\setup.exe"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSINSTALLDIR="

if exist "%VSWHERE%" (
  for /f "usebackq tokens=* delims=" %%I in (`"%VSWHERE%" -latest -products * -property installationPath`) do (
    set "VSINSTALLDIR=%%I"
  )
)

if not defined VSINSTALLDIR set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools"

if exist "%VSSETUP%" (
  echo info: Ensuring no stale Visual Studio Installer process is blocking repair...
  for %%P in (setup.exe vs_BuildTools.exe vs_setup_bootstrapper.exe WinSdkInstaller.exe winsdksetup.exe) do (
    taskkill /IM %%P /F >nul 2>nul
  )
  timeout /t 2 /nobreak >nul

  "%VSSETUP%" modify --installPath "%VSINSTALLDIR%" --quiet --norestart --force --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended
  if errorlevel 1 (
    echo warning: Visual Studio Installer modify command failed. Falling back to winget.
  ) else (
    exit /b 0
  )
)

where winget >nul 2>nul
if errorlevel 1 (
  echo error: Could not auto-repair Visual Studio Build Tools because both setup.exe and winget are unavailable.
  exit /b 1
)

winget install --id Microsoft.VisualStudio.2022.BuildTools -e --force --accept-package-agreements --accept-source-agreements --override "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
if errorlevel 1 (
  echo error: Failed to install Visual Studio Build Tools C++ components.
  exit /b 1
)

exit /b 0

:run_app
set "APP_EXE="
if exist "%BUILD_DIR%\src\ui\%CONFIG%\aurora-eda.exe" set "APP_EXE=%BUILD_DIR%\src\ui\%CONFIG%\aurora-eda.exe"
if "!APP_EXE!"=="" if exist "%BUILD_DIR%\src\ui\aurora-eda.exe" set "APP_EXE=%BUILD_DIR%\src\ui\aurora-eda.exe"
if "!APP_EXE!"=="" (
  echo error: aurora-eda.exe was not found under %BUILD_DIR%
  exit /b 1
)

rem --- deploy Qt DLLs alongside the executable so it can launch ---
set "WDQT="
where windeployqt6.exe >nul 2>nul && set "WDQT=windeployqt6.exe"
if "!WDQT!"=="" where windeployqt.exe >nul 2>nul && set "WDQT=windeployqt.exe"
if "!WDQT!"=="" if defined VCPKG_ROOT (
  if exist "%VCPKG_ROOT%\installed\x64-windows\tools\Qt6\bin\windeployqt6.exe" (
    set "WDQT=%VCPKG_ROOT%\installed\x64-windows\tools\Qt6\bin\windeployqt6.exe"
  )
)
if "!WDQT!"=="" if defined VCPKG_ROOT (
  if exist "%VCPKG_ROOT%\installed\x64-windows\tools\Qt6\windeployqt6.exe" (
    set "WDQT=%VCPKG_ROOT%\installed\x64-windows\tools\Qt6\windeployqt6.exe"
  )
)
if defined WDQT (
  "!WDQT!" --no-translations "!APP_EXE!" >nul 2>&1
) else (
  echo warning: windeployqt not found -- Qt DLLs may be missing. Add Qt bin dir to PATH if the app fails to launch.
)

"!APP_EXE!"
exit /b %errorlevel%
