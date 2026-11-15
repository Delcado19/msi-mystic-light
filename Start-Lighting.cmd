@echo off
setlocal
rem Development builds: start the most recently built of r4, routing and aurora, so a stale
rem build folder never hides a newer one (CodeRabbit review 2026-11-15).
rem Paths reach PowerShell as environment data, never as source text (apostrophes in the checkout path).
set "APP="
set "LS_R4=%~dp0build\r4\Release\msi_mystic_light.exe"
set "LS_ROUTING=%~dp0build\routing\Release\msi_mystic_light.exe"
set "LS_AURORA=%~dp0build\aurora\Release\msi_mystic_light.exe"
for /f "usebackq delims=" %%A in (`powershell -NoProfile -Command "Get-ChildItem -LiteralPath $env:LS_R4,$env:LS_ROUTING,$env:LS_AURORA -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1 -ExpandProperty FullName"`) do set "APP=%%A"
if defined APP goto launch
set "APP=%~dp0build\dist\LightingStudio-ReliabilityFix\msi_mystic_light.exe"
if exist "%ProgramFiles%\Lighting Studio\service.json" (
  set "APP=%ProgramFiles%\Lighting Studio\msi_mystic_light.exe"
  goto launch
)
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-IceRefined-StartupFix\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Ice\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Stealth-Hotfix\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Stealth\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-GamingStudio\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-EditorControls\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Fullscreen\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Service\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Unified\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Hotfix\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Editor\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio-Audit\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\dist\LightingStudio\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\next\Release\msi_mystic_light.exe"
if not exist "%APP%" set "APP=%~dp0build\ninja\Release\msi_mystic_light.exe"
:launch
if not exist "%APP%" (
  echo Build the Release application first. See README.md.
  pause
  exit /b 1
)
start "" "%APP%" %*
