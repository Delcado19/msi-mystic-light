[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [switch]$RemovePawnIO,
    [switch]$Quiet,
    [switch]$Elevated,
    # The elevated child writes 'removed' or its problems here, so the parent can tell a recorded
    # failure from an interrupted child (CodeRabbit review 2026-11-17).
    [string]$ResultFile
)
# Removes everything Install-LightingStudio.ps1 created, including the editor settings. The shared PawnIO
# driver is the only thing asked about. Editor files are moved to a backup, never deleted (owner decision
# 2026-11-19: no question about personal data, but a backup path the user cannot overlook).
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
# Any copy counts, portable ones too: they write their recovery file into the same folder that is backed
# up and deleted (CodeRabbit review 2026-11-19).
function Get-Editor { Get-Process msi_mystic_light -ErrorAction SilentlyContinue }
# User data lives in a user-writable profile, so it is handled by the process the user started, with
# that user's own token, never by the elevated child (security review 2026-11-15: a junction planted in
# the profile must not redirect an administrator's delete). A console that is already elevated has no
# such process, so it leaves user data alone: copying with an administrator token could read a protected
# file through a junction swapped in after the scan (CodeRabbit review 2026-11-19, CWE-367).
# Organization / application name: QSettings in src/main.cpp.
# Owner decision 2026-11-19: every *.json anywhere below %LOCALAPPDATA%\Lighting Studio is copied, unrenamed
# and with its subfolders, to a dated backup in the profile; then the whole folder is deleted, other files
# included (the editor writes nothing there but application.lock and recovery.lighting.json).
$script:backup = $null
$script:scanned = $null
# Collects files without entering links: a planted junction must not pull outside files into the backup.
function Get-JsonFiles([IO.DirectoryInfo]$Folder) {
    foreach ($item in $Folder.GetFileSystemInfos()) {
        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { continue }
        if ($item -is [IO.DirectoryInfo]) { Get-JsonFiles $item }
        elseif ($item.Extension -eq '.json') { $item }
    }
}
# Parameters exist for tests/service/test_installer.ps1, which runs this on a temporary folder and key.
function Remove-UserData(
    [string]$Parent = (Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'Lighting Studio'),
    [string]$BackupRoot = [Environment]::GetFolderPath('UserProfile'),
    [string]$Key = 'HKCU:\Software\Lighting Studio\Editor') {
    $failed = $false
    $script:scanned = $Parent
    try {
        if (Test-Path -LiteralPath $Parent) {
            $root = Get-Item -LiteralPath $Parent -Force
            if ($root.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Not deleted, $Parent is a link: remove it yourself" }
            $files = @(Get-JsonFiles $root)
            # Every copy must succeed before anything is deleted; a failed copy leaves the folder and the
            # settings exactly as they were (CodeRabbit review 2026-11-18).
            if ($files.Count) {
                $base = Join-Path $BackupRoot ('Lighting Studio - Backup - ' + (Get-Date -Format 'yyyy_MM_dd'))
                $target = $base; $number = 2
                while (Test-Path -LiteralPath $target) { $target = "$base ($number)"; $number++ }
                foreach ($file in $files) {
                    $copy = Join-Path $target $file.FullName.Substring($root.FullName.TrimEnd('\').Length + 1)
                    New-Item -ItemType Directory -Force -Path (Split-Path $copy -Parent) | Out-Null
                    Copy-Item -LiteralPath $file.FullName -Destination $copy
                }
                $script:backup = $target
            }
            Remove-Tree $Parent
        }
        if (Test-Path -LiteralPath $Key) { Remove-Item -LiteralPath $Key -Recurse -Force }
        $parentKey = Split-Path $Key -Parent
        if ((Test-Path -LiteralPath $parentKey) -and !(Get-ChildItem -LiteralPath $parentKey) -and !(Get-Item -LiteralPath $parentKey).Property) { Remove-Item -LiteralPath $parentKey -Force }
        Write-Host 'Done: Delete editor settings'
    } catch { $failed = $true; Write-Warning "Editor settings and user files - $($_.Exception.Message)" }
    return !$failed
}
# Shown last, right before the window waits for Enter, so the result is the final thing on screen. Also
# shown when no file was found: silence there confused the owner (2026-11-19).
# -OnlyCopied: after a failure, show a finished backup but never claim that no file was found.
function Show-Backup([switch]$OnlyCopied) {
    if ($Elevated) { return }  # the elevated child never touches user data; its parent reports it
    if ($OnlyCopied -and !$script:backup -and !$script:userDataSkipped) { return }
    $body = if ($script:userDataSkipped) { @('  YOUR EDITOR FILES WERE KEPT UNCHANGED (administrator console, no backup made):', '', "  $(Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'Lighting Studio')\") }
            elseif ($script:backup) { @('  YOUR LIGHTING STUDIO FILES WERE BACKED UP TO:', '', "  $($script:backup)") }
            else { @("  NO PERSONAL LIGHTING STUDIO FILES FOUND IN $($script:scanned.TrimEnd('\'))\") }
    # Long paths overflowed the fixed 78-column box (owner test 2026-11-19): size it to the longest line.
    $width = [Math]::Max(78, ($body | ForEach-Object Length | Measure-Object -Maximum).Maximum + 2)
    $line = '=' * $width
    foreach ($text in @('', $line) + $body + @($line, '')) {
        Write-Host $text.PadRight($width) -ForegroundColor Black -BackgroundColor Yellow
    }
}
if (!$PSCmdlet.ShouldProcess($destination, 'Remove Lighting Studio service, editor logon task, program files and service data')) { return }
if (!$Quiet -and !$Elevated) {
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
        if ($Quiet) { throw 'Lighting Studio is running. Quit it from its tray icon (right-click > Quit), then retry.' }
        Write-Host 'Lighting Studio is running. Quit it from its tray icon (right-click > Quit) so unsaved changes are not lost.' -ForegroundColor Yellow
        Read-Host 'Press Enter once it is closed' | Out-Null
    }
}
# This script may run from inside the folder it deletes; no process may keep that folder as its working directory.
Set-Location -LiteralPath $env:SystemRoot
# Set-Location moves only the runspace; the process working directory would still lock the folder
# (CodeRabbit review 2026-11-16).
[Environment]::CurrentDirectory = $env:SystemRoot
$identity = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (!$identity.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    if ($PSCommandPath -match '["\r\n]') { throw 'Invalid uninstaller path' }
    $arguments = @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"' + $PSCommandPath + '"'),'-Elevated')
    if ($RemovePawnIO) { $arguments += '-RemovePawnIO' }
    if ($Quiet) { $arguments += '-Quiet' }
    $resultFile = Join-Path ([IO.Path]::GetTempPath()) ('LightingStudio-uninstall-' + [guid]::NewGuid().ToString('N') + '.result')
    if ($resultFile -match '["\r\n]') { throw 'Invalid result path' }
    $arguments += @('-ResultFile',('"' + $resultFile + '"'))
    # The elevated window stays visible: it lists every step and anything that could not be removed.
    if (!$Quiet) { Write-Host 'Removing in the administrator window that opens now. Confirm the Windows prompt (UAC).' }
    $child = Start-Process -FilePath "$PSHOME/powershell.exe" -ArgumentList $arguments -Verb RunAs -Wait -PassThru -WorkingDirectory $env:SystemRoot
    $reported = if (Test-Path -LiteralPath $resultFile -PathType Leaf) { (Get-Content -LiteralPath $resultFile -Raw).Trim() } else { $null }
    Remove-Item -LiteralPath $resultFile -Force -ErrorAction SilentlyContinue
    # A failure the child recorded (state folder, PawnIO) counts even if the main items are gone.
    if ($reported -and $reported -ne 'removed') {
        Write-Host "`nLighting Studio was NOT removed completely: $reported" -ForegroundColor Red
        Write-Host 'Your editor data was kept. Run the uninstaller again or check the administrator window.'
        if (!$Quiet) { Read-Host 'Press Enter to close' | Out-Null }
        exit 1
    }
    # Without a report (window closed, child interrupted), judge by what is left, not by the exit code: closing the elevated window with X
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
    # A later step (registry) can fail after the copy; the user must still learn where the files are (CodeRabbit 2026-11-19).
    if (!(Remove-UserData)) { Show-Backup -OnlyCopied; if (!$Quiet) { Read-Host 'Press Enter to close' | Out-Null }; exit 1 }
    if (!$Quiet) {
        Write-Host "`nLighting Studio was removed. Projects saved outside $destination were not touched." -ForegroundColor Green
    }
    Show-Backup
    if (!$Quiet) { Read-Host 'Press Enter to close' | Out-Null }
    return
}
$problems = New-Object Collections.Generic.List[string]
$self = [IO.File]::ReadAllText($PSCommandPath)
$clock = [Diagnostics.Stopwatch]::StartNew()
$stepCount = 6 + [int][bool]$RemovePawnIO; $stepIndex = 0
# Progress bar plus one timed line per step, so a slow step does not look like a hang (2026-11-17).
function Invoke-Step([string]$Name, [scriptblock]$Action) {
    $script:stepIndex++
    Write-Progress -Activity 'Removing Lighting Studio' -Status "Step $script:stepIndex of ${stepCount}: $Name" -PercentComplete ([Math]::Min(100, 100 * ($script:stepIndex - 1) / $stepCount))
    try { & $Action; Write-Host ('[{0:mm\:ss}] Done: {1}' -f $clock.Elapsed, $Name) } catch { $problems.Add("$Name - $($_.Exception.Message)"); Write-Warning "$Name - $($_.Exception.Message)" }
}
if (Get-Editor) {
    Write-Host 'Lighting Studio is running. Quit it from its tray icon (right-click > Quit), then retry. Nothing was removed.' -ForegroundColor Red
    if ($ResultFile) { Set-Content -LiteralPath $ResultFile -Encoding UTF8 -Value 'Lighting Studio is still running; nothing was removed' }
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
# Only an already elevated console gets here without -Elevated; it never touches user data (see above).
if (!$Elevated) { $script:userDataSkipped = $true }
# The retry entry point is decided last, after every other step: after any failure the Apps entry stays
# and the script is put back, recreating its folder if needed. If removing the entry itself fails, the
# script is put back too, so the entry never points to a missing script (CodeRabbit reviews 2026-11-15/18).
$restore = { New-Item -ItemType Directory -Force -Path $destination | Out-Null; [IO.File]::WriteAllText((Join-Path $destination 'Uninstall-LightingStudio.ps1'), $self) }
if ($problems.Count) {
    Invoke-Step 'Keep the uninstaller for a retry' $restore
} else {
    Invoke-Step 'Remove the Windows Apps entry' {
        try { if (Test-Path -LiteralPath $uninstallKey) { Remove-Item -LiteralPath $uninstallKey -Recurse -Force } }
        catch { & $restore; throw }
    }
}
Write-Progress -Activity 'Removing Lighting Studio' -Completed
if ($ResultFile) { Set-Content -LiteralPath $ResultFile -Encoding UTF8 -Value $(if ($problems.Count) { $problems -join '; ' } else { 'removed' }) }
if ($problems.Count) {
    Write-Host "`nLighting Studio was NOT removed completely:" -ForegroundColor Red
    $problems | ForEach-Object { Write-Host "  $_" }
} else { Write-Host "`nLighting Studio was removed. Projects saved outside $destination were not touched." -ForegroundColor Green }
Show-Backup -OnlyCopied:($problems.Count -gt 0)
# The elevated child no longer waits for Enter on success: the visible parent reports the result.
if (!$Quiet -and ($problems.Count -or !$Elevated)) { Read-Host 'Press Enter to close' | Out-Null }
elseif ($Elevated -and !$Quiet) { Write-Host 'This window closes in 5 seconds.'; Start-Sleep -Seconds 5 }
if ($problems.Count) { exit 1 }
