# MSI Mainboard-LED-Protokoll, "800er"-Generation (VID `0x0DB0`/PID `0x0076`)

Betrifft u.a. das in `OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md`
analysierte MSI MPG B850 Edge TI WiFi. Quelle: `MSI_800sLed.cs`, identisch
(bis auf kleine Abweichungen) in `LEDKeeper2.exe\MSI_LED\` und
`MysticLight_AllDevice.dll\Device\MB_800\`. Treiberauswahl bestätigt über
`VID_MSI_Common = 3504` (`0x0DB0`), `PID_MSI_Common = 118` (`0x0076`) —
exakt die VID:PID des B850-Boards.

**Diese Datei ist eine Korrektur** zu einem Teil von
`OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md` (dort als "Offener Kernbefund"/
"Bestätigtes Muster" markiert) — siehe die "Korrektur 2026-10-17"-Sektion,
die in jener Datei direkt eingefügt wurde. Alles hier ist **rein aus dem
decompilierten Vendor-Code abgeleitet, nichts davon wurde ans Board
gesendet.**

## Report `0x50` ("Gen1"), 290 Byte — Status- und Default-Konfiguration

```
Byte 0:        Report-ID (0x50 / 80)
Byte 1-288:    18 Records à 16 Byte (siehe unten), lückenlos ab Byte 1
Byte 289:      "Store"-Flag (1 = dauerhaft ins Board-NVRAM schreiben, 0 = nur Sitzung)
```

**Korrektur gegenüber der ursprünglichen Capture-Analyse**: Es gibt
**keinen separaten 13-Byte-Header** vor den Records — Record 0 beginnt
direkt bei Byte 1. Was die ursprüngliche Analyse als "Header" (Byte 1-13)
gelesen hat, sind tatsächlich die ersten 13 der 16 Bytes von Record 0
(`LightingMode` + `Color_1..4`, ohne die letzten 3 Felder). Das erklärt
rückwirkend sowohl die beobachtete "Farb-Record = Auswahl-Record − 1"-Formel
als auch den Sonderfall "Header = JARGB1-Farbe": beide Beobachtungen sind
Artefakte einer um 13 Byte verschobenen Record-Grenze, keine echte
Protokoll-Eigenheit.

### Record-Layout (16 Byte, pro Zone/"Area")

| Offset im Record | Feld | Bedeutung |
|---|---|---|
| 0 | `LightingMode` | `Off=0,Wave=1,Steady=2,Flame=3,Breathing=4,ColorRing=5,Lightning=6,Recreation=7,Meteor=8,Advanced=9,GodLike=10` |
| 1-3 | `Color_1` (R,G,B) | |
| 4-6 | `Color_2` (R,G,B) | |
| 7-9 | `Color_3` (R,G,B) | |
| 10-12 | `Color_4` (R,G,B) | |
| 13 | `Option1` | Bits 0-1: `ColorCount` (1-4 aktive Farben) |
| 14 | `Option2` | Bit 7: `SelectAll` · Bit 6: `Direction` · Bit 5: `ColorSelection` (0=Rainbow,1=User) · Bits 4-2: `Brightness` (0-5 → 0/20/40/60/80/100%) · Bits 1-0: `Speed` (0-2) |
| 15 | `Cycle_Num` | z.B. `30` (Standard) |

18 Records = `Enum_LedArea`: `JARGB1,JARGB2,JARGB3,JAF,JPIPE1-5,JRGB1,JRGB2,
Onboard1-6,SelectAll` (Index 0-17). **Diese Namen sind die internen
Vendor-Bezeichnungen** — wie sie sich auf die vom Nutzer beobachteten
Board-Labels (IO Cover, EZ Conn, JARGB_V2_1/2/3) abbilden, ist **noch nicht
feldweise nachgewiesen** (nur plausibel, nicht verifiziert: Namensähnlichkeit
≠ Index-Übereinstimmung).

### Verifikation anhand des bestehenden Capture-Templates

Der 290-Byte-Rohwert aus `MSIMysticLight290Controller.cpp`
(`MSI290_TEMPLATE`, eine echte, vom Board akzeptierte Capture) deckt sich
exakt mit obigem Layout, u.a.:

- `Cycle_Num` an der vorhergesagten Position = `0x1E` = 30 (Standardwert) ✓
- `Option2` = `0xB5` an der vorhergesagten Position → dekodiert als
  `SelectAll=true, ColorSelection=User, Brightness=100%, Speed=Medium` —
  der beobachtete Wertewechsel `0xB5↔0x35` in den Captures ist also **nur
  das `SelectAll`-Bit**, kein eigenständiges "Zonen-Auswahl-Marker-Byte".

### ⚠️ Sicherheitsrelevanter Befund: `Store`-Flag wird bei jedem Schreibvorgang gesetzt

`MSI290_TEMPLATE` (und damit jeder Schreibvorgang von
`MSIMysticLight290Controller::Update()`, der dieses Template unverändert als
Basis kopiert) hat Byte 289 = `0x01` — **persistiert bei jedem einzelnen
`SET_REPORT` ins Board-NVRAM**, auch während des mit 50ms gedrosselten
Effects-Plugin-Render-Loops (bis zu 20 Flash-Schreibvorgänge/Sekunde bei
Dauerbetrieb eines Effekts). Das ist spezifisch der Flash-Verschleiß, den
die Sicherheitsregel in `PROTOCOL_NOTES.md` vermeiden soll, und ein
plausibler (bisher nicht als solcher identifizierter) Mitverursacher des
dort dokumentierten Hänger-Bugs (`hid_send_feature_report` blockierte
dauerhaft). **Noch nicht behoben** — Fix wäre, Byte 289 für
Direct-Mode-Schreibvorgänge auf `0x00` zu setzen (Store nur bei einem
expliziten "Anwenden"/Speichern-Klick). Nicht ungefragt umgesetzt, siehe
Sicherheitsregel in `PROTOCOL_NOTES.md`.

## Report `0x51` ("Gen1_Adv"), 727 Byte — Per-LED-Ansteuerung einzelner Segmente

```
Byte 0:   Report-ID (0x51 / 81)
Byte 1:   fester Wert 0x09
Byte 2-3: Area (big-endian uint16, siehe Enum_Gen1AdvArea)
Byte 4-5: reserviert (0x00)
Byte 6:   LED-Anzahl in diesem Paket
Byte 7+:  LED-Farben, 3 Byte (R,G,B) je LED
```

`Enum_Gen1AdvArea`: `JRGB1=512,JRGB2=513,JPIPE1=768,JPIPE2=769,JPIPE3=770,
JPIPE4=771,JPIPE5=772,JARGB1=1024,JARGB2=1025,JARGB3=1026,
OnBoard1-6=1792-1797,SelectAll=1536,JAF=2048`.

**Das widerlegt die vorherige Schlussfolgerung in `PROTOCOL_NOTES.md`**
("es gibt für unser konkretes Board aktuell keinen bekannten Mechanismus,
um einzelne LEDs innerhalb eines JARGB-Headers separat anzusprechen"): Dieser
Report existiert im offiziellen Treiber für genau diese VID:PID-Familie und
wird **aktiv genutzt** — `RGBControlClass.cs` (die SignalRGB-UDP-Brücke,
siehe `NETWORK_PROTOCOL.md`) ruft `Gen1_Adv_ApplyArea_Byte` für `JARGB1`,
`JARGB2`, `JARGB3`, `JAF`, `SelectAll` auf, sobald ein Drittanbieter-Client
individuelle LED-Farben schickt.

Der frühere Test (`probe_0x51.py`, `GET_FEATURE_REPORT(0x51, 761 Byte)`) ist
damit kein Widerspruch: er testete die **falsche Richtung** (GET statt SET)
mit der **falschen Länge** (761 Byte, aus einer anderen Boardfamilie
übernommen, statt der hier korrekten 727 Byte). Ein `GET_FEATURE_REPORT`
auf einen Report, den die Firmware nur als `SET` kennt, liefert plausibel
einen Timeout, ohne dass der Report als solcher fehlt.

**Noch nicht getestet** (würde das Board tatsächlich ansprechen — nicht
ohne explizite Freigabe gemäß Sicherheitsregel): `SetFeature` mit 727 Byte,
`Area=JARGB1 (1024)`, kleiner `Colors`-Array. Bricht die bisherige
"keine Einzel-LED-Ansteuerung"-Annahme auf Code-Ebene auf, ist aber am
echten Board noch nicht verifiziert.

## Report `0x90`+Port / `0xB0`+Port ("Gen2"/"Gen2_Adv") — neueres Strip-Modell

Zusätzliches, neueres System für adressierbare Multi-Strip-Header
(`Enum_TargetPort`: `JARGB1,JARGB2,JARGB3,JAF`), bis zu 6 physische Strips
pro Port à 180 LEDs (`Gen2_PortMaxStripCount=6`, `Gen2_StripMaxLedCount=180`).
Reports `0x90+Port` (Konfiguration/Erkennung, `Gen2_Detect`) und
`0xB0+Port` (761 Byte, tatsächliche Farben, `Gen2_Adv_ApplyPort`) — die
761-Byte-Länge deckt sich mit der von der Community
(`nmelo/msi-mystic-light-x870e`) für eine andere Boardfamilie dokumentierten
Struktur, dort allerdings unter Report-ID `0x51` vermutet. Plausibelste
Erklärung: **unterschiedliche Board-/Firmware-Generationen nutzen
unterschiedliche Reports für dasselbe Konzept** — das macht eine pauschale
"Report X = Feature Y"-Zuordnung über Boardfamilien hinweg unsicher, auch
innerhalb von MSIs eigenem Code (Gen1 vs. Gen2 sind zwei komplett getrennte
Codepfade mit eigenen Report-ID-Bereichen).

**Nicht geklärt**: ob das B850-Board (laut `PROTOCOL_NOTES.md` mit
"JARGB_V2"-Headern, was auf das neuere "Gen2"-System hindeuten könnte) in
der Praxis Gen1 oder Gen2 verwendet — dafür fehlt die Aufrufstelle, die bei
Geräte-Erkennung zwischen beiden Pfaden umschaltet (wahrscheinlich über die
Firmware-Version aus `Get_FwVersion`/`CheckConnectedDevice`, nicht weiter
verfolgt).

## Warum Mystic Light "weicher" wirkt als OpenRGB (Einordnung der Nutzerbeobachtung)

Zwei unabhängige, im Code belegte Gründe — **keine zusätzliche
Interpolations-/Smoothing-Logik auf Software-Seite gefunden** (weder in
`MSI_800sLed.cs` noch in `SyncStyleControl.cs`/`CControl.cs` — dort werden
Farben direkt weitergereicht, kein Easing/Blending-Code):

1. **MSI Center benutzt die firmware-eigenen Animationsmodi** (`Wave`,
   `Breathing`, `ColorRing`, `Meteor`, `GodLike`, …) — diese Übergänge laufen
   komplett in der Mikrocontroller-Firmware des Boards, inklusive deren
   eigener (uns nicht bekannter) Interpolation. OpenRGBs generischer
   `RGBController`/Direct-Mode-Ansatz setzt dagegen praktisch immer
   `LightingMode=Steady` und pusht einzelne, diskrete Farben vom Host aus,
   typischerweise ohne diese Modi überhaupt zu nutzen.
2. **Per-LED-Ansteuerung existiert** (siehe `0x51`/`0x90+`/`0xB0+` oben) und
   wird von MSI selbst für Drittanbieter-Integration genutzt — ein
   eigenständiger Treiber, der diese Reports nutzt statt nur `0x50`, könnte
   dieselbe Granularität erreichen wie MSI Center.

Für eine eigenständige App bedeutet das: **die Smoothness ist eine
Protokoll-/Feature-Wahl, kein Hardware-Vorteil von MSI.** Die relevante
Arbeit ist, `LightingMode` korrekt zu setzen bzw. die Per-LED-Reports zu
nutzen, nicht eigene Übergangs-Mathematik zu bauen.
