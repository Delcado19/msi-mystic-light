@echo off
setlocal
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
