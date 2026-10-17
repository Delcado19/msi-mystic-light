# `Mystic Light Online Data.dat` — decryption and content

`SyncData.dll`'s `DataBase` class (previously thought unused) is actually
loaded by `MLModule.dll`'s `CLEDParser.GetListData()` as the authoritative
device/zone catalog, with a hardcoded fallback (`DataBase.ListData_Backup`)
if the online file is missing. Found while re-checking `SyncData.dll` call
sites for dead code — it isn't dead, it was just not referenced by name
("DataBase.ListData"/"DataBase.GetListData()", not "SyncData.DataBase").

## Decryption recipe (read-only, nothing sent to any device)

File: `C:\Program Files (x86)\MSI\MSI Center\Mystic Light\Mystic Light
Online Data.dat` (also mirrored under `...\MSI Center\Data\`). Same format
as `SyncData.C_Encrypt.EncryptBase64`'s output (`"!!MSI!!" + Base64(AES-256-CBC(...))`):

- Key source: `SyncData.DataBase.CryptoKey = 232345599.ToString("X")` =
  the literal string `"DD94FFF"`.
- AES key = `SHA256("DD94FFF")` (32 bytes), IV = `MD5("DD94FFF")` (16 bytes),
  CBC mode, PKCS7 padding — same derivation as `C_Encrypt`.
- Strip the 7-character `"!!MSI!!"` prefix, base64-decode, AES-256-CBC
  decrypt.

```bash
KEY=$(printf "DD94FFF" | openssl dgst -sha256 -binary | xxd -p -c 256)
IV=$(printf "DD94FFF" | openssl dgst -md5 -binary | xxd -p -c 256)
tail -c +8 "Mystic Light Online Data.dat" | base64 -d > data.bin
openssl enc -d -aes-256-cbc -K "$KEY" -iv "$IV" -in data.bin -out data.txt
```

## Content

~2000 lines, sections: `[UI_Image]`, `[Motherboard]`, `[Graphics]`,
`[GraphicsNumber]`, `[MAG_Monitor_Index]`, `[MAG_Monitor_List]`,
`[DynamicLight_List]`, `[Device_Online_Image]`, `[Edit_Time]`, `[MIXEFFECT]`,
`[SyncData]`. `CLEDParser` only reads the `[SyncData]` section into
`DataBase.ListData` — this is the actual per-board zone catalog, one line
per board variant:

```
M01|<ModelID>|<ModelID>|<Zone1Name>,<Zone1Index>,<...>|<Zone2Name>,<Zone2Index>,<...>|...
```

### Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi)

Six variant entries (`MS-7E62_1` through `MS-7E62_6`, presumably covering
different SKUs/regions with slightly different populated headers). Zone →
index mapping, **officially confirmed by this first-party MSI data** (not
inferred from packet captures):

| Zone label | Index |
|---|---|
| JARGB_V2_1 | 0 |
| JARGB_V2_2 | 1 |
| JARGB_V2_3 | 2 |
| EZ Conn | 3 |
| IO COVER | 4 |
| JRGB1 | 9 |
| SELECT ALL | 11 |

This independently confirms the Record-Index → Zone table in
`OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md` (established there via the
marker-byte-flip method on live captures) **exactly**, digit for digit.
It also resolves part of the "Records 8/16 always populated, meaning
unresolved" open question there: in the generic vendor `Enum_LedArea`
(see `MB_800_HID_PROTOCOL.md`), index 11 is named `Onboard1`, but this
board's own catalog entry relabels that same slot `SELECT ALL` — a
board-specific repurposing of an otherwise-generic area slot, not a
mysterious constant block.

## Scope: how many other boards share this exact protocol

The `[SyncData]` section lists 262 distinct motherboard model IDs in
total. Of those, **135 use the same zone-naming convention as MS-7E62**
(`JARGB_V2_1/2/3`, `EZ Conn`, `IO COVER`, `JRGB1`, `SELECT ALL` — zone
labels, not just similar names) — a strong indicator they all speak the
same `0x50`/18-record "Gen1" protocol (and, by extension, the same `0x51`
per-LED "Gen1_Adv" report) that `MB_800_HID_PROTOCOL.md` documents, not
just boards that happen to look similar. Sampled a few at random to check:
`MS-7E70` and `MS-7E82` both have byte-for-byte identical per-zone
parameter strings to `MS-7E62` (`69,1342D02C23469345A74401,,10,FF0000,
3+2,5+5,+1301`), differing only in *which* zones are physically present
(e.g. `MS-7E82` has no `EZ Conn`) — not in the underlying protocol. The
other 127 boards (e.g. `MS-7D14`, using `JRGB1`/`JRAINBOW1` zone names
instead) are the older `0x52`-based "112/162/185-byte" generation that
OpenRGB's existing `MSIMotherboard112/162/185Controller`s already target.

This also explains an architectural point that wasn't obvious from
`MSIMotherboardControllerDetect.cpp` alone: boards are differentiated by
**DMI motherboard name** (`dmi.getMainboard()`, matched against this exact
catalog's `MS-xxxx` keys), not by USB PID — many physically different
boards share the same `VID_0DB0`/`PID_0076` USB identity (one shared
EC/controller chip across a whole board generation), and this catalog file
is MSI's own source of truth for which zones each specific board actually
has. A standalone driver could read this same file instead of
hand-maintaining a static board table (which is what OpenRGB's
`MSIMotherboard761Controller.cpp` currently does, with only 34 boards
hardcoded — a fraction of the 135 this catalog lists for the same
protocol family).

The full `JARGB_V2`-family model list (135 boards, model IDs only — not
proprietary byte data; `MS-7D14` is *not* in this list, it only appears
above to illustrate the older-style exception found while sampling):

```
MS-7D33, MS-7D40, MS-7D46, MS-7D67, MS-7D68, MS-7D69, MS-7D70, MS-7D71,
MS-7D73, MS-7D74, MS-7D75, MS-7D76, MS-7D77, MS-7D78, MS-7D79, MS-7D80,
MS-7D85, MS-7D86, MS-7D87, MS-7D89, MS-7D90, MS-7D91, MS-7D93, MS-7D94,
MS-7D96, MS-7D97, MS-7D98, MS-7D99, MS-7E00, MS-7E01, MS-7E03, MS-7E06,
MS-7E07, MS-7E09, MS-7E10, MS-7E11, MS-7E12, MS-7E13, MS-7E14, MS-7E16,
MS-7E17, MS-7E18, MS-7E19, MS-7E20, MS-7E21, MS-7E22, MS-7E23, MS-7E24,
MS-7E25, MS-7E26, MS-7E27, MS-7E28, MS-7E29, MS-7E30, MS-7E31, MS-7E32,
MS-7E33, MS-7E34, MS-7E36, MS-7E37, MS-7E39, MS-7E40, MS-7E41, MS-7E42,
MS-7E43, MS-7E45, MS-7E46, MS-7E47, MS-7E48, MS-7E49, MS-7E50, MS-7E51,
MS-7E53, MS-7E54, MS-7E56, MS-7E57, MS-7E58, MS-7E59, MS-7E61, MS-7E62,
MS-7E63, MS-7E64, MS-7E65, MS-7E66, MS-7E67, MS-7E68, MS-7E69, MS-7E70,
MS-7E71, MS-7E72, MS-7E73, MS-7E74, MS-7E75, MS-7E76, MS-7E77, MS-7E78,
MS-7E79, MS-7E80, MS-7E81, MS-7E82, MS-7E83, MS-7E84, MS-7E85, MS-7E86,
MS-7E87, MS-7E88, MS-7E89, MS-7E90, MS-7E91, MS-7E92, MS-7E93, MS-7E95,
MS-7E96, MS-7E97, MS-7E98, MS-7F01, MS-7F02, MS-7F04, MS-7F05, MS-7F06,
MS-7F07, MS-7F08, MS-7F09, MS-7F10, MS-7F12, MS-7F14, MS-7F15, MS-7F16,
MS-7F17, MS-7F18, MS-7F20, MS-7F21, MS-7F22, MS-7F23
```

**Not pursued further here**: the other catalog sections (GPU image
mappings, monitor lists) are out of scope for this investigation. The full
decrypted catalog is not committed to this repo (proprietary MSI content,
large); only this file documents how to reproduce it locally.
