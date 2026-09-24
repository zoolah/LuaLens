@echo off
setlocal EnableExtensions
cd /d "%~dp0"

where cmake >nul 2>&1
if errorlevel 1 (
  echo CMake is required. Install it from https://cmake.org/download/ and re-run build.bat
  exit /b 1
)

echo Configuring...
cmake -S . -B build -A x64
if errorlevel 1 (
  echo Retrying without -A x64...
  cmake -S . -B build
)
if errorlevel 1 exit /b 1

echo Building Release...
cmake --build build --config Release
if errorlevel 1 exit /b 1

echo.
if exist "build\Release\LuaLens.exe" (
  echo Built: build\Release\LuaLens.exe
) else if exist "build\LuaLens.exe" (
  echo Built: build\LuaLens.exe
) else (
  echo Build finished. Look for LuaLens.exe under build\
)
endlocal
