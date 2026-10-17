# Entwurf: Kommentar für GitLab-Issues #4935 / #5429

**Status (2026-10-17): vom Nutzer freigegeben zum Posten auf beide Issues,
aber noch nicht gepostet** — kein GitLab-API-Zugriff verfügbar (gecachtes
Credential hat nur `write_repository`-Scope, kein `api`-Scope). Enthält
keinen Vendor-Code, nur Byte-Layout-Tabellen/Fakten. Kein eigener PR (siehe
`CONTRIBUTING.md`s "AI Guidelines" — PR nur, wenn der Nutzer den Diff selbst
versteht/verantwortet); der 761-Byte-Fix liegt stattdessen als Branch auf
dem Fork (`fix/motherboard-761-probe-length`, bereits gepusht), auf den
dieser Kommentar verlinkt.

Text unterhalb der Trennlinie ist 1:1 zum Posten gedacht (Markdown, GitLab-
kompatibel). Alles darüber ist nur interne Notiz, nicht Teil des Kommentars.

---

Reverse-engineered the vendor driver (MSI Mystic Light / LEDKeeper2, current
retail build) for the `0DB0:0076` "185-byte boards" family via .NET
decompilation (no raw USB capture needed for this part). Posting in case it
helps anyone else working this board family — happy to share more detail on
request.

**Report `0x50` (290 bytes) is not a 13-byte header + 17×16-byte records.**
It's 18 fixed-size 16-byte records starting immediately at byte 1 (no
separate header), one per named area (`JARGB1, JARGB2, JARGB3, JAF, JPIPE1-5,
JRGB1, JRGB2, OnBoard1-6, SelectAll`), plus a single trailing "store to
NVRAM" flag byte at offset 289. Per-record layout (16 bytes):

```
0:     LightingMode  (Off/Wave/Steady/Flame/Breathing/ColorRing/
                       Lightning/Recreation/Meteor/Advanced/GodLike)
1-3:   Color_1 (R,G,B)
4-6:   Color_2 (R,G,B)
7-9:   Color_3 (R,G,B)
10-12: Color_4 (R,G,B)
13:    ColorCount (bits 0-1)
14:    SelectAll(bit7) | Direction(bit6) | ColorSelection(bit5) |
       Brightness(bits4-2) | Speed(bits1-0)
15:    CycleNum
```

**Per-LED addressing exists for this family**, via a separate report
`0x51` ("Gen1_Adv", 727 bytes: `[0x51][0x09][Area hi][Area lo][0x00][0x00]
[LedCount][RGB...]`), and is actively used by MSI's own SignalRGB bridge
(a local UDP service on port 61337) for `JARGB1/JARGB2/JARGB3/JAF`. There's
also a newer "Gen2" per-strip system (`0x90+port`/`0xB0+port`, up to 6
strips × 180 LEDs per port) that may be what boards with "JARGB_V2" headers
actually use instead of the `0x50`/`0x51` pair above — not yet confirmed
which path this specific board takes.

**This isn't a one-board finding.** The vendor driver's internal zone
list (`JARGB_V2_1/2/3`, `EZ Conn`, `IO COVER`, `JRGB1`, `SELECT ALL`) with
byte-identical per-zone parameters shows up, unchanged, across roughly 135
distinct MSI motherboard models of this generation (every board whose
vendor software reports that same zone set) — not just the one board this
was tested on. A `0x51`/`Gen1_Adv` controller for this family would add
per-LED addressing for all of them at once, not a single-board fix.

**Two concrete bugs in the existing `MSIMotherboardController` code**,
found independently of the protocol work above, both still unfixed
upstream as far as I can tell:

1. `FeaturePacket_PerLED_761` (`MSIMotherboardCommon.h`) is actually 727
   bytes (`sizeof()`), not 761 — the name is a carry-over from a different
   board family's community writeup. `MSIMotherboard761Controller::Update()`
   already uses `sizeof(FeaturePacket_PerLED_761)` correctly, but the
   detection probe in `MSIMotherboardControllerDetect.cpp` sends a
   hardcoded `761`. Since `HidD_SetFeature` requires an exact length match
   with the device's HID report, that probe can fail outright regardless
   of whether a given board actually supports the report — worth checking
   if this explains any of the "detected but `761Controller` doesn't do
   anything" reports out there. One-line fix, pushed to a branch here in
   case it's useful as a starting point (not submitted as a merge request
   myself — happy to leave that to whoever picks this up, or point me at
   the right process if a MR is wanted):
   https://gitlab.com/Delcado19/OpenRGB/-/tree/fix/motherboard-761-probe-length

   ```diff
   -           size_t packet_length_new_attempt = hid_send_feature_report(dev, second_buffer, 761);
   +           size_t packet_length_new_attempt = hid_send_feature_report(dev, second_buffer, sizeof(FeaturePacket_PerLED_761));
   ```
2. Any `MSIMotherboard*Controller`/custom controller for this family that
   copies a captured `0x50` template verbatim should double-check its
   trailing byte: it's a "persist this to the board's NVRAM" flag, and
   genuine captures have it set to `1`. A controller that re-sends that
   template at high frequency (e.g. a continuous software effect) will
   hit flash on every single write unless that byte is explicitly zeroed
   for non-persistent updates.

Happy to answer questions on this; haven't tested the `0x51`/Gen2 paths
against real hardware yet.
