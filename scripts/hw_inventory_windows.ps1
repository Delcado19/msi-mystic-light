# Read-only hardware inventory for cross-platform RGB backend research.
# No writes, no SMBus/I2C probing, no device open.

Write-Host "`n=== Board identity (CIM) ==="
Get-CimInstance Win32_BaseBoard | Select-Object Manufacturer, Product, Version
Get-CimInstance Win32_BIOS | Select-Object Manufacturer, SMBIOSBIOSVersion

Write-Host "`n=== PCI devices (relevant classes) ==="
Get-PnpDevice -PresentOnly | Where-Object { $_.Class -in 'Display','SMBus','System' } |
  Select-Object FriendlyName, InstanceId, Class | Format-Table -AutoSize

Write-Host "`n=== HID devices (VID/PID) ==="
Get-PnpDevice -PresentOnly -Class HIDClass |
  Select-Object FriendlyName, InstanceId | Format-Table -AutoSize

Write-Host "`n=== USB devices ==="
Get-PnpDevice -PresentOnly -Class USB |
  Select-Object FriendlyName, InstanceId | Format-Table -AutoSize

Write-Host "`n=== Known RGB-relevant services ==="
Get-Service | Where-Object { $_.Name -match 'LEDKeeper|MysticLight|SignalRgb|OpenRGB|NTIOLib|PawnIO' } |
  Select-Object Name, Status, DisplayName

Write-Host "`n=== Known RGB-relevant drivers (signed status only, no load) ==="
Get-CimInstance Win32_SystemDriver | Where-Object { $_.Name -match 'SignalRgb|PawnIO|NTIOLib|EneIo' } |
  Select-Object Name, State, PathName
