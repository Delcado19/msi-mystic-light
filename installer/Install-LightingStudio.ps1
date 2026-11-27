[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [string]$Source,
    [string]$OwnerSid,
    [string]$PawnIOSetup,
    [switch]$Elevated,
    # Written by the elevated child only after it completed; proves this run, not a previous install.
    [string]$ResultFile
)
$ErrorActionPreference = 'Stop'
# Windows PowerShell 5.1 leaves $PSScriptRoot empty in param() defaults of [CmdletBinding()] scripts,
# which broke Install.cmd from the CI package; resolve the default here, where it is populated.
if (!$Source) { $Source = Split-Path $PSScriptRoot -Parent }
# Capture the interactive owner before elevation, including credential-based UAC.
if (!$OwnerSid) { $OwnerSid = [Security.Principal.WindowsIdentity]::GetCurrent().User.Value }
if ($OwnerSid -notmatch '^S-1-5-21-\d+-\d+-\d+-\d+$') { throw 'Select a local or domain user SID, not a service account.' }
$Source = [IO.Path]::GetFullPath($Source)
$destination = Join-Path ([Environment]::GetFolderPath('ProgramFiles')) 'Lighting Studio'
$state = Join-Path ([Environment]::GetFolderPath('CommonApplicationData')) 'LightingStudio'
$required = @('msi_mystic_light.exe','lighting_service_host.exe','Qt6Core.dll','Qt6Gui.dll','Qt6Widgets.dll','Qt6Svg.dll','Qt6Network.dll','hidapi.dll','imageformats/qsvg.dll',
    'modules/msi_gen1_module.dll','modules/peripherals_module.dll','modules/auxiliary_module.dll',
    'modules/asrock_polychrome_module.dll','modules/lenovo_motherboard_module.dll','modules/hp_omen_module.dll',
    'third_party/pawnio/SmbusPIIX4.bin','third_party/pawnio/source-0.2.11.zip','LICENSE','THIRD_PARTY_NOTICES.md','PORTABLE_README.txt')
foreach ($relative in $required) { if (!(Test-Path -LiteralPath (Join-Path $Source $relative) -PathType Leaf)) { throw "Missing runtime input: $relative" } }
foreach ($folder in @('profiles','components','platforms','imageformats','third_party/licenses')) { if (!(Test-Path -LiteralPath (Join-Path $Source $folder) -PathType Container)) { throw "Missing runtime folder: $folder" } }
$uninstaller = Join-Path $PSScriptRoot 'Uninstall-LightingStudio.ps1'
if (!(Test-Path -LiteralPath $uninstaller -PathType Leaf)) { throw 'Missing runtime input: installer/Uninstall-LightingStudio.ps1' }
if (!$PSCmdlet.ShouldProcess($destination, 'Install Lighting Studio service, official PawnIO if missing, and the Start menu entry')) { return }
$pawnIOKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PawnIO'
$editor = Join-Path $destination 'msi_mystic_light.exe'
# An open editor blocks the file copy. Ask before
# UAC and before anything is stopped (2026-11-17: the check used to follow the service stop).
function Test-EditorRunning { [bool](Get-Process msi_mystic_light -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $editor }) }
# A pre-R38 editor can run hidden in the tray, so the message says how to reach it (CodeRabbit PR #10).
$editorRunning = 'Lighting Studio is running. Close its window (if it is hidden, open it from its icon in the notification area first), then start the installer again.'
if (Test-EditorRunning) { throw $editorRunning }
$identity = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
# A path ending in a backslash (e.g. D:\) would escape the closing quote of a command-line argument;
# "D:\." names the same folder without that trailing backslash (audit 2026-11-14).
function Quote-Argument([string]$Path) { if ($Path.EndsWith('\')) { $Path += '.' }; return '"' + $Path + '"' }
if (!$identity.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    $arguments = @('-NoProfile','-ExecutionPolicy','Bypass','-File',(Quote-Argument $PSCommandPath),'-Source',(Quote-Argument $Source),'-OwnerSid',$OwnerSid)
    if ($PawnIOSetup) { $arguments += @('-PawnIOSetup',(Quote-Argument ([IO.Path]::GetFullPath($PawnIOSetup)))) }
    $resultFile = Join-Path ([IO.Path]::GetTempPath()) ('LightingStudio-install-' + [guid]::NewGuid().ToString('N') + '.result')
    $arguments += @('-Elevated','-ResultFile',(Quote-Argument $resultFile))
    foreach ($path in @($PSCommandPath,$Source,$PawnIOSetup,$resultFile)) { if ($path -match '["\r\n]') { throw 'Invalid installer path' } }
    # The elevated window is visible and shows each step (owner report 2026-11-17: a hidden child left
    # only an empty window for minutes). Success is judged by the installed state, not the exit code,
    # so closing that window early can neither hide nor fake the result.
    Write-Host 'Installing in the administrator window that opens now. Confirm the Windows prompt (UAC).'
    $child = Start-Process -FilePath "$PSHOME/powershell.exe" -ArgumentList $arguments -Verb RunAs -Wait -PassThru
    # During an update the old service and editor already satisfy the state check, so this run must
    # also have reported completion (CodeRabbit review 2026-11-17).
    $completed = (Test-Path -LiteralPath $resultFile -PathType Leaf) -and ((Get-Content -LiteralPath $resultFile -Raw).Trim() -eq 'installed')
    Remove-Item -LiteralPath $resultFile -Force -ErrorAction SilentlyContinue
    $service = Get-Service -Name LightingStudio -ErrorAction SilentlyContinue
    if (!$completed -or !$service -or $service.Status -ne 'Running' -or !(Test-Path -LiteralPath $editor -PathType Leaf)) {
        throw "Installation did not complete (exit code $($child.ExitCode)). The administrator window listed the failed step; run Install.cmd again to retry."
    }
    Write-Host 'Installed. Lighting starts with Windows; open Lighting Studio from the Start menu.' -ForegroundColor Green
    Read-Host 'Press Enter to close' | Out-Null
    return
}
# Elevated part. A failure stays visible in this window; the parent re-checks the result anyway.
trap {
    Write-Host "`nInstallation failed: $($_.Exception.Message)" -ForegroundColor Red
    if ($Elevated) { Read-Host 'Press Enter to close' | Out-Null }
    break
}
$clock = [Diagnostics.Stopwatch]::StartNew()
$stepCount = 8; $stepIndex = 0
# The elapsed time per line shows which step is slow; no step has been timed on a real install yet.
function Show-Step([string]$Text) {
    $script:stepIndex++
    Write-Progress -Activity 'Installing Lighting Studio' -Status "Step $script:stepIndex of ${stepCount}: $Text" -PercentComplete ([Math]::Min(100, 100 * ($script:stepIndex - 1) / $stepCount))
    Write-Host ('[{0:mm\:ss}] {1}' -f $clock.Elapsed, $Text)
}
# A blocking -Wait looks like a hang; poll and count the seconds instead.
function Wait-Visible($Process, [string]$Text) {
    $null = $Process.Handle  # Cache the handle; otherwise ExitCode stays empty after the process ends.
    $started = Get-Date
    while (!$Process.HasExited) {
        Write-Progress -Activity 'Installing Lighting Studio' -Status "Step $script:stepIndex of ${stepCount}: $Text ($([int]((Get-Date) - $started).TotalSeconds) s)" -PercentComplete ([Math]::Min(100, 100 * ($script:stepIndex - 1) / $stepCount))
        Start-Sleep -Milliseconds 500
    }
    return $Process.ExitCode
}
function Protect-Directory([string]$Path, [bool]$Readable) {
    if (Test-Path -LiteralPath $Path) {
        if ((Get-Item -LiteralPath $Path -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Reparse target rejected: $Path" }
        if (Get-ChildItem -LiteralPath $Path -Force -Recurse | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }) { throw "Reparse entry rejected in $Path" }
        # Do not hash or copy over a tree that a normal user can still modify.
        # Existing installs created here have SYSTEM/Administrators ownership and inherited ACLs.
        $trusted = @('S-1-5-18','S-1-5-32-544')
        $writeMask = [Security.AccessControl.FileSystemRights]::WriteData -bor [Security.AccessControl.FileSystemRights]::AppendData -bor [Security.AccessControl.FileSystemRights]::WriteExtendedAttributes -bor [Security.AccessControl.FileSystemRights]::WriteAttributes -bor [Security.AccessControl.FileSystemRights]::Delete -bor [Security.AccessControl.FileSystemRights]::DeleteSubdirectoriesAndFiles -bor [Security.AccessControl.FileSystemRights]::ChangePermissions -bor [Security.AccessControl.FileSystemRights]::TakeOwnership
        foreach ($entry in @((Get-Item -LiteralPath $Path -Force)) + @(Get-ChildItem -LiteralPath $Path -Force -Recurse)) {
            $existingAcl = Get-Acl -LiteralPath $entry.FullName
            if ($existingAcl.GetOwner([Security.Principal.SecurityIdentifier]).Value -notin $trusted) { throw "Untrusted installation owner: $($entry.FullName). Repair/remove this tree explicitly as administrator." }
            foreach ($rule in $existingAcl.Access) {
                if ($rule.AccessControlType -eq 'Allow' -and ($rule.FileSystemRights -band $writeMask) -and $rule.IdentityReference.Translate([Security.Principal.SecurityIdentifier]).Value -notin $trusted) { throw "Untrusted write permission: $($entry.FullName). Repair its ACL explicitly before installing." }
            }
        }
    } else { New-Item -ItemType Directory -Path $Path | Out-Null }
    $acl = New-Object Security.AccessControl.DirectorySecurity
    $acl.SetAccessRuleProtection($true,$false)
    $acl.SetOwner([Security.Principal.SecurityIdentifier]'S-1-5-32-544')
    foreach ($sid in @('S-1-5-18','S-1-5-32-544')) {
        $acl.AddAccessRule((New-Object Security.AccessControl.FileSystemAccessRule([Security.Principal.SecurityIdentifier]$sid,'FullControl','ContainerInherit,ObjectInherit','None','Allow')))
    }
    if ($Readable) { $acl.AddAccessRule((New-Object Security.AccessControl.FileSystemAccessRule([Security.Principal.SecurityIdentifier]'S-1-5-32-545','ReadAndExecute','ContainerInherit,ObjectInherit','None','Allow'))) }
    Set-Acl -LiteralPath $Path -AclObject $acl
}
Show-Step 'Checking the PawnIO driver'
if (Test-EditorRunning) { throw $editorRunning }
# Keep official signed binaries unchanged. An elevated parent supplies the only UAC prompt.
# PawnIO 2.2.0 embeds this CLI contract: -install -silent (it is not an NSIS /S installer).
# PawnIO's own uninstaller removed its Apps entry but left the driver service registered
# (observed 2026-11-17), so both must exist; otherwise the official setup runs again.
if (!(Get-Service -Name PawnIO -ErrorAction SilentlyContinue) -or !(Test-Path -LiteralPath $pawnIOKey)) {
    $downloadedSetup = $null
    # Hash and run a copy in an administrators-only folder: a file in the user's temp folder could be
    # swapped by an unelevated process between the hash check and the start (audit 2026-11-14).
    $setupFolder = Join-Path ([Environment]::GetFolderPath('CommonApplicationData')) ('LightingStudio-PawnIO-' + [guid]::NewGuid().ToString())
    # Download and staging run inside the cleanup scope: a failed download or folder setup must not
    # leave the setup file behind (CodeRabbit review 2026-11-18).
    try {
        if (!$PawnIOSetup) {
            Write-Host '        Downloading PawnIO 2.2.0 from github.com/namazso ...'
            $downloadedSetup = Join-Path ([IO.Path]::GetTempPath()) ('LightingStudio-PawnIO-' + [guid]::NewGuid().ToString() + '.exe')
            $PawnIOSetup = $downloadedSetup
            # Windows PowerShell 5.1 downloads many times slower while it draws its own progress bar.
            $previousProgress = $ProgressPreference; $ProgressPreference = 'SilentlyContinue'
            try { Invoke-WebRequest 'https://github.com/namazso/PawnIO.Setup/releases/download/2.2.0/PawnIO_setup.exe' -UseBasicParsing -OutFile $PawnIOSetup }
            finally { $ProgressPreference = $previousProgress }
        }
        Protect-Directory $setupFolder $false
        $protectedSetup = Join-Path $setupFolder 'PawnIO_setup.exe'
        Copy-Item -LiteralPath $PawnIOSetup -Destination $protectedSetup
        if ((Get-FileHash -LiteralPath $protectedSetup -Algorithm SHA256).Hash.ToLowerInvariant() -ne '1f519a22e47187f70a1379a48ca604981c4fcf694f4e65b734aaa74a9fba3032') { throw 'PawnIO installer digest rejected' }
        $exitCode = Wait-Visible (Start-Process -FilePath $protectedSetup -ArgumentList @('-install','-silent') -PassThru -WindowStyle Hidden) 'Installing PawnIO'
        if ($exitCode -ne 0) { throw "PawnIO installation failed or needs attention ($exitCode)." }
    } finally {
        Remove-Item -LiteralPath $setupFolder -Recurse -Force -ErrorAction SilentlyContinue
        # Only our own download is deleted; a setup file supplied with -PawnIOSetup belongs to the caller.
        if ($downloadedSetup) { Remove-Item -LiteralPath $downloadedSetup -Force -ErrorAction SilentlyContinue }
    }
    if (!(Get-Service -Name PawnIO -ErrorAction SilentlyContinue)) { throw 'PawnIO installation did not register its driver. Restart Windows if requested, then rerun installation.' }
}
Show-Step 'Stopping the running Lighting Studio service'
$existing = Get-Service -Name LightingStudio -ErrorAction SilentlyContinue
if ($existing) { Stop-Service LightingStudio; $existing.WaitForStatus('Stopped',[TimeSpan]::FromSeconds(30)) }
Show-Step 'Checking the installation folders'
Protect-Directory $destination $true
Protect-Directory $state $false
Show-Step 'Copying program files'
# Mirror the runtime folders: the service refuses to start when modules holds a DLL it does not expect,
# and a profile or component dropped from the package must not stay active (audit 2026-11-17).
# Protect-Directory above rejected reparse points inside the tree, so the recursive delete stays inside.
foreach ($folder in @('modules','profiles','components','platforms','imageformats','third_party')) {
    $old = Join-Path $destination $folder
    if (Test-Path -LiteralPath $old) { Remove-Item -LiteralPath $old -Recurse -Force }
}
foreach ($relative in $required) {
    $target = Join-Path $destination $relative
    New-Item -ItemType Directory -Force -Path (Split-Path $target -Parent) | Out-Null
    Copy-Item -LiteralPath (Join-Path $Source $relative) -Destination $target -Force
}
foreach ($folder in @('profiles','components','platforms','imageformats','third_party/licenses')) {
    if (!(Test-Path -LiteralPath (Join-Path $Source $folder) -PathType Container)) { throw "Missing runtime folder: $folder" }
    $targetFolder = Join-Path $destination $folder
    New-Item -ItemType Directory -Force -Path $targetFolder | Out-Null
    foreach ($entry in Get-ChildItem -LiteralPath (Join-Path $Source $folder) -Force) {
        Copy-Item -LiteralPath $entry.FullName -Destination $targetFolder -Recurse -Force
    }
}
Show-Step 'Securing file permissions'
# Root ACLs are already protected; reset descendants without reopening the root to parent grants.
foreach ($root in @($destination,$state)) {
    & icacls.exe $root /setowner '*S-1-5-32-544' /T /Q | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "Cannot secure ownership: $root" }
    foreach ($entry in Get-ChildItem -LiteralPath $root -Force) {
        & icacls.exe $entry.FullName /reset /T /Q | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "Cannot reset access permissions: $($entry.FullName)" }
    }
}
Show-Step 'Registering the service'
$hashes = @{}
foreach ($name in @('msi_gen1_module.dll','peripherals_module.dll','auxiliary_module.dll','asrock_polychrome_module.dll','lenovo_motherboard_module.dll','hp_omen_module.dll')) { $hashes[$name] = (Get-FileHash -LiteralPath (Join-Path $destination "modules/$name") -Algorithm SHA256).Hash.ToLowerInvariant() }
@{ownerSid=$OwnerSid;modules=$hashes} | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $destination 'service.json') -Encoding UTF8
$exe = Join-Path $destination 'lighting_service_host.exe'
if (!$existing) { New-Service -Name LightingStudio -DisplayName 'Lighting Studio' -BinaryPathName ('"'+$exe+'" --service') -StartupType Automatic | Out-Null }
else {
    # Use structured WMI arguments so Windows PowerShell cannot strip service-path quotes.
    $service = Get-CimInstance Win32_Service -Filter "Name='LightingStudio'"
    $change = Invoke-CimMethod -InputObject $service -MethodName Change -Arguments @{PathName=('"'+$exe+'" --service');StartMode='Automatic';StartName='LocalSystem';StartPassword=''}
    if ($change.ReturnValue -ne 0) { throw "Service update failed ($($change.ReturnValue))" }
}
& sc.exe description LightingStudio 'Persistent RGB playback; controlled by the unelevated Lighting Studio editor.' | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'Service description update failed' }
& sc.exe failure LightingStudio reset= 86400 actions= restart/10000/restart/30000/restart/60000 | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'Service recovery configuration failed' }
Show-Step 'Registering Start menu entry and uninstaller'
# No editor logon task since GS-06-R38 (owner decision 2026-11-25): the service plays the lights alone and
# the hidden tray editor rendered its preview at 60 fps. Updates remove the task older versions created.
if (Get-ScheduledTask -TaskName 'Lighting Studio Editor' -ErrorAction SilentlyContinue) {
    Unregister-ScheduledTask -TaskName 'Lighting Studio Editor' -Confirm:$false
}
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut((Join-Path ([Environment]::GetFolderPath('CommonPrograms')) 'Lighting Studio.lnk'))
$shortcut.TargetPath = Join-Path $destination 'msi_mystic_light.exe'
$shortcut.WorkingDirectory = $destination
$shortcut.Save()
# Complete removal: the uninstaller is installed with the program and registered in Windows Settings > Apps.
Copy-Item -LiteralPath $uninstaller -Destination $destination -Force
$uninstallKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\LightingStudio'
New-Item -Path $uninstallKey -Force | Out-Null
Set-ItemProperty -LiteralPath $uninstallKey -Name DisplayName -Value 'Lighting Studio'
Set-ItemProperty -LiteralPath $uninstallKey -Name DisplayIcon -Value (Join-Path $destination 'msi_mystic_light.exe')
Set-ItemProperty -LiteralPath $uninstallKey -Name Publisher -Value 'Delcado'
Set-ItemProperty -LiteralPath $uninstallKey -Name InstallLocation -Value $destination
Set-ItemProperty -LiteralPath $uninstallKey -Name UninstallString -Value ('powershell.exe -NoProfile -ExecutionPolicy Bypass -File "' + (Join-Path $destination 'Uninstall-LightingStudio.ps1') + '"')
Set-ItemProperty -LiteralPath $uninstallKey -Name NoModify -Value 1 -Type DWord
Set-ItemProperty -LiteralPath $uninstallKey -Name NoRepair -Value 1 -Type DWord
& sc.exe failureflag LightingStudio 1 | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'Service failure recovery flag could not be set' }
Show-Step 'Starting the Lighting Studio service'
Start-Service LightingStudio
(Get-Service LightingStudio).WaitForStatus('Running',[TimeSpan]::FromSeconds(30))
if ($ResultFile) { Set-Content -LiteralPath $ResultFile -Value 'installed' -Encoding ascii }
Write-Progress -Activity 'Installing Lighting Studio' -Completed
Write-Host ('[{0:mm\:ss}] Installed. This window closes in 5 seconds.' -f $clock.Elapsed) -ForegroundColor Green
if ($Elevated) { Start-Sleep -Seconds 5 }
