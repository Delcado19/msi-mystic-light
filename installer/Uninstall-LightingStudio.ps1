[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [switch]$RemovePawnIO,
    [switch]$RemoveUserData,
    [switch]$Quiet,
    [string]$OwnerSid,
    [switch]$Elevated
)
# Removes everything Install-LightingStudio.ps1 created. The shared PawnIO driver and the owner's
# editor settings are removed only on request; saved project files are never touched.
$ErrorActionPreference = 'Stop'
$destination = Join-Path ([Environment]::GetFolderPath('ProgramFiles')) 'Lighting Studio'
$state = Join-Path ([Environment]::GetFolderPath('CommonApplicationData')) 'LightingStudio'
$uninstallKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\LightingStudio'
$pawnIOKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PawnIO'
# QSettings organization / application name set in src/main.cpp.
$settingsPath = 'Software\Delcado\MSI Mystic Light Standalone'
$dataPath = 'AppData\Local\Delcado\MSI Mystic Light Standalone'
# Capture the interactive owner before elevation, including credential-based UAC.
if (!$OwnerSid) { $OwnerSid = [Security.Principal.WindowsIdentity]::GetCurrent().User.Value }
if ($OwnerSid -notmatch '^S-1-5-21-\d+-\d+-\d+-\d+$') { throw 'Select a local or domain user SID, not a service account.' }
if (!$PSCmdlet.ShouldProcess($destination, 'Remove Lighting Studio service, editor logon task, program files and service data')) { return }
if (!$Quiet -and !$Elevated) {
    if (!$RemoveUserData) { $RemoveUserData = (Read-Host 'Also delete your editor settings and unsaved recovery data? Saved project files are kept. [y/N]') -match '^(y|j)' }
    if (!$RemovePawnIO -and (Test-Path -LiteralPath $pawnIOKey)) {
        Write-Host 'The PawnIO driver is shared. OpenRGB, Fan Control, LibreHardwareMonitor, ZenTimings and other'
        Write-Host 'hardware tools may need it and stop reading sensors or RAM if it is removed.'
        $RemovePawnIO = (Read-Host 'Also uninstall PawnIO? [y/N]') -match '^(y|j)'
    }
}
$identity = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (!$identity.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    if ($PSCommandPath -match '["\r\n]') { throw 'Invalid uninstaller path' }
    $arguments = @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"' + $PSCommandPath + '"'),'-OwnerSid',$OwnerSid,'-Elevated')
    if ($RemovePawnIO) { $arguments += '-RemovePawnIO' }
    if ($RemoveUserData) { $arguments += '-RemoveUserData' }
    if ($Quiet) { $arguments += '-Quiet' }
    # The elevated window stays visible: it lists every step and anything that could not be removed.
    $child = Start-Process -FilePath "$PSHOME/powershell.exe" -ArgumentList $arguments -Verb RunAs -Wait -PassThru
    if ($child.ExitCode -ne 0) { throw "Removal was incomplete ($($child.ExitCode)); see the elevated window or rerun from an administrator console." }
    return
}
# This script may run from inside the folder it deletes.
Set-Location -LiteralPath $env:SystemRoot
$problems = New-Object Collections.Generic.List[string]
function Invoke-Step([string]$Name, [scriptblock]$Action) {
    try { & $Action; Write-Host "Done: $Name" } catch { $problems.Add("$Name - $($_.Exception.Message)"); Write-Warning "$Name - $($_.Exception.Message)" }
}
# rd does not follow junctions; Remove-Item -Recurse can delete a junction target's contents.
function Remove-Tree([string]$Path) {
    if (!(Test-Path -LiteralPath $Path)) { return }
    & cmd.exe /d /c rd /s /q "$Path" 2>&1 | Out-Null
    if (Test-Path -LiteralPath $Path) { throw "Could not delete $Path (a file may still be in use)" }
}
Invoke-Step 'Close the installed editor' {
    Get-Process msi_mystic_light -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq (Join-Path $destination 'msi_mystic_light.exe') } |
        ForEach-Object { Stop-Process -Id $_.Id -Force; $_.WaitForExit(5000) | Out-Null }
}
Invoke-Step 'Remove the editor logon task' {
    if (Get-ScheduledTask -TaskName 'Lighting Studio Editor' -ErrorAction SilentlyContinue) { Unregister-ScheduledTask -TaskName 'Lighting Studio Editor' -Confirm:$false }
}
Invoke-Step 'Stop and remove the LightingStudio service' {
    $service = Get-Service -Name LightingStudio -ErrorAction SilentlyContinue
    if ($service) {
        if ($service.Status -ne 'Stopped') { Stop-Service LightingStudio -Force; $service.WaitForStatus('Stopped',[TimeSpan]::FromSeconds(30)) }
        & sc.exe delete LightingStudio | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "sc.exe delete failed ($LASTEXITCODE)" }
    }
}
Invoke-Step 'Remove the Start menu shortcut' {
    $shortcut = Join-Path ([Environment]::GetFolderPath('CommonPrograms')) 'Lighting Studio.lnk'
    if (Test-Path -LiteralPath $shortcut) { Remove-Item -LiteralPath $shortcut -Force }
}
# service.json names the owner chosen at installation; read it before the folder is deleted.
Invoke-Step 'Read the installed owner' {
    $config = Join-Path $destination 'service.json'
    if (Test-Path -LiteralPath $config) {
        $installedOwner = (Get-Content -LiteralPath $config -Raw | ConvertFrom-Json).ownerSid
        if ($installedOwner -match '^S-1-5-21-\d+-\d+-\d+-\d+$') { $script:OwnerSid = $installedOwner }
    }
}
Invoke-Step "Delete $destination" { Remove-Tree $destination }
Invoke-Step "Delete $state" { Remove-Tree $state }
Invoke-Step 'Remove the Windows Apps entry' {
    if (Test-Path -LiteralPath $uninstallKey) { Remove-Item -LiteralPath $uninstallKey -Recurse -Force }
}
if ($RemoveUserData) {
    Invoke-Step 'Delete editor settings' {
        $hive = "Registry::HKEY_USERS\$OwnerSid"
        # The hive is loaded only while that user is signed in.
        if (!(Test-Path -LiteralPath $hive)) { throw "The settings of $OwnerSid are not loaded; sign in as that user and delete HKCU\$settingsPath" }
        $key = "$hive\$settingsPath"
        if (Test-Path -LiteralPath $key) { Remove-Item -LiteralPath $key -Recurse -Force }
        $parent = Split-Path $key -Parent
        if ((Test-Path -LiteralPath $parent) -and !(Get-ChildItem -LiteralPath $parent) -and !(Get-Item -LiteralPath $parent).Property) { Remove-Item -LiteralPath $parent -Force }
    }
    Invoke-Step 'Delete editor recovery data' {
        $profilePath = (Get-ItemProperty -LiteralPath "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList\$OwnerSid" -ErrorAction Stop).ProfileImagePath
        $data = Join-Path $profilePath $dataPath
        Remove-Tree $data
        $parent = Split-Path $data -Parent
        if ((Test-Path -LiteralPath $parent) -and !(Get-ChildItem -LiteralPath $parent -Force)) { Remove-Item -LiteralPath $parent -Force }
    }
} else { Write-Host 'Kept: editor settings and recovery data.' }
if ($RemovePawnIO) {
    Invoke-Step 'Uninstall PawnIO' {
        if (!(Test-Path -LiteralPath $pawnIOKey)) { Write-Host 'PawnIO is not installed.'; return }
        # PawnIO registers this command itself as QuietUninstallString: "<InstallLocation>\uninstall.exe" -uninstall -silent
        $location = (Get-ItemProperty -LiteralPath $pawnIOKey).InstallLocation
        $uninstaller = Join-Path $location 'uninstall.exe'
        if (!$location -or !(Test-Path -LiteralPath $uninstaller -PathType Leaf)) { throw 'PawnIO uninstaller not found; remove PawnIO in Windows Settings > Apps' }
        $result = Start-Process -FilePath $uninstaller -ArgumentList @('-uninstall','-silent') -Wait -PassThru -WindowStyle Hidden
        if ($result.ExitCode -ne 0) { throw "PawnIO uninstaller failed ($($result.ExitCode))" }
        if (Test-Path -LiteralPath $pawnIOKey) { throw 'PawnIO is still registered; remove it in Windows Settings > Apps' }
    }
} elseif (Test-Path -LiteralPath $pawnIOKey) { Write-Host 'Kept: PawnIO driver (shared with other hardware tools).' }
if ($problems.Count) {
    Write-Host "`nLighting Studio was NOT removed completely:" -ForegroundColor Red
    $problems | ForEach-Object { Write-Host "  $_" }
} else { Write-Host "`nLighting Studio was removed. Saved project files were not touched." }
if (!$Quiet) { Read-Host 'Press Enter to close' | Out-Null }
if ($problems.Count) { exit 1 }
