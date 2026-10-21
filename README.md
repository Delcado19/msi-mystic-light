# MSI Mystic Light — Reverse Engineering

Reverse-engineering notes on the official MSI Mystic Light software
(`C:\Program Files (x86)\MSI\MSI Center\Mystic Light`), produced with the
`reverse-engineer-anything` skill/REA MCP server + `ilspycmd` (.NET
decompiler).

**Goal:** foundation for a standalone app that can drive MSI RGB hardware
without MSI Center.

**No vendor code in this repo.** The decompiled `.cs` files only live in the
local scratchpad directory of the respective Claude Code session (temporary,
not part of this repo). Documents here describe behavior/byte layouts in
our own words and cite file:line in the original source as evidence — they
do not reproduce it at length.

## Contents

- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — overall architecture:
  Windows service, LEDKeeper2.exe, MysticLight_AllDevice.dll, MLModule.dll,
  SyncData.dll.
- [`docs/NETWORK_PROTOCOL.md`](docs/NETWORK_PROTOCOL.md) — local network
  interfaces (HTTP SDK, UDP pairing/sync, SignalRGB UDP bridge).
- [`docs/MB_800_HID_PROTOCOL.md`](docs/MB_800_HID_PROTOCOL.md) — USB HID
  protocol of the "800-series" mainboard generation (VID `0x0DB0`/PID
  `0x0076`, report `0x50`/`0x51`/Gen2) — counterpart/correction to
  `OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md`.
- [`docs/OPENRGB_UPSTREAM_DRAFT.md`](docs/OPENRGB_UPSTREAM_DRAFT.md) — draft
  comment for the OpenRGB issues #4935/#5429. **Not posted yet, local draft
  only.**

## Related repos

Part of a repo cluster under `Software_Projects/` (see also
`OpenRGB/OpenRGBResearch/CONTINUE.md`):

- `../OpenRGB/OpenRGB` — our own OpenRGB fork with `MSIMysticLight290Controller`
- `../OpenRGB/OpenRGBResearch` — raw packet captures, analysis scripts

## License

This project is licensed under GPL-2.0-or-later (see [`LICENSE`](LICENSE)) as of
this change.

The Roccat, Razer and SteelSeries peripheral drivers are adapted from
[OpenRGB](https://openrgb.org), which is also licensed GPL-2.0-or-later and is
the upstream source for those drivers.

## Vulcan keyboard layout

The Vulcan 100/120 Aimo canvas uses the OpenRGB ISO (DE/UK/FR) physical
key positions mapped to hardware LED addresses, including the `< >`, Windows,
and Right Control keys. The 144-slot wire format remains unchanged; unused
slots do not extend the physical bounds. ANSI/JIS layout detection is not
implemented. The layout correction still requires visual hardware verification.

On Windows, building `MsiMysticLightApp` also runs `windeployqt` to place the
matching Qt runtime DLLs and platform plugins beside the executable. Launch
the app from that output directory; copying the EXE alone is insufficient.

The Lighting Studio window opens at startup; closing it keeps the tray app running.

## Board lighting limitations and defaults

On the MPG B850 Edge TI WiFi, IOShield supports firmware-native effects only:
previous hardware checks found no visible response to per-LED report 0x51.
The UI no longer offers Canvas for IOShield and clamps older assignments to
Native. New board channel entries start with steady white instead of black;
existing saved colors, including intentional black/off, remain unchanged.
A successful HID write alone does not prove that a physical LED changed.

## Lighting Studio interface

Lighting Studio uses a device sidebar, a fitted spatial preview, and a focused
inspector. Select a device in the list or on the canvas; drag zones to arrange
their physical layout. The inspector exposes only controls relevant to the
selected lighting mode. Large color buttons show the current hex value.

Pause preview stops only on-screen animation. Hardware effects continue.
The preview also pauses while hidden and respects the Windows client-animation
preference. It is not a connectivity indicator or an exact firmware simulator.
See [the UI audit and design decisions](docs/UI_REDESIGN_2026.md).

## Components, keys and custom effects

Motherboard headers now accept ordered fan/strip components. Open **Edit LED colors...** to view the Vulcan ISO keys, mouse zones, or the selected fan chain.
Assign fixed region colors or use the temporary white-only identification test.
The desktop canvas has no decorative case/desk outlines.

The desktop preview uses OpenGL composition; serialized USB output runs on a
separate hardware thread. Effects use continuous HSV/gradient interpolation.
**Effects > New effect** provides nine pattern types: drag a pattern onto
the live preview, drag gradient stops, then edit colors with the wheel or exact
RGB/hex inputs. Saved recipes work as shared or individual effects; existing
profiles and color-only recipes remain compatible. The standard selectors have
21 choices. SignalRGB HTML scripts are not supported by this native engine.

See [component configuration and calibration](docs/COMPONENTS_AND_EFFECTS.md)
for wire addresses, paired-region templates, mirrored-hub limits and validation.

Hardware configuration includes the locally installed SignalRGB component catalog
when available, custom fan/strip definitions and JSON import. The desktop supports
component move/resize/rotation; **Arrange components / LED groups** supports LED
point dragging, software groups, Line/Circle/Ellipse/Rectangle layouts and Reset.
Removing the final configured component clears its Canvas output. Preview and HID
sampling share a single coordinate transform and honor component wire mappings.
The Vulcan inspector offers direct wire-address calibration; OpenRGB reference
positions alone do not prove a physical key mapping on every unit.


Lighting Studio: use **Hardware** to add/remove actual fans or strips per
header and **Effects** to create or edit saved palettes. The desktop shows the live spatial effect underneath movable hardware shapes.
Shared Canvas output samples that field; independent effects can run along LED
addresses. Add products through the channel-scoped picker and remove them on
their component cards. See
[component and effect workflow](docs/COMPONENTS_AND_EFFECTS.md) for wiring limits
and the remaining physical Vulcan verification.


The workspace has three primary sections: Layout, Effects, and Hardware. Effect
creation/editing stays in Effects. Layout uses bundled generic SVG device artwork
and actual key/LED-zone metadata; detailed colors open from the selected-device
inspector. The product catalog uses installed SignalRGB component JSONs plus generic
presets; the fallback Vulcan ISO map is based on OpenRGB. Workspace profiles remain
local app settings, and generic artwork is not a claim of exact product imagery.
