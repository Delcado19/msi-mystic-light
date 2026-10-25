# Third-party runtime components

Qt is dynamically linked and remains replaceable. The WebP image format plugin
is unmodified Qt ImageFormats v6.8.3 (commit
3b6b1fb96041f1bc85a8f42593bef2ee7bcdca6c), built with its bundled libwebp.
Qt plugin copyright: Copyright (C) 2016, 2019 The Qt Company Ltd.
Qt code is used under LGPL-3.0-only; license texts and libwebp copyright/patent
notices are included in third_party/licenses and copied beside the application.

Corresponding Qt ImageFormats source (including plugin and bundled libwebp):
https://code.qt.io/cgit/qt/qtimageformats.git/tree/?id=3b6b1fb96041f1bc85a8f42593bef2ee7bcdca6c

Build/replacement instructions: tools/build-webp.ps1 and README.md. The image
plugin can be replaced in the application's imageformats directory; the
application does not enforce signatures or checksums on replacement plugins.
Qt Core/Gui/Widgets, JPEG/GIF and the Windows platform plugin are supplied by
the user's matching dynamically linked Qt SDK, under Qt's respective licenses.
HIDAPI is separately supplied by the configured HIDAPI installation; see README.
Research references do not add runtime code or change these licenses.

Optional component metadata is read from a user's existing local VortxEngine
installation for compatibility. Only validated selected geometry and LED mapping
are embedded in the user's project. No SignalRGB application code, scripts,
effects, images or binaries are distributed or executed by this integration.
Product names identify compatible hardware; no affiliation is implied.

Restored peripheral protocol encoders retain their GPL-2.0-or-later headers and
OpenRGB provenance from the original Lighting Studio source worktree. They cover
Roccat Kone/Vulcan, Razer Basilisk V3 and SteelSeries Apex 3. The Vulcan ISO geometry
also derives from OpenRGB's RoccatVulcan120AimoLayouts, as documented in the original
RoccatVulcanDriver. See LICENSE and the detailed source-header attribution.

The restored AeroBackend derives from the original Lighting Studio
GigabyteAeroDriver, with GPL-2.0-or-later provenance to OpenRGB's
GigabyteRGBFusion2BlackwellGPUController and i2c_smbus_nvapi. The NVIDIA
NVAPI driver is not bundled. Source headers retain their attribution.
No OpenRGB executable, PawnIO driver/library or RAM helper is distributed.

RAM support directly calls the installed official PawnIOLib ABI. ENE register
encoding and AMD SMBus transfer layout follow OpenRGB's GPL-2.0-or-later
ENESMBusController/ENESMBusInterface and i2c_smbus_pawnio references; source
comments retain provenance. The unchanged signed SmbusPIIX4.bin is from
namazso/PawnIO.Modules 0.2.11 (Steve-Tech and upstream contributors), under
LGPL-2.1-or-later. Its corresponding source archive and license are included
under third_party/pawnio and third_party/licenses. See third_party/pawnio/README.md.
The official installed PawnIO binaries have separate upstream distribution terms;
this app neither redistributes nor silently installs them.

## Hardware artwork

Fan artwork: "Computer fan" by Delapouite (https://delapouite.com),
https://game-icons.net/1x1/delapouite/computer-fan.html, licensed CC BY 3.0:
https://creativecommons.org/licenses/by/3.0/ . Original SVG from game-icons/icons
revision 82d948812bfe3f269ef8f731dcdb07b08160edc4, preserved as
assets/hardware/fan-source.svg. Changes in fan.svg: remove opaque background,
add a square housing and use muted colors. No endorsement is implied.
Source URL and SHA256 are recorded in assets/hardware/sources.json.
Artwork credits are also available in the application's Help menu.
Other technical hardware SVGs are original Lighting Studio drawings under
GPL-2.0-or-later. All runtime SVGs are compiled into the application; no remote
images are fetched. Qt SVG is dynamically linked under the same LGPL-3.0 terms
as the other Qt runtime libraries and is replaceable as Qt6Svg.dll.

Keyboard schematic key positions follow the shipped Vulcan ISO component geometry,
whose OpenRGB-derived mapping provenance is described above. Fullsize models share this registered SVG; per-key colored markers continue to
use the saved LED positions, while zone devices retain their real zone markers.
