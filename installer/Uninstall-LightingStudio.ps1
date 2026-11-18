[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [switch]$RemovePawnIO,
    [switch]$RemoveUserData,
    [switch]$Quiet,
    [switch]$Elevated
)
# Removes everything Install-LightingStudio.ps1 created. The shared PawnIO driver and the owner's
# editor settings are removed only on request; files the user created are moved to a backup, never deleted.
$ErrorActionPreference = 'Stop'
$destination = Join-Path ([Environment]::GetFolderPath('ProgramFiles')) 'Lighting Studio'
$state = Join-Path ([Environment]::GetFolderPath('CommonApplicationData')) 'LightingStudio'
$uninstallKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\LightingStudio'
$pawnIOKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PawnIO'
# Links inside the tree are removed, never followed: Remove-Item -Recurse can delete a junction target's
# contents. No cmd.exe either, which expands %NAME% inside a path (CodeRabbit review 2026-11-18).
function Remove-TreeItem([IO.FileSystemInfo]$Item) {
    if ($Item -is [IO.DirectoryInfo] -and !($Item.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        foreach ($child in $Item.GetFileSystemInfos()) { Remove-TreeItem $child }
    }
    if ($Item.Attributes -band [IO.FileAttributes]::ReadOnly) { $Item.Attributes = $Item.Attributes -bxor [IO.FileAttributes]::ReadOnly }
    $Item.Delete()
}
function Remove-Tree([string]$Path) {
    if (!(Test-Path -LiteralPath $Path)) { return }
    try { Remove-TreeItem (Get-Item -LiteralPath $Path -Force) }
    catch { throw "Could not delete $Path (a file may still be in use): $($_.Exception.Message)" }
}
$editor = Join-Path $destination 'msi_mystic_light.exe'
function Get-Editor { Get-Process msi_mystic_light -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $editor } }
# User data lives in a user-writable profile, so it is deleted by the process the user started, with
# that user's own token, never by the elevated child (security review 2026-11-15: a junction planted in
# the profile must not redirect an administrator's delete). A console that is already elevated has no
# such process; there both folders must be real directories or nothing is deleted.
# Organization / application name: QSettings in src/main.cpp.
# The app itself writes only application.lock and recovery.lighting.json there. Everything else, and the
# unsaved changes, is moved to a dated backup in the profile instead of being deleted (owner decision
# 2026-11-18; CodeRabbit review 2026-11-18: a project saved into this folder was deleted with it).
$script:backup = $null
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
        if (Test-Path -LiteralPath $data) {
            $lock = Join-Path $data 'application.lock'
            if (Test-Path -LiteralPath $lock) { Remove-Item -LiteralPath $lock -Force }
            $recovery = Join-Path $data 'recovery.lighting.json'
            if ((Test-Path -LiteralPath $recovery) -and !(Test-Path -LiteralPath (Join-Path $data 'Unsaved changes.lighting.json'))) {
                Rename-Item -LiteralPath $recovery -NewName 'Unsaved changes.lighting.json'
            }
            if (Get-ChildItem -LiteralPath $data -Force) {
                $base = Join-Path ([Environment]::GetFolderPath('UserProfile')) ('Lighting Studio - Backup - ' + (Get-Date -Format 'yyyy_MM_dd'))
                $target = $base; $number = 2
                while (Test-Path -LiteralPath $target) { $target = "$base ($number)"; $number++ }
                # A move across volumes fails and leaves every file where it was; nothing is deleted then.
                Move-Item -LiteralPath $data -Destination $target
                $script:backup = $target
            } else { Remove-Item -LiteralPath $data -Force }
        }
        if ((Test-Path -LiteralPath $parent) -and !(Get-ChildItem -LiteralPath $parent -Force)) { Remove-Item -LiteralPath $parent -Force }
        Write-Host 'Done: Delete editor settings'
        if ($script:backup) { Write-Host "Your own files and unsaved changes were moved to: $($script:backup)" -ForegroundColor Yellow }
    } catch { $failed = $true; Write-Warning "Editor settings and user files - $($_.Exception.Message)" }
    return !$failed
}
if (!$PSCmdlet.ShouldProcess($destination, 'Remove Lighting Studio service, editor logon task, program files and service data')) { return }
if (!$Quiet -and !$Elevated) {
    if (!$RemoveUserData) { $RemoveUserData = (Read-Host 'Also delete your editor settings? Your own files and unsaved changes are moved to a backup folder in your profile. [y/N]') -match '^(y|j)' }
    if (!$RemovePawnIO -and (Test-Path -LiteralPath $pawnIOKey)) {
        Write-Host 'The PawnIO driver is shared. OpenRGB, Fan Control, LibreHardwareMonitor, ZenTimings and other'
        Write-Host 'hardware tools may need it and stop reading sensors or RAM if it is removed.'
        $RemovePawnIO = (Read-Host 'Also uninstall PawnIO? [y/N]') -match '^(y|j)'
    }
}
# Killing the editor discarded edits not yet in its recovery file (CodeRabbit review 2026-11-18): the
# user closes it, so its own Save/Discard prompt runs. Removal never starts while it is open.
if (!$Elevated) {
    while (Get-Editor) {
        if ($Quiet) { throw 'Lighting Studio is running. Close it (also from the tray), then retry.' }
        Write-Host 'Lighting Studio is running. Close it (also from the tray) so unsaved changes are not lost.' -ForegroundColor Yellow
        Read-Host 'Press Enter once it is closed' | Out-Null
    }
}
# This script may run from inside the folder it deletes; no process may keep that folder as its working
# directory. Set-Location alone moves only the runspace, not the process (CodeRabbit review 2026-11-18).
Set-Location -LiteralPath $env:SystemRoot
[Environment]::CurrentDirectory = $env:SystemRoot
$identity = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (!$identity.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    if ($PSCommandPath -match '["\r\n]') { throw 'Invalid uninstaller path' }
    $arguments = @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"' + $PSCommandPath + '"'),'-Elevated')
    if ($RemovePawnIO) { $arguments += '-RemovePawnIO' }
    if ($Quiet) { $arguments += '-Quiet' }
    # The elevated window stays visible: it lists every step and anything that could not be removed.
    $child = Start-Process -FilePath "$PSHOME/powershell.exe" -ArgumentList $arguments -Verb RunAs -Wait -PassThru
    if ($child.ExitCode -ne 0) { throw "Removal was incomplete ($($child.ExitCode)); see the elevated window or rerun from an administrator console." }
    # Only after the program is gone, so a declined UAC prompt leaves the settings in place.
    if ($RemoveUserData -and !(Remove-UserData)) { if (!$Quiet) { Read-Host 'Press Enter to close' | Out-Null }; exit 1 }
    return
}
$problems = New-Object Collections.Generic.List[string]
$self = [IO.File]::ReadAllText($PSCommandPath)
function Invoke-Step([string]$Name, [scriptblock]$Action) {
    try { & $Action; Write-Host "Done: $Name" } catch { $problems.Add("$Name - $($_.Exception.Message)"); Write-Warning "$Name - $($_.Exception.Message)" }
}
if (Get-Editor) {
    Write-Host 'Lighting Studio is running. Close it (also from the tray), then retry. Nothing was removed.' -ForegroundColor Red
    if (!$Quiet) { Read-Host 'Press Enter to close' | Out-Null }
    exit 1
}
Invoke-Step 'Remove the editor logon task' {
    if (Get-ScheduledTask -TaskName 'Lighting Studio Editor' -ErrorAction SilentlyContinue) { Unregister-ScheduledTask -TaskName 'Lighting Studio Editor' -Confirm:$false }
}
$script:serviceGone = $false
Invoke-Step 'Stop and remove the LightingStudio service' {
    $service = Get-Service -Name LightingStudio -ErrorAction SilentlyContinue
    if ($service) {
        if ($service.Status -ne 'Stopped') { Stop-Service LightingStudio -Force; $service.WaitForStatus('Stopped',[TimeSpan]::FromSeconds(30)) }
        & sc.exe delete LightingStudio | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "sc.exe delete failed ($LASTEXITCODE)" }
    }
    $script:serviceGone = $true
}
Invoke-Step 'Remove the Start menu shortcut' {
    $shortcut = Join-Path ([Environment]::GetFolderPath('CommonPrograms')) 'Lighting Studio.lnk'
    if (Test-Path -LiteralPath $shortcut) { Remove-Item -LiteralPath $shortcut -Force }
}
# A service still registered keeps its program and state: deleting them left an automatic service
# without an executable (CodeRabbit review 2026-11-18).
if ($script:serviceGone) {
    Invoke-Step "Delete $destination" { Remove-Tree $destination }
    Invoke-Step "Delete $state" { Remove-Tree $state }
} else { $problems.Add('Program files and service data were kept because the service is still registered') }
# After any failure the Apps entry stays and the script is put back, recreating its folder if needed,
# so the removal can be retried from Settings > Apps (CodeRabbit reviews 2026-11-15 and 2026-11-18).
if ($problems.Count) {
    Invoke-Step 'Keep the uninstaller for a retry' {
        New-Item -ItemType Directory -Force -Path $destination | Out-Null
        [IO.File]::WriteAllText((Join-Path $destination 'Uninstall-LightingStudio.ps1'), $self)
    }
} else {
    Invoke-Step 'Remove the Windows Apps entry' {
        if (Test-Path -LiteralPath $uninstallKey) { Remove-Item -LiteralPath $uninstallKey -Recurse -Force }
    }
}
# The elevated child never touches the profile; its unelevated parent deletes user data afterwards.
if ($RemoveUserData -and !$Elevated) { if (!(Remove-UserData)) { $problems.Add('Editor settings and user files were not removed completely') } }
elseif (!$RemoveUserData -and !$Elevated) { Write-Host 'Kept: editor settings and unsaved changes.' }
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
