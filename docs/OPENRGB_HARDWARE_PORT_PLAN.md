# Porting OpenRGB/SignalRGB hardware support into this app — motherboard tier

Date: 2026-10-29. Scope: the 22 OpenRGB controllers that set
`DEVICE_TYPE_MOTHERBOARD` (`Controllers/*`, current master `aac7e551`,
local checkout `build/research/openrgb`, gitignored). Decision context:
pack in hardware support from both OpenRGB and SignalRGB as modules that
only activate for detected hardware (same pattern as the existing
`msi.gen1`/`modules/msi-gen1`), starting with the motherboard category.
This project is GPL-2.0 (`LICENSE`), same as OpenRGB (GPL-2.0-or-later) —
no license conflict for porting OpenRGB source. SignalRGB has no found
LICENSE file; its plugins stay facts-only, never copied as code.

## Transport classification (21 real controllers + 1 debug, excluded)

| Controller | Detector macro | Transport | Notes |
|---|---|---|---|
| MSIMotherboardController (112/162/185/761 sub-controllers) | `REGISTER_HID_DETECTOR_PU` | HID feature reports | **Our board (`0DB0:0076`) reconciled below — already covered by `msi.gen1`** |
| LenovoMotherboardController | `REGISTER_HID_DETECTOR_PU` | HID feature reports | Not yet inspected in depth |
| JGINYUEInternalUSBController / V2 | `REGISTER_HID_DETECTOR[_IPU]` | HID | Not yet inspected |
| HPOmen30LController | `REGISTER_HID_DETECTOR` | HID | Not yet inspected |
| AsusAuraUSBController (mainboard variant only) | `REGISTER_HID_DETECTOR[_PU]` | HID | Not yet inspected |
| ASRockPolychromeUSBController | `REGISTER_HID_DETECTOR` | HID output+input reports (`hid_write`/`hid_read`) | **Inspected in depth below — blocked** |
| GigabyteRGBFusion2USBController | `REGISTER_HID_DETECTOR_PU` | HID | Not yet inspected |
| MSIRGBController | `REGISTER_DETECTOR` (custom) | Unclear, not plain HID/I2C | Not yet inspected |
| GigabyteSuperIORGBController | `REGISTER_DETECTOR` (custom) | Super I/O port I/O | Out of scope for now — no portable abstraction exists in this app at all |
| GigabyteRGBFusionController (legacy) | `REGISTER_I2C_DETECTOR` | SMBus | SMBus tier — see open question |
| GigabyteRGBFusion2SMBusController | `REGISTER_I2C_DETECTOR` | SMBus | SMBus tier |
| EVGASMBusController | `REGISTER_I2C_DETECTOR` | SMBus | SMBus tier |
| ASRockSMBusController (Polychrome V1/V2, ASRRGB) | `REGISTER_I2C_DETECTOR` | SMBus | SMBus tier |
| ENESMBusController (+ ROGArion) | `REGISTER_I2C_DETECTOR` + `REGISTER_I2C_PCI_DETECTOR` | SMBus | **Direct overlap with our own PawnIO RAM path — see below** |
| FrameworkDesktopController | none (Linux-only, EC/sysfs) | Linux sysfs, not HID/SMBus | Out of scope — this app has no Linux EC-sysfs transport at all; would need its own new backend, unrelated to the HID/SMBus work here |
| DebugController | — | — | Excluded, not real hardware |

The SMBus/Super I/O tier (9 of 21) needs a widened PawnIO broker beyond
the currently qualified `1022:790b`/port 0/`0x71`/`0x73` scope, which
`PAWNIO_SERVICE_RESEARCH_2026-10-27.md` explicitly says not to do without
review — **open question, not decided here.**

## Overlap deep-dive 1: our board (`0DB0:0076`) — resolved, no conflict

OpenRGB's `DetectMSIMotherboardControllers` (`MSIMotherboardControllerDetect.cpp:37-130`)
does **not** statically map `0DB0:0076` to one sub-controller. It opens the
HID device and reads report `0x52` to get a byte count, then branches:

1. Length matches `FeaturePacket_185`/`162`/`112` (report `0x52`-family,
   zone-table layout `j_rgb_1, j_rainbow_1, j_corsair_1, ...`) → one of the
   112/162/185 controllers.
2. Otherwise (the `else` branch, our board's actual path): sends a 290-byte
   report `0x50` "enable reading" packet, then **probes report `0x51` with
   a hardcoded length of 761** (`MSIMotherboardControllerDetect.cpp:85-99`
   — this is the same upstream bug already reported on GitLab #4935/#5429
   and reconfirmed unfixed in the previous research pass) → constructs
   `MSIMotherboard761Controller`.

Cross-checking all three independent sources for this exact VID:PID:

- **Our vendor-driver decompile** (`NETWORK_PROTOCOL.md`/GitLab comment):
  report `0x50` (290B, 18×16B zone records) + report `0x51` (727B per-LED,
  `Gen1_Adv`).
- **OpenRGB's fallback branch**: same `0x50`/`0x51` pair, same byte counts
  (290/727, modulo its probe-length bug), landing in `MSIMotherboard761Controller`.
- **SignalRGB's `MSI_185Byte_Controller_0x0DB0.js`**: despite the filename
  (a legacy/misleading name — compare the already-documented
  `FeaturePacket_PerLED_761` struct that is actually 727 bytes, same class
  of stale vendor/community naming), its actual code reads report `0x50`
  zone state and uses an explicit per-board profile table
  (`SIGNALRGB_REPOSITORY_RESEARCH.md`, "Exact MSI B850 findings").

**All three agree on the real protocol** (`0x50`/`0x51`, 18 zones, 727-byte
per-LED). `msi.gen1` already implements exactly this. No change needed for
our own board; the OpenRGB/SignalRGB "185-byte" naming is a false lead from
stale file/struct names, not a competing protocol.

**New finding, separate from our board**: the `112`/`162`/`185` families
are **real, different MSI board generations** (other VID/PIDs in the same
`REGISTER_HID_DETECTOR_PU` list, e.g. `MS_1720`, `MS_7B12`...`MS_7B93` —
dozens of model-specific PIDs all routed through the same probe cascade).
Porting these would extend support to other MSI boards beyond ours — but
OpenRGB selects the family by a **read-back length probe**, not a static
PID table, which this app's architecture cannot currently do (see blocker
below). A static PID→family lookup table would need to be built from
OpenRGB's own PID registration list as a workaround, each entry unverified
until tested.

## Overlap deep-dive 2: ENE SMBus — not yet completed

`Controllers/ENESMBusController` is SMBus-tier (`REGISTER_I2C_DETECTOR`/
`REGISTER_I2C_PCI_DETECTOR`), the same bus class as our own PawnIO RAM
path (`AMD 1022:790b`, port 0, RAM endpoints `0x71`/`0x73`, ENE
`AUDA0-E6K5-0101`, direct-mode sequence `0x8020`→`0x80A0`→`0x8100`,
documented in `HARDWARE_MODULES.md`). **Not yet read in this pass** — the
register-sequence comparison advisor flagged is still open. Do before any
SMBus-tier module is written, since a mismatch here is exactly the kind of
thing that caused the `FeaturePacket_PerLED_761`/`761`-probe bug.

## ASRock Polychrome USB — implemented (2026-10-29, same session)

Ported as `modules/asrock-polychrome` (`asrock.polychrome-usb`, API 3),
protocol in `src/protocol/AsrockPolychromeProtocol.{h,cpp}`, profile
`profiles/asrock-polychrome-usb.json`, tests in
`tests/modules/test_asrock_polychrome.cpp` (ctest `asrock_polychrome`, all
green; `driver_modules`, `app_scaffold`, `components`, `peripheral_reports`,
`hid_backend` re-run clean afterward). The real app loads it:
`Loaded driver modules: asrock.polychrome-usb, lighting.auxiliary, msi.gen1,
lighting.peripherals` / `Available profile: ASRock Polychrome USB`.

**Scope implemented**: the 8 whole-zone single-color "native mode" writes
(`SET_ZONE` 0x10, zone 0-7, 15 modes off/static/breathing/strobe/
spectrum-cycle/random/music/wave/spring/stack/cram/scan/neon/water/rainbow —
fixing upstream's own copy-paste bug where "Music" reused Random's 0x05
instead of the header's `0x06`). Color order fixed non-swapped (G,R,B),
matching upstream's own safe default. Every write is deduplicated against
cached per-zone state before sending, because upstream's own doc comment
says this hardware "will save with each update" — there is no software
volatile/persist distinction here like MSI's trailing store byte, so an
unthrottled continuous software effect would wear the flash.

**Scope deliberately NOT implemented, by design, not oversight**:

- RGSwap (per-header color-order override): skipped entirely; always
  non-swapped. The one real discrepancy found between OpenRGB and
  SignalRGB (config read-back offset 4 vs 5) was never resolved against
  real hardware, so reading/writing this config safely isn't possible yet.
- Direct/per-LED "Chroma" streaming (report sequence `0x10`/`0xE3`/`0xE4`):
  needs a combined cross-channel buffer (all direct-mode zones' current
  colors resent together on every change, per both OpenRGB's
  `WriteAllZones` and SignalRGB's `concatRGBData`) that this driver does
  not have, and its flash-persistence behavior under continuous real-time
  updates is unverified. `IDriver::pushColors` always returns `false` for
  this module.
- `IDriver::discoverLeds()` uses the interface default (`Unknown`); the
  real per-zone LED-count read (`ReadConfigTables`) was not ported because
  of the same unresolved offset-4-vs-5 discrepancy.

**Still unqualified**: no ASRock Polychrome board is owned by this project;
the profile and protocol are shipped per this app's existing precedent for
unverified hardware (`HARDWARE_MODULES.md`: "Adding a profile is not
evidence that the hardware has been tested"), gated behind the existing
report-consent flow. Do not claim tested support in user-facing docs.

## Lenovo motherboard — implemented (2026-10-29, same session)

Ported as `modules/lenovo-motherboard` (`lenovo.motherboard`, API 1 —
feature reports only, no read needed), protocol in
`src/protocol/LenovoMotherboardProtocol.{h,cpp}`, profile
`profiles/lenovo-motherboard.json`, tests in
`tests/modules/test_lenovo_motherboard.cpp` (ctest `lenovo_motherboard`).
Full suite re-run clean after both ports: 21/21 ctest passing.

Simpler and safer than ASRock Polychrome: single VID:PID (`0x17EF:0xC955`,
usage page `0xFF89`, usage `0xCC`), two fixed zones (`CpuFan`=0x12,
`RearFan`=0x11, one representative color each), 7 native modes
(static/sparkle/breathing/wave/specter/rainbow-wave/random), and upstream's
own metadata marks it `@save :x:` — no flash-persistence behavior to worry
about, unlike ASRock. Each `setNativeMode` call sends **two** 64-byte
feature reports with the same fields at different offsets (`[1..7]` and
`[33..39]`); upstream sends both unconditionally and does not document why
a second write exists, so this port does the same rather than guessing
which one is "real."

**Not ported/verified**: the direction of upstream's documented 1-4
speed/brightness ranges (which end is fastest/brightest) is undocumented
upstream and unverified here; this port clamps our own 0-2/0-5 scales
linearly into 1-4 as a reasonable default, not a confirmed mapping.
Unqualified on real hardware, same as ASRock Polychrome — no Lenovo board
is owned by this project.

## Resolved: `IHidDevice` read capability added (API 3)

`src/hid/IHidDevice.h` originally only exposed `sendFeatureReport` and
`writeOutputReport` — no read method existed. User-approved same session:
added `getFeatureReport`/`readInputReport` (API 3; additive vtable slots,
default `return 0`, existing API 1/2 modules unaffected). Implemented in
`HidApiDevice` (`hid_get_feature_report`/`hid_read_timeout` — a bounded
1s wait, unlike hidapi's default indefinite block) and `FakeHidDevice`;
`ModuleRegistry` accepts `apiVersion: 3`. Tests: `hid_backend` ctest
(`reads`/`fakeReads` slots). The ASRock Polychrome module (below) uses
`readInputReport` to drain the ack after each write.

This does **not** mean every read-dependent controller is portable now —
the interface limitation is gone, but per-controller discovery logic still
needs its own correctness verification before porting. Both controllers
inspected in depth need reads for discovery specifically:

- **ASRockPolychromeUSBController**: `ReadConfigTables()` does
  `hid_write` (request) + `hid_read` (response) to learn **actual**
  per-header LED counts and RGB-swap config from the hardware — this is
  real discoverable topology, not table-driven like MSI. Also calls
  `WriteRGSwap()` **during construction** (`SetDeviceInfo()`,
  `ASRockPolychromeUSBController.cpp:86-192`), i.e. it writes a device
  config register before any explicit connect/report-consent step — this
  app's existing rule ("Creating a driver must not send reports",
  `IDriverModule.h:44`) would be violated by a direct port; the RGSwap
  write would need to move to the driver's first explicit `pushColors`/
  `setNativeMode` call instead.
- **MSI 112/162/185 family selection**: needs the report-`0x52` read-back
  probe described above just to know which sub-protocol a given board
  uses — no static table currently exists to avoid it.

Discovery for both remains unported: ASRock's `ReadConfigTables` because of
the unresolved offset-4-vs-5 discrepancy against SignalRGB (below), and the
MSI 112/162/185 family selection because no static PID→family table has
been built yet from OpenRGB's registration list.

## What's portable right now

Controllers whose protocol is write-only and whose LED/zone capacities are
static/table-driven (like `msi.gen1`) are the simplest port. Controllers
needing read-back for *discovery* (ASRock's `ReadConfigTables`, MSI's
112/162/185 family-selection probe) are possible now that `IHidDevice` can
read, but each still needs its own hardware-verified correctness check
before the read path is actually used — ASRock's config read was
deliberately left unported this session for exactly that reason (see
"ASRock Polychrome USB — implemented" below). Lenovo is now ported (above). None of the other 5
inspected-by-name HID controllers (JGINYUE x2, HP Omen, Gigabyte
Fusion2 USB, Asus Aura mainboard) have been read in enough depth yet to
know which category they fall into — that is the immediate next step.

## Three-way cross-check: SignalRGB plugin tree added as a third source

Installed plugin tree: `%LOCALAPPDATA%\VortxEngine\app-2.5.80\Signal-x64\Plugins`
(same installation already used for `ENE_RAM.js` in `RAM_ACCESS_RESEARCH.md`).
Read-only inspection of `.js` source; nothing executed, no device touched.

### Our board (`0DB0:0076`) — triple-confirmed, including the shared bug

`Plugins/Msi/185 Byte Motherboard Controller/MSI_185Byte_Controller_0x0DB0.js`
(despite the misleading folder/file name) contains a `MysticLight761` class
with `standardPerledPacket` at report **`0x50`** (290 bytes, 18×16B records,
trailing store byte) and per-LED writes at report **`0x51`**
(`device.send_report([0x51, 0x09, area_hi, area_lo, 0x00, 0x00, ledCount]
.concat(colors), 727)`) — exact match to our vendor decompile and to
OpenRGB's `MSIMotherboard761Controller` fallback path. Its board library
contains `"MPG B850 EDGE TI WIFI (MS-7E62)"` **by name**, with
`JPIPE1: "IO Shield", LedCount: 11` and `JRGB1: "12V RGB Header 1",
LedCount: 1` — matching our own documented zone/LED counts exactly.

**The `761` probe-length bug exists independently in SignalRGB too**:
`checkProtocolVersion()` sends `device.send_report([0x51], 761)` — the same
stale length as OpenRGB's hardcoded probe and the same community-sourced
struct name. Three independent implementations (ours, OpenRGB's, SignalRGB's)
all carry the same legacy "761" number for an actually-727-byte report. This
raises confidence that `761` really is just a stale historical label, not a
real protocol variant we're missing — but also means neither upstream project
is a reliable source to "fix" it against; our own decompile remains the
primary source of truth.

`readDeviceZones()`'s own comment independently confirms the already-documented
finding a third time: reading `0x50` "doesn't give led counts" — only mode/zone
config, not attached topology.

**New opportunity, not yet acted on**: this file's `Library` table has
ready-made zone/LED-count data for roughly 50 additional MSI boards (B840,
B860, Z790, X870, X870E, Z890 families), all using the same `0x50`/`0x51`
protocol our qualified `msi.gen1` driver already implements. These would be
new **profiles** under the existing driver, not new modules or protocol code —
low-risk, since the transport/protocol is already qualified; each profile is
still individually unqualified/untested hardware per our existing policy.
Not yet added; flagging as a concrete, cheap next increment.

### ASRock Polychrome USB (`26CE:01A2`) — mostly confirmed, one open discrepancy

`Plugins/AsRock/Asrock_Motherboard_Controller.js` targets the identical
VID:PID. Its ARGB per-LED streaming matches OpenRGB byte-for-byte at the
structural level: initial packet `[0x00, 0x10, 0x00, 0xFF, 0xE3, ...]` then
54 bytes of color data at offset 9, continuation packets
`[0x00, 0x10, 0x00, 0xFF, 0xE4, ...]` with 57 bytes at offset 5 — identical
header byte positions and chunk sizes to OpenRGB's `WriteAllZones`
(`usb_buf[0x01]=SET_ZONE, usb_buf[0x03]=0xFF, usb_buf[0x04]=INITIAL_CHUNK/
SEND_CHUNK`, same 9/54 then 5/57 offsets). High confidence this part of a
future port is correct. SignalRGB hardcodes 15 continuation packets ("I'm
still not a fan of hardcoding things, but this works on all of the boards" —
its own comment); OpenRGB computes the count from actual LED data length,
which is the more correct approach to port.

Also found a **second, non-per-LED write path** neither previously
documented from OpenRGB alone: `grabZones()` sends
`[0x00, 0x10, 0x00, 0x07, 0xE2].concat(RGBData)` — one packet with all 8
zones' colors (24 bytes), using `0xE2` ("Chroma"/whole-device mode, a value
OpenRGB's `GetZoneConfig` only ever *reads* to detect current mode, never
*writes*). OpenRGB's `WriteZone` instead sends one packet per individual
zone (`zone_type` 0–7). Both appear to be valid hardware paths distinguished
by the mode byte; SignalRGB's batched write is more efficient. Worth
considering for a native port instead of 8 separate packets, but unverified
until tested against real hardware.

**Open discrepancy, unresolved — flagged as Unknown, not decided here**:
config read-back byte offset. OpenRGB's `ReadConfigTables()` reads the
max-LED-count table and the "header1"/comparison byte at response **offset
4** (`&usb_buf[0x04]`, `usb_buf[4]`). SignalRGB's `ReadConfig()`/`LEDConfig()`
reads the equivalent fields at response **offset 5** (`.slice(5, 13)`,
`[...][5]`). Both send the same request (`[0x00, 0x14, 0x00, ConfigId]`).
This could be a real off-by-one in one implementation, or a difference in
how `device.read()` (SignalRGB's host-provided call) vs raw `hid_read()`
handle a leading report-ID byte — not resolved by source reading alone.
**Do not port `ReadConfigTables`/`LEDConfig` without testing both offsets
against real hardware first.**

### 162-byte MSI family — corroborated, not conflicting

`Plugins/Msi/162 Byte Motherboard Controller/` has five per-model files
(`..._0x1720.js`, `_0x7B17.js`, `_0x7B18.js`, `_0x7B85.js`, `_0xB926.js`),
each matching a specific PID already in OpenRGB's `MSI_USB_VID` (`0x1462`)
registration list (`MS_1720`, `MS_7B17`, `MS_7B18`, `MS_7B85`). Independent
corroboration that OpenRGB's family assignment for those exact PIDs is
correct — not yet read in protocol-byte detail (next step, not done here).

### ENE SMBus (RAM) — register map confirmed 3-way, detection method rejected

Three independent sources for the exact same ENE register map: our own
PawnIO path (`HARDWARE_MODULES.md`), OpenRGB's `ENESMBusController`
(`Controllers/ENESMBusController/ENESMBusController.h`), and SignalRGB's
`Plugins/Asus/ENE_RAM.js`.

**Register map — exact agreement, all three**:

| Register | Our doc | OpenRGB | SignalRGB |
|---|---|---|---|
| Direct-mode select | `0x8020` | `ENE_REG_DIRECT = 0x8020` | `DirectMode: 0x8020` |
| Apply (volatile) | `0x80A0` | `ENE_REG_APPLY = 0x80A0`, written with `ENE_APPLY_VAL = 0x01` | matches |
| Direct color buffer (V2) | `0x8100` | `ENE_REG_COLORS_DIRECT_V2 = 0x8100` | `ColorCtlV2: 0x8100` |
| Firmware/version string | `AUDA0-E6K5-0101` (V2) | `strcmp(device_version, "AUDA0-E6K5-0101")` → `ENE_REG_COLORS_DIRECT_V2` | `"AUDA0-E6K5-0101": "V2"` |
| Flash/persist value | never sent (`0xAA`) | `ENE_SAVE_VAL = 0x AA`, written to `ENE_REG_APPLY` only from an explicit `SaveMode()` call, never from the direct-color path | not inspected for this exact value, but the same apply-register pattern is present |
| Color byte order | R-B-G, volatile only | `ENESMBusController::SetAllColorsDirect`: `color_buf[i]=R, [i+1]=B, [i+2]=G` | not inspected in this pass |

This fully corroborates our own PawnIO register sequence (`0x8020=1` →
`0x80A0=1` → `0x8100=colors`) a third time, independently. No change needed
to our implementation.

**Detection method — both reference sources use exactly the unsafe pattern
our project has already decided against; do not port their detection as-is**:

- OpenRGB's `DetectENESMBusDRAMControllers`
  (`ENESMBusControllerDetect.cpp:149-222`) scans **16 SMBus addresses**
  per bus (`0x70-0x77, 0x4F, 0x66-0x67, 0x39-0x3D`) with
  `i2c_smbus_read_byte`/`read_byte_data` quick-probes, reads a register
  range `0xA0-0xB0` to "fingerprint" a candidate device, and — if it finds
  an unmapped stick on `0x77` — **actively writes new slot/address
  assignments** via `ENERegisterWrite(..., ENE_REG_SLOT_INDEX, slot)` /
  `ENERegisterWrite(..., ENE_REG_I2C_ADDRESS, address << 1)` before even
  constructing a controller object. `IF_DRAM_SMBUS` gates this to
  `AMD_FCH_SMBUS_DEV = 0x790B` (`pci_ids.h:21,1068-1080`) among others —
  **this is our exact chipset** (`AMD_VEN 0x1022` + `0x790B`), not a
  different-platform concern.
- SignalRGB's `ENE_RAM.js` does the same thing independently: starts at
  `0x77`, uses `bus.WriteQuick(address)` (a quick-command bus probe, the
  same primitive our advisor flagged — "the default quick-write probe can
  corrupt SPD or EEPROM") to find a free address, then "Attempting to
  remap Ram addresses" (`ENE_RAM.js:755-783`).

Both independent implementations treat scan-and-remap as the normal,
required way to use this chip family on boards with an unmapped/shared
starting address. Our own project's explicit policy (`PAWNIO_SERVICE_RESEARCH`:
"Future devices need explicit protocol qualification, not an arbitrary scan
sweep"; `HARDWARE_MODULES.md`: "No remapping, SPD or flash writes are
exposed") rules this out. **Verdict: the ENE register protocol is safe to
keep using exactly as we do; the detection/remap method from both reference
projects must not be ported.** Any additional ENE RAM module beyond our
currently qualified fixed `0x71`/`0x73` pair needs its own explicit,
narrow qualification per board — never a generic address sweep — consistent
with how `msi.gen1` profiles are added today.

### Not yet cross-checked this pass

- `Plugins/Asus/Motherboard Controllers/AsusSMBusMotherboardController.js`
  against OpenRGB's legacy `GigabyteRGBFusionController`-style I2C detectors.
- The remainder of `MSI_185Byte_Controller_0x0DB0.js` (lines 1379–2617, not
  yet read — likely covers `MysticLight185`/other classes for non-761 boards
  sharing this VID).

## Open questions

1. ~~Extend `IHidDevice` with a read capability?~~ **Done** (API 3, see
   above). User-approved; ASRock Polychrome ported using it (write path +
   ack drain only, not yet for discovery).
2. **SMBus/Super I/O tier** (9 of 21): widen the PawnIO broker beyond its
   currently qualified AMD `1022:790b` scope, and add new PawnIO modules
   (Intel I801, LPC/Super I/O)? User-approved to go ahead; not yet
   started. `PAWNIO_SERVICE_RESEARCH_2026-10-27.md` explicitly flags this
   as needing review before any new fixed address is qualified.

The five remaining unread HID controllers (JGINYUE x2, HP Omen,
Gigabyte Fusion2 USB, Asus Aura mainboard) are next, since some of them
may turn out to be write-only/table-driven like `msi.gen1` and portable
immediately. The ASRock config-read offset discrepancy (4 vs 5) and the
MSI 112/162/185 PID→family table remain open before those two specific
discovery paths can be added.
