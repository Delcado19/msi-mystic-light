# MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`)

Covers, among others, the MSI MPG B850 Edge TI WiFi analyzed in
`OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md`. Source: `MSI_800sLed.cs`,
identical (aside from minor differences) in `LEDKeeper2.exe\MSI_LED\` and
`MysticLight_AllDevice.dll\Device\MB_800\`. Driver selection confirmed via
`VID_MSI_Common = 3504` (`0x0DB0`), `PID_MSI_Common = 118` (`0x0076`) —
exactly the VID:PID of the B850 board.

**This file is a correction** to part of
`OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md` (there marked as "Open core
finding"/"Confirmed pattern") — see the "Correction 2026-10-17" section
inserted directly into that file. Everything here is **derived purely from
the decompiled vendor code; nothing was sent to the board.**

## Report `0x50` ("Gen1"), 290 bytes — status and default configuration

```
Byte 0:        Report ID (0x50 / 80)
Byte 1-288:    18 records of 16 bytes each (see below), contiguous from byte 1
Byte 289:      "Store" flag (1 = persist to board NVRAM, 0 = session only)
```

**Correction to the original capture analysis**: there is **no separate
13-byte header** before the records — record 0 starts directly at byte 1.
What the original analysis read as a "header" (bytes 1-13) is actually the
first 13 of the 16 bytes of record 0 (`LightingMode` + `Color_1..4`, minus
the last 3 fields). This retroactively explains both the observed
"color record = selection record − 1" formula and the special case "header =
JARGB1 color": both observations are artifacts of a record boundary shifted
by 13 bytes, not a real protocol quirk.

### Record layout (16 bytes, per zone/"area")

| Offset in record | Field | Meaning |
|---|---|---|
| 0 | `LightingMode` | `Off=0,Wave=1,Steady=2,Flame=3,Breathing=4,ColorRing=5,Lightning=6,Recreation=7,Meteor=8,Advanced=9,GodLike=10` |
| 1-3 | `Color_1` (R,G,B) | |
| 4-6 | `Color_2` (R,G,B) | |
| 7-9 | `Color_3` (R,G,B) | |
| 10-12 | `Color_4` (R,G,B) | |
| 13 | `Option1` | Bits 0-1: `ColorCount` (1-4 active colors) |
| 14 | `Option2` | Bit 7: `SelectAll` · Bit 6: `Direction` · Bit 5: `ColorSelection` (0=Rainbow,1=User) · Bits 4-2: `Brightness` (0-5 → 0/20/40/60/80/100%) · Bits 1-0: `Speed` (0-2) |
| 15 | `Cycle_Num` | e.g. `30` (default) |

18 records = `Enum_LedArea`: `JARGB1,JARGB2,JARGB3,JAF,JPIPE1-5,JRGB1,JRGB2,
Onboard1-6,SelectAll` (index 0-17). These are the internal vendor labels;
for the MS-7E62 board specifically, the mapping to the labels shown in MSI
Center (IO Cover, EZ Conn, JARGB_V2_1/2/3) is now **confirmed, not
inferred** — MSI's own `Mystic Light Online Data.dat` catalog ships a
per-board zone list with this exact mapping (`JARGB_V2_1=0, JARGB_V2_2=1,
JARGB_V2_3=2, EZ Conn=3, IO COVER=4, JRGB1=9, SELECT ALL=11`), which matches
`OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md`'s independently
capture-derived Record-Index table exactly. See
[`ONLINE_DATA_CATALOG.md`](ONLINE_DATA_CATALOG.md). Note index 11 is
`Onboard1` in the generic enum but relabeled `SELECT ALL` in this board's
own catalog entry — a board-specific repurposing, not a separate area.

### Verification against the existing capture template

The 290-byte raw payload from `MSIMysticLight290Controller.cpp`
(`MSI290_TEMPLATE`, a real capture already accepted by the board) matches
this layout exactly, e.g.:

- `Cycle_Num` at the predicted position = `0x1E` = 30 (default value) ✓
- `Option2` = `0xB5` at the predicted position → decodes to
  `SelectAll=true, ColorSelection=User, Brightness=100%, Speed=Medium` —
  the observed `0xB5↔0x35` toggle seen in captures is thus **only the
  `SelectAll` bit**, not a standalone "zone selection marker byte".

### ⚠️ Safety-relevant finding: the `Store` flag is set on every write

`MSI290_TEMPLATE` (and thus every write performed by
`MSIMysticLight290Controller::Update()`, which copies this template
unchanged as its base) has byte 289 = `0x01` — **persists to board NVRAM on
every single `SET_REPORT`**, including during the 50ms-throttled Effects
Plugin render loop (up to 20 flash writes/second while an effect runs
continuously). This is specifically the kind of flash wear the safety rule
in `PROTOCOL_NOTES.md` is meant to avoid, and a plausible (previously
unidentified) contributor to the hang bug documented there
(`hid_send_feature_report` blocking indefinitely). **Fixed** (2026-10-17):
the controller's constructor now explicitly zeroes byte 289 right after
copying the template, since this controller is Direct-mode only and never
needs the persist-to-flash path. See `OpenRGB/Controllers/
MSIMysticLightController/MSIMysticLight290Controller/
MSIMysticLight290Controller.cpp` (`MSI290_STORE_OFFSET`). Not re-tested
against real hardware yet.

## Report `0x51` ("Gen1_Adv"), 727 bytes — per-LED addressing of individual segments

```
Byte 0:   Report ID (0x51 / 81)
Byte 1:   fixed value 0x09
Byte 2-3: Area (big-endian uint16, see Enum_Gen1AdvArea)
Byte 4-5: reserved (0x00)
Byte 6:   LED count in this packet
Byte 7+:  LED colors, 3 bytes (R,G,B) per LED
```

`Enum_Gen1AdvArea`: `JRGB1=512,JRGB2=513,JPIPE1=768,JPIPE2=769,JPIPE3=770,
JPIPE4=771,JPIPE5=772,JARGB1=1024,JARGB2=1025,JARGB3=1026,
OnBoard1-6=1792-1797,SelectAll=1536,JAF=2048`.

**This overturns the previous conclusion in `PROTOCOL_NOTES.md`** ("there
is currently no known mechanism for addressing individual LEDs within a
JARGB header separately for our specific board"): this report exists in the
official driver for exactly this VID:PID family and is **actively used** —
`RGBControlClass.cs` (the SignalRGB UDP bridge, see
`NETWORK_PROTOCOL.md`) calls `Gen1_Adv_ApplyArea_Byte` for `JARGB1`,
`JARGB2`, `JARGB3`, `JAF`, `SelectAll` as soon as a third-party client sends
individual LED colors.

The earlier test (`probe_0x51.py`, `GET_FEATURE_REPORT(0x51, 761 bytes)`) is
not a contradiction: it tested the **wrong direction** (GET instead of SET)
with the **wrong length** (761 bytes, carried over from a different board
family, instead of the correct 727 bytes here). A `GET_FEATURE_REPORT` on a
report the firmware only implements as `SET` plausibly times out without
the report actually being absent.

**Confirmed working via a third-party production app (2026-10-18)**:
SignalRGB (installed locally, v2.5.80, officially signed by WhirlwindFX)
ships a maintained plugin for this exact VID (`0x0DB0`, file
`185 Byte Motherboard Controller/MSI_185Byte_Controller_0x0DB0.js`) with
an explicit entry for `MPG B850 EDGE TI WIFI (MS-7E62)`. It detects this
report with `device.send_report([0x51], 761)` — a **SET**, not the GET
our earlier probe used — and on success sends real per-header color data
as `[0x51, 0x09, 0x04, HeaderIndex, 0x00, 0x00, LedCount, ...RGB]` at
**727 bytes**, matching our derivation exactly (761 is only the probe's
declared length, not the real packet size — same naming artifact we
found in OpenRGB's own code, apparently inherited from the same
community lineage). No "Store" flag equivalent for this report; it's
purely transient, sent every render frame with no explicit per-packet
delay in this code path (global frame-rate capping happens elsewhere in
the app). This closes the "not yet tested" gap without touching the
board ourselves — static analysis of already-running, signed vendor
software.

**Exact LED topology for `MPG B850 EDGE TI WIFI (MS-7E62)`**, per that
plugin's own device table: 3 ARGB headers (`JARGB1/2/3`, up to 120 LEDs
each), 1 EZ Connector (120 LEDs), `JPIPE1` ("IO Shield", 11 LEDs,
vertical), `JRGB1` (1 non-addressable LED). Other `MS-7E62`-coded SKUs in
the same table (e.g. `MAG B850 TOMAHAWK MAX WIFI`) omit the IO Shield
zone — confirms board SKUs sharing a chip ID can still differ in which
optional zones are physically present, as already suspected from the
MSI online catalog.

## Report `0x90`+port / `0xB0`+port ("Gen2"/"Gen2_Adv") — newer strip model

An additional, newer system for addressable multi-strip headers
(`Enum_TargetPort`: `JARGB1,JARGB2,JARGB3,JAF`), up to 6 physical strips per
port at 180 LEDs each (`Gen2_PortMaxStripCount=6`,
`Gen2_StripMaxLedCount=180`). Reports `0x90+port` (configuration/detection,
`Gen2_Detect`) and `0xB0+port` (761 bytes, actual colors,
`Gen2_Adv_ApplyPort`) — the 761-byte length matches the structure documented
by the community (`nmelo/msi-mystic-light-x870e`) for a different board
family, though there assumed to be report ID `0x51`. Most plausible
explanation: **different board/firmware generations use different reports
for the same concept** — this makes a blanket "report X = feature Y"
mapping across board families unreliable, even within MSI's own code (Gen1
vs. Gen2 are two entirely separate code paths with their own report ID
ranges).

**Resolved (2026-10-18)**: this board uses Gen1 only, confirmed from three
independent sources now — `LEDKeeper2.exe`'s `Class_MB_800.cs` never calls
any `Gen2_*` method, neither does the official redistributable SDK
(`MysticLight_AllDevice.dll`), and SignalRGB's own plugin for VID `0x0DB0`
(the `MysticLight761` class) only ever uses the `0x50`/`0x51` pair. Gen2 is
real tech, but only for a *different* VID (`0x1462`, an older/different
chip generation) — that plugin's code explicitly disables Gen2 support on
its own boards via a dedicated write, something never exercised anywhere
for `0x0DB0`.

## Upstream OpenRGB bug found while cross-checking: `FeaturePacket_PerLED_761` is 727 bytes, not 761

Independent of this repo's own findings, reading OpenRGB's existing (not
our fork's) `MSIMotherboardController` code against the vendor-confirmed
727-byte `Gen1_Adv` report above turned up a genuine bug in upstream:

- `MSIMotherboardCommon.h`: `FeaturePacket_PerLED_761` is named "761" but
  its actual `sizeof()` is 727 bytes (7-byte header + `NUM_LEDS_761=720`
  color bytes) — the name is a historical carry-over from the
  `nmelo/msi-mystic-light-x870e` reference (761 bytes, different board),
  not the real size of this struct.
- `MSIMotherboard761Controller::Update()` already uses
  `sizeof(FeaturePacket_PerLED_761)` (727, correct) for its real writes.
- `MSIMotherboardControllerDetect.cpp`'s detection probe instead sent a
  hardcoded `761` — a real length mismatch against the same report, not
  just a cosmetic naming issue. Windows' `HidD_SetFeature` requires an
  exact length match with the device's HID report descriptor, so the probe
  could fail outright independent of whether the board actually supports
  report `0x51`.

**Fixed** (2026-10-17, local fix, not upstreamed): the probe now sends
`sizeof(FeaturePacket_PerLED_761)` instead of the literal `761`. The struct
is still named "761" — renaming it would touch class/file names across
multiple files and wasn't done as part of this fix.

## Why Mystic Light appears "softer" than OpenRGB (context for the user's observation)

Two independent, code-backed reasons — **no additional interpolation/
smoothing logic found on the software side** (neither in `MSI_800sLed.cs`
nor in `SyncStyleControl.cs`/`CControl.cs` — colors are passed through
directly there, no easing/blending code):

1. **MSI Center uses the firmware's own animation modes** (`Wave`,
   `Breathing`, `ColorRing`, `Meteor`, `GodLike`, …) — these transitions run
   entirely inside the board's microcontroller firmware, including its own
   (to us unknown) interpolation. OpenRGB's generic `RGBController`/Direct
   mode approach, by contrast, practically always sets `LightingMode=Steady`
   and pushes individual, discrete colors from the host, typically never
   using these modes at all.
2. **Per-LED addressing exists** (see `0x51`/`0x90+`/`0xB0+` above) and is
   used by MSI itself for third-party integration — a standalone driver
   that uses these reports instead of only `0x50` could achieve the same
   granularity as MSI Center.

For a standalone app this means: **the smoothness is a protocol/feature
choice, not a hardware advantage of MSI's.** The relevant work is setting
`LightingMode` correctly, or using the per-LED reports, not building custom
transition math.
