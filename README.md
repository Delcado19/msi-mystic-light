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
