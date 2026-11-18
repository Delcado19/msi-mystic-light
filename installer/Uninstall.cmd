@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Uninstall-LightingStudio.ps1" %*
if errorlevel 1 pause
