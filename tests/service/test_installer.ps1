param([string]$Installer,[string]$Runtime)
$ErrorActionPreference='Stop'
$tokens=$null;$errors=$null
[Management.Automation.Language.Parser]::ParseFile($Installer,[ref]$tokens,[ref]$errors) | Out-Null
if ($errors.Count) { throw ($errors | Out-String) }
$sourceText = Get-Content -LiteralPath $Installer -Raw
if ($sourceText -notmatch "-ArgumentList @\('-install','-silent'\)") { throw 'PawnIO silent installation arguments differ from the verified official binary CLI' }
# WhatIf must reject a missing package and must return before elevation/system changes.
try { & $Installer -Source (Join-Path $Runtime 'missing') -WhatIf; throw 'Missing runtime accepted' }
catch { if ($_.Exception.Message -notmatch 'Missing runtime input') { throw } }
# Release packaging has its own binary/dependency test. A bounded fixture also
# exercises planning in Debug builds, whose Qt DLL names deliberately differ.
$temporaryRoot=[IO.Path]::GetFullPath([IO.Path]::GetTempPath())
$fixture=[IO.Path]::GetFullPath((Join-Path $temporaryRoot ('LightingStudio-plan-'+[guid]::NewGuid().ToString())))
try {
    foreach ($name in @('msi_mystic_light.exe','lighting_service_host.exe','Qt6Core.dll','Qt6Gui.dll','Qt6Widgets.dll','Qt6Svg.dll','Qt6Network.dll','hidapi.dll','modules/msi_gen1_module.dll','modules/peripherals_module.dll','modules/auxiliary_module.dll','modules/asrock_polychrome_module.dll','modules/lenovo_motherboard_module.dll','modules/hp_omen_module.dll','third_party/pawnio/SmbusPIIX4.bin','third_party/pawnio/source-0.2.11.zip','LICENSE','THIRD_PARTY_NOTICES.md','PORTABLE_README.txt','profiles/fixture.json','components/fixture.json','platforms/qwindows.dll','imageformats/qjpeg.dll','imageformats/qsvg.dll','third_party/licenses/fixture.txt')) {
        $path=Join-Path $fixture $name
        New-Item -ItemType Directory -Force -Path (Split-Path $path -Parent) | Out-Null
        [IO.File]::WriteAllText($path,'offline plan fixture')
    }
    & $Installer -Source $fixture -OwnerSid 'S-1-5-21-1-2-3-1001' -WhatIf
    # Install.cmd path: no -Source, run by Windows PowerShell 5.1 via -File from another working
    # directory. The default must be the package (the installer's parent folder), not the working
    # directory: a copy inside the complete fixture must plan successfully (CodeRabbit review 2026-11-18).
    $packaged = Join-Path $fixture 'installer'
    New-Item -ItemType Directory -Force -Path $packaged | Out-Null
    foreach ($script in @($Installer,(Join-Path (Split-Path $Installer -Parent) 'Uninstall-LightingStudio.ps1'))) { Copy-Item -LiteralPath $script -Destination $packaged }
    # 'Stop' would turn the child's stderr into a terminating NativeCommandError.
    $ErrorActionPreference='Continue'
    Push-Location -LiteralPath $temporaryRoot
    try { $output = & "$PSHOME/powershell.exe" -NoProfile -ExecutionPolicy Bypass -File (Join-Path $packaged (Split-Path $Installer -Leaf)) -OwnerSid 'S-1-5-21-1-2-3-1001' -WhatIf 2>&1 | Out-String; $code = $LASTEXITCODE }
    finally { Pop-Location; $ErrorActionPreference='Stop' }
    if ($code -ne 0) { throw "Default -Source did not resolve to the package: $output" }
} finally {
    if (!$fixture.StartsWith($temporaryRoot,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $fixture -Leaf) -notlike 'LightingStudio-plan-*') { throw 'Unsafe fixture cleanup path' }
    if (Test-Path -LiteralPath $fixture) { Remove-Item -LiteralPath $fixture -Recurse -Force }
}
# The uninstaller must parse, use PawnIO's own registered quiet-uninstall arguments, and return on
# WhatIf before any prompt, elevation or deletion.
$uninstaller = Join-Path (Split-Path $Installer -Parent) 'Uninstall-LightingStudio.ps1'
[Management.Automation.Language.Parser]::ParseFile($uninstaller,[ref]$tokens,[ref]$errors) | Out-Null
if ($errors.Count) { throw ($errors | Out-String) }
if ((Get-Content -LiteralPath $uninstaller -Raw) -notmatch "-ArgumentList @\('-uninstall','-silent'\)") { throw 'PawnIO silent removal arguments differ from its registered QuietUninstallString' }
& $uninstaller -RemovePawnIO -Quiet -WhatIf
# User data (owner decision 2026-11-19): every *.json below the folder is copied unrenamed to a dated
# backup, then the folder is deleted; nothing found means no backup; a failed copy changes nothing.
$ast=[Management.Automation.Language.Parser]::ParseFile($uninstaller,[ref]$null,[ref]$null)
foreach ($name in 'Remove-UserData','Get-JsonFiles','Remove-Tree','Remove-TreeItem') {
    . ([scriptblock]::Create(($ast.FindAll({$args[0] -is [Management.Automation.Language.FunctionDefinitionAst] -and $args[0].Name -eq $name},$true) | Select-Object -First 1).Extent.Text))
}
$userRoot=[IO.Path]::GetFullPath((Join-Path $temporaryRoot ('LightingStudio-userdata-'+[guid]::NewGuid().ToString())))
$keyRoot='HKCU:\Software\LightingStudioTest-'+[guid]::NewGuid().ToString()
try {
    foreach ($case in 'copied','empty','failed') {
        $parent=Join-Path $userRoot "$case\Lighting Studio"; $key="$keyRoot\$case\Editor"
        New-Item -ItemType Directory -Force -Path (Join-Path $parent 'Editor\sub') | Out-Null
        $names = if ($case -eq 'empty') { @('Editor\application.lock','Editor\sub\notes.txt') } else { @('top.json','Editor\recovery.lighting.json','Editor\application.lock','Editor\sub\deep.json','Editor\sub\notes.txt') }
        foreach ($file in $names) { [IO.File]::WriteAllText((Join-Path $parent $file),$file) }
        New-Item -Path $key -Force | Out-Null
        $backupRoot = Join-Path $userRoot "$case\profile"
        # A file where the profile folder should be makes the copy fail.
        if ($case -eq 'failed') { [IO.File]::WriteAllText($backupRoot,'not a folder') } else { New-Item -ItemType Directory -Force -Path $backupRoot | Out-Null }
        $script:backup=$null
        $ok = Remove-UserData -Parent $parent -BackupRoot $backupRoot -Key $key 3>$null 6>$null
        if ($case -eq 'failed') {
            if ($ok) { throw 'A failed backup copy reported success' }
            foreach ($file in $names) { if (!(Test-Path -LiteralPath (Join-Path $parent $file))) { throw "Failed copy lost $file" } }
            if (!(Test-Path -LiteralPath $key)) { throw 'Failed copy deleted the settings' }
            continue
        }
        if (!$ok -or (Test-Path -LiteralPath $parent) -or (Test-Path -LiteralPath $key)) { throw "User data removal ($case) left the folder or settings behind" }
        if ($case -eq 'empty') { if ($script:backup) { throw 'No JSON file but a backup was reported' }; continue }
        $backup=Join-Path $backupRoot ('Lighting Studio - Backup - '+(Get-Date -Format 'yyyy_MM_dd'))
        if ($script:backup -ne $backup) { throw 'Backup path not reported' }
        foreach ($file in 'top.json','Editor\recovery.lighting.json','Editor\sub\deep.json') { if (!(Test-Path -LiteralPath (Join-Path $backup $file))) { throw "Backup lacks $file" } }
        foreach ($file in 'Editor\application.lock','Editor\sub\notes.txt') { if (Test-Path -LiteralPath (Join-Path $backup $file)) { throw "Backup contains non-JSON $file" } }
    }
} finally {
    if (!$userRoot.StartsWith($temporaryRoot,[StringComparison]::OrdinalIgnoreCase)) { throw 'Unsafe user data cleanup path' }
    if (Test-Path -LiteralPath $userRoot) { Remove-Item -LiteralPath $userRoot -Recurse -Force }
    if (Test-Path -LiteralPath $keyRoot) { Remove-Item -LiteralPath $keyRoot -Recurse -Force }
}
Write-Host 'Installer and uninstaller parse; WhatIf validates a complete runtime without installing or removing.'
