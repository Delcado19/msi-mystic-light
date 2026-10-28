[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [string]$Source = (Split-Path $PSScriptRoot -Parent),
    [string]$OwnerSid,
    [string]$PawnIOSetup,
    [switch]$NoEditorAutostart
)
$ErrorActionPreference = 'Stop'
# Capture the interactive owner before elevation, including credential-based UAC.
if (!$OwnerSid) { $OwnerSid = [Security.Principal.WindowsIdentity]::GetCurrent().User.Value }
if ($OwnerSid -notmatch '^S-1-5-21-\d+-\d+-\d+-\d+$') { throw 'Select a local or domain user SID, not a service account.' }
$Source = [IO.Path]::GetFullPath($Source)
$destination = Join-Path ([Environment]::GetFolderPath('ProgramFiles')) 'Lighting Studio'
$state = Join-Path ([Environment]::GetFolderPath('CommonApplicationData')) 'LightingStudio'
$required = @('msi_mystic_light.exe','lighting_service_host.exe','Qt6Core.dll','Qt6Gui.dll','Qt6Widgets.dll','Qt6Svg.dll','Qt6Network.dll','hidapi.dll',
    'modules/msi_gen1_module.dll','modules/peripherals_module.dll','modules/auxiliary_module.dll',
    'third_party/pawnio/SmbusPIIX4.bin','third_party/pawnio/source-0.2.11.zip','LICENSE','THIRD_PARTY_NOTICES.md','PORTABLE_README.txt')
foreach ($relative in $required) { if (!(Test-Path -LiteralPath (Join-Path $Source $relative) -PathType Leaf)) { throw "Missing runtime input: $relative" } }
foreach ($folder in @('profiles','components','platforms','imageformats','third_party/licenses')) { if (!(Test-Path -LiteralPath (Join-Path $Source $folder) -PathType Container)) { throw "Missing runtime folder: $folder" } }
if (!$PSCmdlet.ShouldProcess($destination, 'Install Lighting Studio service, official PawnIO if missing, and editor logon task')) { return }
$identity = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (!$identity.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    $arguments = @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"' + $PSCommandPath + '"'),'-Source',('"' + $Source + '"'),'-OwnerSid',$OwnerSid)
    if ($PawnIOSetup) { $arguments += @('-PawnIOSetup',('"' + [IO.Path]::GetFullPath($PawnIOSetup) + '"')) }
    if ($NoEditorAutostart) { $arguments += '-NoEditorAutostart' }
    foreach ($path in @($PSCommandPath,$Source,$PawnIOSetup)) { if ($path -match '["\r\n]') { throw 'Invalid installer path' } }
    $child = Start-Process -FilePath "$PSHOME/powershell.exe" -ArgumentList $arguments -Verb RunAs -Wait -PassThru -WindowStyle Hidden
    if ($child.ExitCode -ne 0) { throw "Elevated installation failed ($($child.ExitCode)); inspect the installer output or rerun from an administrator console." }
    return
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
# Keep official signed binaries unchanged. An elevated parent supplies the only UAC prompt.
# PawnIO 2.2.0 embeds this CLI contract: -install -silent (it is not an NSIS /S installer).
if (!(Get-Service -Name PawnIO -ErrorAction SilentlyContinue)) {
    if (!$PawnIOSetup) {
        $PawnIOSetup = Join-Path ([IO.Path]::GetTempPath()) ('LightingStudio-PawnIO-' + [guid]::NewGuid().ToString() + '.exe')
        Invoke-WebRequest 'https://github.com/namazso/PawnIO.Setup/releases/download/2.2.0/PawnIO_setup.exe' -UseBasicParsing -OutFile $PawnIOSetup
    }
    if ((Get-FileHash -LiteralPath $PawnIOSetup -Algorithm SHA256).Hash.ToLowerInvariant() -ne '1f519a22e47187f70a1379a48ca604981c4fcf694f4e65b734aaa74a9fba3032') { throw 'PawnIO installer digest rejected' }
    $result = Start-Process -FilePath $PawnIOSetup -ArgumentList @('-install','-silent') -Wait -PassThru -WindowStyle Hidden
    if ($result.ExitCode -ne 0) { throw "PawnIO installation failed or needs attention ($($result.ExitCode))." }
    if (!(Get-Service -Name PawnIO -ErrorAction SilentlyContinue)) { throw 'PawnIO installation did not register its driver. Restart Windows if requested, then rerun installation.' }
}
$existing = Get-Service -Name LightingStudio -ErrorAction SilentlyContinue
if ($existing) { Stop-Service LightingStudio; $existing.WaitForStatus('Stopped',[TimeSpan]::FromSeconds(30)) }
Protect-Directory $destination $true
Protect-Directory $state $false
if (Get-Process msi_mystic_light -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq (Join-Path $destination 'msi_mystic_light.exe') }) { throw 'Close the installed editor before updating.' }
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
# Root ACLs are already protected; reset descendants without reopening the root to parent grants.
foreach ($root in @($destination,$state)) {
    & icacls.exe $root /setowner '*S-1-5-32-544' /T /Q | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "Cannot secure ownership: $root" }
    foreach ($entry in Get-ChildItem -LiteralPath $root -Force) {
        & icacls.exe $entry.FullName /reset /T /Q | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "Cannot reset access permissions: $($entry.FullName)" }
    }
}
$hashes = @{}
foreach ($name in @('msi_gen1_module.dll','peripherals_module.dll','auxiliary_module.dll')) { $hashes[$name] = (Get-FileHash -LiteralPath (Join-Path $destination "modules/$name") -Algorithm SHA256).Hash.ToLowerInvariant() }
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
# An interactive task uses the installing user's ordinary token, never highest privileges.
if (!$NoEditorAutostart) {
    $principal = New-ScheduledTaskPrincipal -UserId $OwnerSid -LogonType Interactive -RunLevel Limited
    $trigger = New-ScheduledTaskTrigger -AtLogOn -User $OwnerSid
    $action = New-ScheduledTaskAction -Execute (Join-Path $destination 'msi_mystic_light.exe') -Argument '--background'
    Register-ScheduledTask -TaskName 'Lighting Studio Editor' -Action $action -Trigger $trigger -Principal $principal -Force | Out-Null
} elseif (Get-ScheduledTask -TaskName 'Lighting Studio Editor' -ErrorAction SilentlyContinue) {
    Unregister-ScheduledTask -TaskName 'Lighting Studio Editor' -Confirm:$false
}
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut((Join-Path ([Environment]::GetFolderPath('CommonPrograms')) 'Lighting Studio.lnk'))
$shortcut.TargetPath = Join-Path $destination 'msi_mystic_light.exe'
$shortcut.WorkingDirectory = $destination
$shortcut.Save()
& sc.exe failureflag LightingStudio 1 | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'Service failure recovery flag could not be set' }
Start-Service LightingStudio
(Get-Service LightingStudio).WaitForStatus('Running',[TimeSpan]::FromSeconds(30))
Write-Host 'Installed. Lighting starts with Windows; open Lighting Studio to configure devices and effects.'
