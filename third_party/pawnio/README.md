# PawnIO AMD SMBus module

The user approved direct PawnIO integration on 2026-10-24. No OpenRGB process,
network helper, MSI driver or custom unsigned driver is used for RAM.

`SmbusPIIX4.bin` is the unchanged, signed AMD SMBus module from upstream
PawnIO.Modules release 0.2.11:
https://github.com/namazso/PawnIO.Modules/releases/tag/0.2.11

SHA-256: `91f9b4b1c39e3d399ce48477a89d8f6bd3e58a2241064a4daec3a1513dff56e5`
Size: 42,404 bytes. The host checks both before loading. PawnIO verifies the
upstream signature. This module is not rebuilt or modified by this project.

`source-0.2.11.zip` contains the corresponding upstream source, includes and
license obtained from GitHub's source archive endpoint for the release tag.
The module source is LGPL-2.1-or-later; see the included source copyright
headers and `../licenses/PawnIO-Modules-LGPL-2.1.txt`.

Install the official signed PawnIO 2.x distribution from https://pawnio.eu.
The app loads the installed `Program Files/PawnIO/PawnIOLib.dll` directly.
The driver, installer and library are not redistributed here; there is no
automatic installation, service launch, security-policy change or fallback
to an unsigned module. Installing PawnIO requires Windows elevation. Runtime
driver/mutex access must also be permitted by the current security policy. The
official installed driver grants access to SYSTEM/administrators; run Lighting
Studio as administrator for RAM output. The app does not elevate itself or
change the driver ACL.

The RAM host adapter is currently qualified only for AMD SMBus PCI 1022:790b,
primary port 0, addresses 0x71/0x73, exact ENE AUDA0-E6K5-0101 firmware and eight
LEDs. Profile VID/PID identify the PCI bus controller, not fabricated USB RAM.
GUI discovery selects registers to read identity/configuration. It does not
change colors, remap addresses, write SPD, save flash, or sweep other addresses.
`--scan` remains passive and skips active RAM probing. `--probe-ram` is an
explicit identity-only diagnostic (no colors) and exits without opening the UI.

SMBus operations hold `Global\Access_SMBUS.HTP.Method` across a full ENE read or
frame. The primary bus selection is restored afterwards, even on transfer errors.
The lock wait is bounded to 250 ms; module transfer timeouts are upstream-owned.
Programs that ignore the shared mutex can still conflict; use the existing
startup conflict dialog to choose which software may control RGB hardware.

## Updating the dependency

The runtime is loaded from the official installed location on each application
start; updating that installation takes effect after restarting Lighting Studio.
The current ABI guard accepts major version 2 and verifies required entry points.
It is a compatibility check, not an online latest-version check.

The SMBus module is separately pinned to 0.2.11, its size and SHA-256. Check
https://github.com/namazso/PawnIO.Modules/releases/latest and https://pawnio.eu for
upstream updates before qualifying a new application release. New modules require
an explicit source/license, ABI and protocol review, updating the pinned binary,
source archive and integrity check together, followed by Release/Debug and RAM
regression checks. Physical output must be requalified separately. No automatic
download, installation, driver replacement or online update checker is implemented.
