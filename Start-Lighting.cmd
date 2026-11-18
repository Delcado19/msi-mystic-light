@echo off
setlocal
rem Development launcher: start the most recently built of build\r4, routing and aurora, so a stale
rem build folder never hides a newer one; without a local build, start the installed editor.
rem PowerShell reads the paths as environment data and prints only the ASCII key (R4/ROUTING/AURORA),
rem so neither apostrophes nor console code page translation can alter the path (CodeRabbit review 2026-11-15).
set "APP="
set "PICK="
set "LS_R4=%~dp0build\r4\Release\msi_mystic_light.exe"
set "LS_ROUTING=%~dp0build\routing\Release\msi_mystic_light.exe"
set "LS_AURORA=%~dp0build\aurora\Release\msi_mystic_light.exe"
for /f "usebackq delims=" %%A in (`powershell -NoProfile -Command "'R4','ROUTING','AURORA' | ForEach-Object { $f = Get-Item -LiteralPath ([Environment]::GetEnvironmentVariable('LS_' + $_)) -ErrorAction SilentlyContinue; if ($f) { [pscustomobject]@{ Key = $_; Time = $f.LastWriteTimeUtc } } } | Sort-Object Time -Descending | Select-Object -First 1 -ExpandProperty Key"`) do set "PICK=%%A"
if "%PICK%"=="R4" set "APP=%LS_R4%"
if "%PICK%"=="ROUTING" set "APP=%LS_ROUTING%"
if "%PICK%"=="AURORA" set "APP=%LS_AURORA%"
if not defined APP if exist "%ProgramFiles%\Lighting Studio\service.json" set "APP=%ProgramFiles%\Lighting Studio\msi_mystic_light.exe"
if not defined APP goto missing
if exist "%APP%" goto launch
:missing
echo Build the Release application first. See README.md.
pause
exit /b 1
:launch
start "" "%APP%" %*
