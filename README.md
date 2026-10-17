# MSI Mystic Light — Reverse Engineering

Reverse-Engineering-Notizen zur offiziellen MSI-Mystic-Light-Software
(`C:\Program Files (x86)\MSI\MSI Center\Mystic Light`), erstellt mit dem
`reverse-engineer-anything`-Skill/REA-MCP-Server + `ilspycmd` (.NET-Decompiler).

**Ziel:** Grundlage für eine eigenständige App, die MSI-RGB-Hardware ohne
MSI Center ansteuern kann.

**Kein Vendor-Code im Repo.** Die decompilierten `.cs`-Dateien liegen nur im
lokalen Scratchpad-Verzeichnis der jeweiligen Claude-Code-Session (temporär,
nicht Teil dieses Repos). Dokumente hier beschreiben Verhalten/Byte-Layouts
in eigenen Worten und verweisen per Datei:Zeile auf die Originalquelle als
Beleg — sie zitieren sie nicht großflächig.

## Inhalt

- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — Gesamtarchitektur:
  Windows-Dienst, LEDKeeper2.exe, MysticLight_AllDevice.dll, MLModule.dll,
  SyncData.dll.
- [`docs/NETWORK_PROTOCOL.md`](docs/NETWORK_PROTOCOL.md) — lokale
  Netzwerk-Schnittstellen (HTTP-SDK, UDP-Pairing/Sync, SignalRGB-UDP-Bridge).
- [`docs/MB_800_HID_PROTOCOL.md`](docs/MB_800_HID_PROTOCOL.md) — USB-HID-
  Protokoll der "800er"-Mainboard-Generation (VID `0x0DB0`/PID `0x0076`,
  Report `0x50`/`0x51`/Gen2) — Gegenstück/Korrektur zu
  `OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md`.
- [`docs/OPENRGB_UPSTREAM_DRAFT.md`](docs/OPENRGB_UPSTREAM_DRAFT.md) —
  Entwurf für einen Kommentar an die OpenRGB-Issues #4935/#5429. **Noch
  nicht gepostet, nur lokaler Entwurf.**

## Verwandte Repos

Teil eines Repo-Verbunds unter `Software_Projects/` (siehe auch
`OpenRGB/OpenRGBResearch/CONTINUE.md`):

- `../OpenRGB/OpenRGB` — eigener OpenRGB-Fork mit `MSIMysticLight290Controller`
- `../OpenRGB/OpenRGBResearch` — rohe Paket-Captures, Analyseskripte
