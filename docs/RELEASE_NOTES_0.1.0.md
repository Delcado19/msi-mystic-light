# Lighting Studio 0.1.0

## Changes

- Separate Layout, Effects and Hardware workspaces in one application.
- Layered paint, shapes, particles, editable gradients, masks, transparency and blend modes.
- Shared green/red Canvas Start/Stop, fullscreen canvas, Layout scrollbars and a bottom-right minimap.
- Growing Layout bounds, live sampling during component/group movement, resize and rotation.
- Readable LED arrangement markers with centered numbers; reserved Vulcan protocol slots stay hidden without changing wire order.
- Flat Endpoint / Profile / State output overview and automatic Canvas output for configured detected devices.
- Restored MSI I/O shield shared-color output and qualified Gigabyte RTX 5080 AERO OC NVAPI output.
- Direct PawnIO RAM output for two qualified ADATA XPG modules, with volatile activation and first-frame register verification.
- Native Windows documentation screenshots and checked portable packaging with SHA-256 output.

## Requirements and limitations

Windows x64, the Microsoft Visual C++ x64 runtime and the complete portable folder
are required. HID access depends on device permissions. The application manifest
uses `asInvoker`; it does not automatically elevate or change device permissions.

RAM additionally requires separately installed official PawnIO 2.x and administrator
access under the installed driver's policy. Support is limited to AMD PCI 1022:790b,
primary SMBus port, addresses 0x71/0x73, ENE AUDA0-E6K5-0101 and eight LEDs per module.
The user confirmed physical Canvas output with the diagnostic build. This does not
qualify other firmware, buses or products. No OpenRGB process or helper is required.

Software tests cover report encoding, invalid/boundary inputs, output scheduling,
editor interactions and packaging. They do not establish physical color calibration
or support for untested hardware. The agent did not perform physical RGB writes.
MSI configuration writes contain all 18 zones; unassigned firmware settings are not
read back or preserved. Stopping output holds the last device state.

Legacy hardware geometry can be imported; legacy custom effect recipes are not
converted. Layout expansion changes the mapping of the fixed Effects raster across
the complete Layout world. Canvas Stop pauses animation; Hardware Stop controls output.

The binaries are unsigned. No custom kernel driver, automatic PawnIO installation,
SignPath approval, Windows certification or public release publication is implied.
