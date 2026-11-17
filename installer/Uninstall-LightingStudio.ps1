[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [switch]$RemovePawnIO,
    [switch]$RemoveUserData,
    [switch]$Quiet,
    [switch]$Elevated
)
# Removes everything Install-LightingStudio.ps1 created. The shared PawnIO driver and the owner's
# editor settings are removed only on request; saved project files are never touched.
$ErrorActionPreference = 'Stop'
$destination = Join-Path ([Environment]::GetFolderPath('ProgramFiles')) 'Lighting Studio'
$state = Join-Path ([Environment]::GetFolderPath('CommonApplicationData')) 'LightingStudio'
$uninstallKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\LightingStudio'
$pawnIOKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PawnIO'
# rd does not follow junctions inside the tree; Remove-Item -Recurse can delete a junction target's contents.
function Remove-Tree([string]$Path) {
    if (!(Test-Path -LiteralPath $Path)) { return }
    & cmd.exe /d /c rd /s /q "$Path" 2>&1 | Out-Null
    if (Test-Path -LiteralPath $Path) { throw "Could not delete $Path (a file may still be in use)" }
}
# User data lives in a user-writable profile, so it is deleted by the process the user started, with
# that user's own token, never by the elevated child (security review 2026-11-15: a junction planted in
# the profile must not redirect an administrator's delete). A console that is already elevated has no
# such process; there both folders must be real directories or nothing is deleted.
# Organization / application name: QSettings in src/main.cpp.
function Remove-UserData {
    $failed = $false
    try {
        $key = 'HKCU:\Software\Delcado\MSI Mystic Light Standalone'
        if (Test-Path -LiteralPath $key) { Remove-Item -LiteralPath $key -Recurse -Force }
        $parentKey = Split-Path $key -Parent
        if ((Test-Path -LiteralPath $parentKey) -and !(Get-ChildItem -LiteralPath $parentKey) -and !(Get-Item -LiteralPath $parentKey).Property) { Remove-Item -LiteralPath $parentKey -Force }
        $parent = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'Delcado'
        $data = Join-Path $parent 'MSI Mystic Light Standalone'
        foreach ($folder in @($parent,$data)) {
            if ((Test-Path -LiteralPath $folder) -and ((Get-Item -LiteralPath $folder -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)) { throw "Not deleted, $folder is a link: remove it yourself" }
        }
        Remove-Tree $data
        if ((Test-Path -LiteralPath $parent) -and !(Get-ChildItem -LiteralPath $parent -Force)) { Remove-Item -LiteralPath $parent -Force }
        Write-Host 'Done: Delete editor settings and recovery data'
    } catch { $failed = $true; Write-Warning "Editor settings and recovery data - $($_.Exception.Message)" }
    return !$failed
}
if (!$PSCmdlet.ShouldProcess($destination, 'Remove Lighting Studio service, editor logon task, program files and service data')) { return }
if (!$Quiet -and !$Elevated) {
    if (!$RemoveUserData) {
        # The one-line question was misread by the owner (2026-11-17); name each item and where it lives.
        Write-Host 'Your personal editor data can be deleted too:'
        Write-Host '  - window layout and editor preferences (registry: HKCU\Software\Delcado)'
        Write-Host '  - the automatic backup of changes you never saved (%LOCALAPPDATA%\Delcado)'
        Write-Host 'Projects you saved yourself (*.lighting.json) are never deleted.'
        $RemoveUserData = (Read-Host 'Delete this personal editor data? [y/N]') -match '^(y|j)'
    }
    if (!$RemovePawnIO -and (Test-Path -LiteralPath $pawnIOKey)) {
        Write-Host 'The PawnIO driver is shared. OpenRGB, Fan Control, LibreHardwareMonitor, ZenTimings and other'
        Write-Host 'hardware tools may need it and stop reading sensors or RAM if it is removed.'
        $RemovePawnIO = (Read-Host 'Also uninstall PawnIO? [y/N]') -match '^(y|j)'
    }
}
# This script may run from inside the folder it deletes; no process may keep that folder as its working directory.
Set-Location -LiteralPath $env:SystemRoot
$identity = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (!$identity.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    if ($PSCommandPath -match '["\r\n]') { throw 'Invalid uninstaller path' }
    $arguments = @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"' + $PSCommandPath + '"'),'-Elevated')
    if ($RemovePawnIO) { $arguments += '-RemovePawnIO' }
    if ($Quiet) { $arguments += '-Quiet' }
    # The elevated window stays visible: it lists every step and anything that could not be removed.
    if (!$Quiet) { Write-Host 'Removing in the administrator window that opens now. Confirm the Windows prompt (UAC).' }
    $child = Start-Process -FilePath "$PSHOME/powershell.exe" -ArgumentList $arguments -Verb RunAs -Wait -PassThru
    # Judge the result by what is left, not by the exit code: closing the elevated window with X
    # reported 0xC000013A although everything had been removed, and the user data step was skipped
    # (owner report 2026-11-17).
    $left = @()
    if (Get-Service -Name LightingStudio -ErrorAction SilentlyContinue) { $left += 'the LightingStudio service' }
    if (Test-Path -LiteralPath $destination) { $left += $destination }
    if (Test-Path -LiteralPath $uninstallKey) { $left += 'the Windows Apps entry' }
    if ($left.Count) {
        Write-Host "`nLighting Studio was NOT removed completely (exit code $($child.ExitCode)). Still present: $($left -join ', ')." -ForegroundColor Red
        Write-Host 'Your editor data was kept. Run the uninstaller again or check the administrator window.'
        if (!$Quiet) { Read-Host 'Press Enter to close' | Out-Null }
        exit 1
    }
    # Only after the program is gone, so a declined UAC prompt leaves the settings in place.
    if ($RemoveUserData -and !(Remove-UserData)) { if (!$Quiet) { Read-Host 'Press Enter to close' | Out-Null }; exit 1 }
    if (!$Quiet) {
        Write-Host "`nLighting Studio was removed. Saved project files were not touched." -ForegroundColor Green
        Read-Host 'Press Enter to close' | Out-Null
    }
    return
}
$problems = New-Object Collections.Generic.List[string]
$self = [IO.File]::ReadAllText($PSCommandPath)
$clock = [Diagnostics.Stopwatch]::StartNew()
$stepCount = 7 + [int][bool]$RemovePawnIO + [int]($RemoveUserData -and !$Elevated); $stepIndex = 0
# Progress bar plus one timed line per step, so a slow step does not look like a hang (2026-11-17).
function Invoke-Step([string]$Name, [scriptblock]$Action) {
    $script:stepIndex++
    Write-Progress -Activity 'Removing Lighting Studio' -Status "Step $script:stepIndex of ${stepCount}: $Name" -PercentComplete ([Math]::Min(100, 100 * ($script:stepIndex - 1) / $stepCount))
    try { & $Action; Write-Host ('[{0:mm\:ss}] Done: {1}' -f $clock.Elapsed, $Name) } catch { $problems.Add("$Name - $($_.Exception.Message)"); Write-Warning "$Name - $($_.Exception.Message)" }
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
Invoke-Step "Delete $destination" { Remove-Tree $destination }
Invoke-Step "Delete $state" { Remove-Tree $state }
# A locked file can stop the folder removal after this script itself was deleted. The Apps entry then
# stays and the script is put back, so the removal can be retried from Settings > Apps (CodeRabbit review 2026-11-15).
if (Test-Path -LiteralPath $destination) {
    Invoke-Step 'Keep the uninstaller for a retry' { [IO.File]::WriteAllText((Join-Path $destination 'Uninstall-LightingStudio.ps1'), $self) }
} else {
    Invoke-Step 'Remove the Windows Apps entry' {
        if (Test-Path -LiteralPath $uninstallKey) { Remove-Item -LiteralPath $uninstallKey -Recurse -Force }
    }
}
# The elevated child never touches the profile; its unelevated parent deletes user data afterwards.
if ($RemoveUserData -and !$Elevated) { Invoke-Step 'Delete editor settings and recovery data' { if (!(Remove-UserData)) { throw 'not deleted completely' } } }
elseif (!$RemoveUserData -and !$Elevated) { Write-Host 'Kept: editor settings and recovery data.' }
if ($RemovePawnIO) {
    Invoke-Step 'Uninstall PawnIO' {
        if (!(Test-Path -LiteralPath $pawnIOKey)) { Write-Host 'PawnIO is not installed.'; return }
        # PawnIO registers this command itself as QuietUninstallString: "<InstallLocation>\uninstall.exe" -uninstall -silent
        $location = (Get-ItemProperty -LiteralPath $pawnIOKey).InstallLocation
        $uninstaller = Join-Path $location 'uninstall.exe'
        if (!$location -or !(Test-Path -LiteralPath $uninstaller -PathType Leaf)) { throw 'PawnIO uninstaller not found; remove PawnIO in Windows Settings > Apps' }
        $result = Start-Process -FilePath $uninstaller -ArgumentList @('-uninstall','-silent') -PassThru -WindowStyle Hidden
        $null = $result.Handle  # Cache the handle; otherwise ExitCode stays empty after the process ends.
        $started = Get-Date
        while (!$result.HasExited) {
            Write-Progress -Activity 'Removing Lighting Studio' -Status "Step $script:stepIndex of ${stepCount}: Uninstall PawnIO ($([int]((Get-Date) - $started).TotalSeconds) s)" -PercentComplete ([Math]::Min(100, 100 * ($script:stepIndex - 1) / $stepCount))
            Start-Sleep -Milliseconds 500
        }
        if ($result.ExitCode -ne 0) { throw "PawnIO uninstaller failed ($($result.ExitCode))" }
        if (Test-Path -LiteralPath $pawnIOKey) { throw 'PawnIO is still registered; remove it in Windows Settings > Apps' }
        # Observed 2026-11-17: the Apps entry was gone but the driver service stayed registered.
        if (Get-Service -Name PawnIO -ErrorAction SilentlyContinue) { Write-Host '        Note: the PawnIO driver service is still registered; Windows may remove it after a restart.' }
    }
} elseif (Test-Path -LiteralPath $pawnIOKey) { Write-Host 'Kept: PawnIO driver (shared with other hardware tools).' }
Write-Progress -Activity 'Removing Lighting Studio' -Completed
if ($problems.Count) {
    Write-Host "`nLighting Studio was NOT removed completely:" -ForegroundColor Red
    $problems | ForEach-Object { Write-Host "  $_" }
} else { Write-Host "`nLighting Studio was removed. Saved project files were not touched." -ForegroundColor Green }
# The elevated child no longer waits for Enter on success: the visible parent reports the result.
if (!$Quiet -and ($problems.Count -or !$Elevated)) { Read-Host 'Press Enter to close' | Out-Null }
elseif ($Elevated -and !$Quiet) { Write-Host 'This window closes in 5 seconds.'; Start-Sleep -Seconds 5 }
if ($problems.Count) { exit 1 }
