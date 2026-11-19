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
# Owner decision 2026-11-19: profile files are never touched, registry settings always go. Only the
# program and service-data folders may be deleted; the editor key and its then empty parent are removed.
$ast=[Management.Automation.Language.Parser]::ParseFile($uninstaller,[ref]$null,[ref]$null)
$deleted = @($ast.FindAll({$args[0] -is [Management.Automation.Language.CommandAst] -and $args[0].GetCommandName() -eq 'Remove-Tree'},$true) | ForEach-Object { $_.CommandElements[1].Extent.Text })
if (($deleted -join ',') -ne '$destination,$state') { throw "Uninstaller deletes more than program and service data: $($deleted -join ', ')" }
. ([scriptblock]::Create(($ast.FindAll({$args[0] -is [Management.Automation.Language.FunctionDefinitionAst] -and $args[0].Name -eq 'Remove-EditorSettings'},$true) | Select-Object -First 1).Extent.Text))
$keyRoot='HKCU:\Software\LightingStudioTest-'+[guid]::NewGuid().ToString()
try {
    New-Item -Path "$keyRoot\Editor\sub" -Force | Out-Null
    Set-ItemProperty -LiteralPath "$keyRoot\Editor" -Name geometry -Value 'x'
    Remove-EditorSettings -Key "$keyRoot\Editor"
    if (Test-Path -LiteralPath $keyRoot) { throw 'Editor settings or their empty parent key were left behind' }
    # A parent key that still holds other values stays.
    New-Item -Path "$keyRoot\Editor" -Force | Out-Null
    Set-ItemProperty -LiteralPath $keyRoot -Name other -Value 'y'
    Remove-EditorSettings -Key "$keyRoot\Editor"
    if ((Test-Path -LiteralPath "$keyRoot\Editor") -or !(Test-Path -LiteralPath $keyRoot)) { throw 'Parent key with other values was not kept' }
} finally {
    if (Test-Path -LiteralPath $keyRoot) { Remove-Item -LiteralPath $keyRoot -Recurse -Force }
}
Write-Host 'Installer and uninstaller parse; WhatIf validates a complete runtime without installing or removing.'
