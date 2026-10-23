@echo off
setlocal
set "APP=%~dp0build\ninja\Release\msi_mystic_light.exe"
if not exist "%APP%" (
  echo Build the Release application first. See README.md.
  pause
  exit /b 1
)
start "" "%APP%" %*
