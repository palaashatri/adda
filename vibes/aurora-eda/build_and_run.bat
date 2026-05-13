@echo off
setlocal enabledelayedexpansion

set "ROOT_DIR=%~dp0"
set "ROOT_DIR=%ROOT_DIR:~0,-1%"
set "BUILD_DIR=%ROOT_DIR%\build"
set "CONFIG=RelWithDebInfo"

echo === aurora-eda: Configure ^& Build ^& Run ===
echo.

rem --- Locate vcpkg toolchain ---
set "TOOLCHAIN_FILE="
if defined VCPKG_ROOT (
  if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    set "TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
  )
) else (
  if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_ROOT=C:\vcpkg"
    set "TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
  ) else if exist "%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
    set "TOOLCHAIN_FILE=%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake"
  )
)

if not defined TOOLCHAIN_FILE (
  echo error: vcpkg toolchain not found. Run build.bat --install-deps first.
  exit /b 1
)

rem --- Locate Qt (aqtinstall layout) ---
set "QT_PREFIX="
for /d %%V in ("C:\Qt\6.*") do (
  if "!QT_PREFIX!"=="" if exist "%%V\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake" (
    set "QT_PREFIX=%%V\msvc2022_64"
  )
)

rem --- Configure ---
echo [1/4] Configuring with CMake...
set "CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%CONFIG%"
set "CMAKE_ARGS=!CMAKE_ARGS! -DAURORA_BUILD_UI=ON"
set "CMAKE_ARGS=!CMAKE_ARGS! -DAURORA_BUILD_PYTHON=ON"
set "CMAKE_ARGS=!CMAKE_ARGS! -DAURORA_BUILD_TESTS=ON"
set "CMAKE_ARGS=!CMAKE_ARGS! -DCMAKE_TOOLCHAIN_FILE=!TOOLCHAIN_FILE!"
set "CMAKE_ARGS=!CMAKE_ARGS! -DVCPKG_TARGET_TRIPLET=x64-windows-release"
if defined QT_PREFIX set "CMAKE_ARGS=!CMAKE_ARGS! -DCMAKE_PREFIX_PATH=!QT_PREFIX!"

cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" !CMAKE_ARGS!
if errorlevel 1 echo error: CMake configuration failed. && exit /b 1
echo.

rem --- Build ---
echo [2/4] Building...
cmake --build "%BUILD_DIR%" --config "%CONFIG%"
if errorlevel 1 echo error: Build failed. && exit /b 1
echo.

rem --- Test ---
echo [3/4] Running tests...
ctest --test-dir "%BUILD_DIR%" --build-config "%CONFIG%" --output-on-failure
if errorlevel 1 echo warning: Some tests failed. && echo.
echo.

rem --- Run ---
echo [4/4] Launching aurora-eda...
set "APP_EXE=%BUILD_DIR%\src\ui\%CONFIG%\aurora-eda.exe"
if not exist "!APP_EXE!" set "APP_EXE=%BUILD_DIR%\src\ui\aurora-eda.exe"
if not exist "!APP_EXE!" (
  echo error: aurora-eda.exe not found. Build may have failed.
  exit /b 1
)

rem --- Deploy Qt DLLs ---
set "WDQT="
where windeployqt6.exe >nul 2>nul && set "WDQT=windeployqt6.exe"
if "!WDQT!"=="" where windeployqt.exe >nul 2>nul && set "WDQT=windeployqt.exe"
if "!WDQT!"=="" (
  for /d %%V in ("C:\Qt\6.*") do (
    if "!WDQT!"=="" if exist "%%V\msvc2022_64\bin\windeployqt6.exe" (
      set "WDQT=%%V\msvc2022_64\bin\windeployqt6.exe"
    )
  )
)
if defined WDQT (
  "!WDQT!" --no-translations "!APP_EXE!" >nul 2>&1
)

"!APP_EXE!"
exit /b %errorlevel%
