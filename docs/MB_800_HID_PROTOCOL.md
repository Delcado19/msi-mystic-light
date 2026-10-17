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
Onboard1-6,SelectAll` (index 0-17). **These names are the internal vendor
labels** — how they map onto the board labels observed by the user (IO
Cover, EZ Conn, JARGB_V2_1/2/3) has **not yet been verified field by
field** (plausible but unverified: name similarity ≠ index match).

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
(`hid_send_feature_report` blocking indefinitely). **Not fixed yet** — the
fix would be to set byte 289 to `0x00` for direct-mode writes (store only on
an explicit "apply"/save click). Not implemented without asking, per the
safety rule in `PROTOCOL_NOTES.md`.

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

**Not yet tested** (would actually address the board — not without
explicit approval per the safety rule): `SetFeature` with 727 bytes,
`Area=JARGB1 (1024)`, a small `Colors` array. Breaks open the previous
"no individual LED addressing" assumption at the code level, but not yet
verified against the real board.

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

**Not resolved**: whether the B850 board (per `PROTOCOL_NOTES.md`, with
"JARGB_V2" headers, which could hint at the newer "Gen2" system) actually
uses Gen1 or Gen2 in practice — the call site that switches between the two
paths during device detection is missing (likely based on the firmware
version from `Get_FwVersion`/`CheckConnectedDevice`, not traced further).

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
