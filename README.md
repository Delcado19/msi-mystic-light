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

- [`CONTINUE.md`](CONTINUE.md) — implementation status, validation, and next steps.
- [`docs/SIGNALRGB_REPOSITORY_RESEARCH.md`](docs/SIGNALRGB_REPOSITORY_RESEARCH.md) — pinned public-repository research on components, plugins, add-ons, and MSI discovery.

## Standalone app implementation

The app now loads local driver plugins and independent device profiles; see
[`docs/HARDWARE_MODULES.md`](docs/HARDWARE_MODULES.md) for the module contract,
adding hardware support, profile schema, and current limitations. The first
module is `msi.gen1`, with an explicit B850 Edge TI board profile.
Data-only component definitions and validated channel-chain mappings are also
implemented. The bundled generic 8-LED strip demonstrates adding a product
without changing a driver. Use `--component-dir` for another local catalog;
repository downloads and a hardware channel assignment UI remain pending.
An offline editor now provides separate Layout/Effects views, movable catalog
components, Static Color/gradient/shape/image layers, soft masks, transparency/blend controls, motion/color animation, seeded particles/trails, direct canvas transforms/cropping,
undo/redo and project save/load. See [`docs/LIGHTING_EDITOR.md`](docs/LIGHTING_EDITOR.md).
Run `build/ninja/Release/msi_mystic_light.exe --editor` for offline editing.

The first implementation increment provides a C++17/CMake project, a Qt6
application scaffold, and pure encoders for the 290-byte `0x50` and 727-byte
`0x51` reports. Qt Test covers byte layouts, option flags, transient versus
persistent settings, LED count boundaries, and invalid input rejection.
The executable initializes Qt, checks potential process conflicts, validates
local modules/profiles/components and opens the editor. Diagnostic commands
`--inspect` and `--scan` exit after their checks. The app
does not yet provide a tray UI, open HID devices, or send any reports.
Hardware support remains unverified for this implementation.

Build from an x64 Visual Studio developer prompt with CMake, Ninja, and a
shared Qt6 installation available (adjust the Qt path for your machine).
Windows builds require matching Release and Debug WebP image plugins. If Qt
ImageFormats is installed with the SDK, skip the build-webp command below.
Otherwise the script downloads the pinned, unmodified Qt ImageFormats 6.8.3
source into ignored build directories and builds both plugins. The current
fallback is specifically for Qt 6.8.3; use matching SDK plugins for other Qt versions:

```bat
set "PATH=C:\Qt\6.8.3\msvc2022_64\bin;%PATH%"
powershell -NoProfile -ExecutionPolicy Bypass -File tools/build-webp.ps1 -QtPrefix C:/Qt/6.8.3/msvc2022_64
cmake -S . -B build/ninja -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build build/ninja
ctest --test-dir build/ninja --output-on-failure
```

The HID transport library and test fake are implemented separately from the
application. Transport tests replace HIDAPI functions at link time, covering
initialization failure, failed opens, repeated close/reopen, invalid buffers,
short writes, and multiple wrapper lifetimes without touching hardware.
The app links the HIDAPI runtime for optional descriptor enumeration; it does
not construct or open a device transport.

HIDAPI headers and a library are now required to build the transport. If they
are not on CMake's search path, add `-Dhidapi_ROOT=C:/path/to/hidapi-sdk`
to the configure command; the root should contain `include/hidapi.h` and
`lib/hidapi.lib` (or `x64/hidapi.lib` on Windows). The validated local SDK is
the existing `../OpenRGB/OpenRGB/dependencies/hidapi-win` directory; no
OpenRGB application code is linked or copied. The Windows build copies the matching HIDAPI DLL beside the executable. Qt must remain dynamically linked.

Original project code is licensed under GPL-2.0-or-later; see [LICENSE](LICENSE).
Qt remains under its own license; see [third-party notices](THIRD_PARTY_NOTICES.md) for the WebP plugin source, build/replacement instructions and license texts. Research references do not relicense vendor
software or include it in this project.

## Research documents

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

Editor design reference: [SignalForge review](docs/SIGNALFORGE_EDITOR_RESEARCH.md).

Use `--scan` to list HID endpoints, system board identity and matching device
profile candidates. This is read-only discovery: no device handles are opened
and no lighting reports are sent. Ambiguous candidates remain unselected.

Additional references: [SignalRGB GitHub topic triage](docs/SIGNALRGB_TOPIC_RESEARCH.md).

Normal startup now checks known competing RGB processes and offers Terminate all,
selected Terminate, Ignore and Cancel. No process is stopped automatically.
Service-owned helpers require manual shutdown. Use `--inspect` for catalog-only
validation; `--scan` remains read-only and bypasses this pre-connection gate.
Windows builds include the required Qt DLLs and Windows platform plugin beside
the executable; a packaged installer is not implemented yet.
