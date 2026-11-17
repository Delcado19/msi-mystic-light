@echo off
setlocal
rem Development launcher for a source checkout: start the local Release build in build\r10; without
rem one, start the installed editor. A downloaded package needs no launcher: run msi_mystic_light.exe.
set "APP=%~dp0build\r10\Release\msi_mystic_light.exe"
if exist "%APP%" goto launch
set "APP=%ProgramFiles%\Lighting Studio\msi_mystic_light.exe"
if exist "%APP%" goto launch
echo Build the Release application first (build\build-r10.cmd). See README.md.
pause
exit /b 1
:launch
start "" "%APP%" %*
