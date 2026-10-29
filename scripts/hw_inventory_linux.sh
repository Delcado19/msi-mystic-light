#!/usr/bin/env bash
# Read-only hardware inventory for cross-platform RGB backend research.
# No writes, no i2c/SMBus bus scans (a quick-write probe can corrupt SPD/EEPROM).
set -euo pipefail

section() { printf '\n=== %s ===\n' "$1"; }

section "Kernel / distro"
uname -a
[ -f /etc/os-release ] && cat /etc/os-release

section "DMI board identity (/sys/class/dmi/id)"
for f in board_vendor board_name board_version sys_vendor product_name bios_version; do
  printf '%s: %s\n' "$f" "$(cat /sys/class/dmi/id/"$f" 2>/dev/null || echo '?')"
done

section "PCI devices (lspci -nnv)"
command -v lspci >/dev/null && lspci -nnv || echo "lspci not found"

section "USB devices (lsusb -v summary)"
command -v lsusb >/dev/null && lsusb || echo "lsusb not found"

section "HID devices (/sys/class/hidraw)"
for d in /sys/class/hidraw/*/device; do
  [ -d "$d" ] || continue
  printf -- '--- %s ---\n' "$d"
  cat "$d/uevent" 2>/dev/null || true
done

section "System LED class devices (/sys/class/leds)"
ls -1 /sys/class/leds 2>/dev/null || echo "none"

section "I2C buses (list only, no scan)"
command -v i2cdetect >/dev/null && i2cdetect -l || echo "i2cdetect not found"

section "Loaded I2C/SMBus kernel modules"
lsmod | grep -iE 'i2c|smbus' || echo "none loaded"

section "OpenRGB presence"
command -v openrgb >/dev/null && openrgb --version || echo "openrgb not installed"

section "hwmon (sensors/fan controllers, informational)"
for d in /sys/class/hwmon/*; do
  [ -d "$d" ] || continue
  printf '%s: %s\n' "$d" "$(cat "$d/name" 2>/dev/null || echo '?')"
done
