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

**Not pursued further here**: the other catalog sections (per-board zone
counts for every other MSI motherboard, GPU image mappings, monitor lists)
are a ready-made, officially-sourced device database covering far more
boards than just this one — potentially very useful for a standalone app
with broader hardware support, but out of scope for the current
single-board investigation. The full decrypted catalog is not committed to
this repo (proprietary MSI content, large); only this file documents how
to reproduce it locally and what the relevant MS-7E62 entry says.
