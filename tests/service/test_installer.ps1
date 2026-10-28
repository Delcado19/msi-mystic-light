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
    foreach ($name in @('msi_mystic_light.exe','lighting_service_host.exe','Qt6Core.dll','Qt6Gui.dll','Qt6Widgets.dll','Qt6Svg.dll','Qt6Network.dll','hidapi.dll','modules/msi_gen1_module.dll','modules/peripherals_module.dll','modules/auxiliary_module.dll','third_party/pawnio/SmbusPIIX4.bin','third_party/pawnio/source-0.2.11.zip','LICENSE','THIRD_PARTY_NOTICES.md','PORTABLE_README.txt','profiles/fixture.json','components/fixture.json','platforms/qwindows.dll','imageformats/qjpeg.dll','third_party/licenses/fixture.txt')) {
        $path=Join-Path $fixture $name
        New-Item -ItemType Directory -Force -Path (Split-Path $path -Parent) | Out-Null
        [IO.File]::WriteAllText($path,'offline plan fixture')
    }
    & $Installer -Source $fixture -OwnerSid 'S-1-5-21-1-2-3-1001' -WhatIf
} finally {
    if (!$fixture.StartsWith($temporaryRoot,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $fixture -Leaf) -notlike 'LightingStudio-plan-*') { throw 'Unsafe fixture cleanup path' }
    if (Test-Path -LiteralPath $fixture) { Remove-Item -LiteralPath $fixture -Recurse -Force }
}
Write-Host 'Installer parses and WhatIf validates a complete runtime without installing.'
