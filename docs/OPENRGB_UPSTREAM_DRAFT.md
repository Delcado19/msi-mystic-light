# Entwurf: Kommentar für GitLab-Issues #4935 / #5429

**Status: NICHT gepostet.** Nur lokaler Entwurf zur Freigabe. Enthält keinen
Vendor-Code (nur Byte-Layout-Tabellen/Fakten, aus dem decompilierten Treiber
abgeleitet). Vor dem Posten: mit Nutzer abstimmen, ob überhaupt (siehe
CONTRIBUTING.md-Hinweis in `OpenRGBResearch/CONTINUE.md`: "kein eigener PR" —
ein reiner Issue-Kommentar mit Reverse-Engineering-Infos könnte trotzdem
gewünscht sein, aber das ist eine externe, sichtbare Aktion und wird hier nur
vorbereitet, nicht ausgeführt).

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

**Safety note for anyone testing against real hardware**: the vendor's own
`0x50` payloads set the "store to NVRAM" trailer byte to `1` on every write.
A controller that re-sends a captured template at high frequency (e.g. a
continuous effect) will hit flash on every write unless that byte is
explicitly zeroed for non-persistent updates.

Happy to answer questions on this; haven't tested the `0x51`/Gen2 paths
against real hardware yet.
