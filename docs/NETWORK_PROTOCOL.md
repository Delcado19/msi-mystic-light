# Mystic Light — lokale Netzwerk-Schnittstellen

Alle Befunde per statischer Dekompilierung (`ilspycmd`), keine
Laufzeitbeobachtung durchgeführt, sofern nicht anders vermerkt. Quelle:
`MLModule.dll` (Netzwerk-Kern) und `LEDKeeper2.exe\MSI_LED\RGBControlClass.cs`
(SignalRGB-Brücke).

## 1. TCP 26820 — lokale HTTP "Web API" (`MLModule.dll:CWebAPI.cs`)

```
HostServer.Prefixes.Add("http://localhost:" + ServerPort + "/");
HostServer.AuthenticationSchemes = AuthenticationSchemes.Anonymous;
```

Anfrageformat: `GET /DeviceType=MSI_KEYBOARD&SetColor=255,0,0` (Query-String
als Pseudo-Pfad, `&`-getrennt). Aktionen: `DeviceType`, `DeviceIndex`,
`GetDeviceInfo`, `GetStyleInfo`, `SetStyleID`, `SetColor`, `SetBright`,
`SetSpeed`, `SetPW`, `SetServerPort`.

**Keine Authentifizierung** — `SetPW` ist zwar als gültiger Parametername
registriert, hat aber **keinen** `case`-Handler im Switch-Statement der
Request-Parser-Methode (totes/unvollendetes Feature). Jeder lokale Prozess
kann ohne Weiteres alle Geräte steuern. Port ist über die Registry
(`WebAPI\ServerPort`) konfigurierbar.

**Für eine eigenständige App: das ist der primäre, stabilste Einstiegspunkt**
— dokumentiertes, SDK-artiges Protokoll, kein Reverse-Engineering des
rohen USB-Verkehrs nötig, solange MSI Center/der Dienst im Hintergrund läuft.

## 2. UDP 26822 — Mobile-App-Pairing (`CBaseServer.cs`/`CBaseFN.cs`)

Pairing-Protokoll für die MSI-Mobile-App. Nachrichtenformat:
`<Code>#<Feld1>#<Feld2>#...` (`#`-getrennt, UTF-8, Klartext).

- `A201` → Server vergibt Sitzungs-ID `CAPI.GetMD5(DateTime.Now.Ticks.ToString())`.
  **Kein Geheimnis** — nur ein Zeitstempel-Hash, im Klartext übertragen.
- `A202`/`A301` nutzen diese ID zur "Authentifizierung" nachfolgender Befehle.
- Jeder im gleichen Broadcast-Segment, der mitschneidet oder schneller als
  die echte App antwortet, kann sich als Mobilgerät ausgeben.

## 3. UDP 26823 — "Party Sync" zwischen mehreren PCs (`CSyncServer.cs`/`CSyncFN.cs`)

Analog zu (2), synchronisiert RGB-Effekte zwischen PCs im selben Netz.
Ebenfalls Klartext, keine Authentifizierung über mehr als eine lokal
generierte `CSyncData.ID`.

Alle drei Server öffnen beim Start automatisch eine `netsh advfirewall`-Regel
(`CNetwork.cs`) für eingehenden Traffic — stilles Firewall-Opening.

## 4. UDP 61337 — SignalRGB-Brücke ("LEDKeeper 2" Dienst)

**Dies war in `OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md` als Sackgasse
eingestuft** ("kein Listener auf Port 61337, trotz mehrfacher Neustarts").
Der Code existiert tatsächlich — `LEDKeeper2.exe\MSI_LED\RGBControlClass.cs`,
`UdpListener` bindet `IPAddress.Any:61337`. Grund für den vorherigen
Negativbefund, mit hoher Sicherheit:

```
App.cs: if (RGBCtrlObj != null) {
            ...
            if (CRegistry.GetKeyValue("LED", "SignalRGB_Switch") == "ON")
                RGBCtrlObj.StartServer();
        }
```

`StartServer()` wird **nur einmal beim Programmstart von `LEDKeeper2.exe`**
aufgerufen (plus über zwei interne String-Kommandos `"SignalRGB_On"` /
`"MSIMLSDK_On"`, deren Aufrufweg — vermutlich IPC vom MSI-Center-GUI-Prozess
— hier nicht weiter verfolgt wurde). Der Registry-Schalter wird also nur
beim **Prozessstart** von `LEDKeeper2.exe` gelesen. Ein Neustart von MSI
Center (GUI) oder des `Mystic_Light_Service`-Windows-Dienstes startet den
bereits laufenden `LEDKeeper2.exe`-Prozess selbst nicht neu (der läuft als
eigener Scheduled-Task-Prozess, nicht als Kindprozess des Dienstes) —
exakt das Verhalten, das in PROTOCOL_NOTES.md bereits beobachtet wurde
("GUI-Neustart startet den eigentlichen Dienst nicht neu").

**Nächster sinnvoller Test** (noch nicht durchgeführt, reine Prozess-
Verwaltung, kein Geräte-Zugriff): `SignalRGB_Switch=ON` setzen, dann
`LEDKeeper2.exe` **per Taskkill beenden** (nicht nur MSI Center/den Dienst)
und den Scheduled Task neu anlaufen lassen — danach sollte Port 61337
gebunden sein.

### Protokoll (bestätigt durch Code, ergänzt gegenüber der `Msi.js`-Rekonstruktion)

`RGBControlClass.cs` verarbeitet eingehende Pakete und routet sie u.a. auf
`MSI_800sLed.Gen1_Adv_ApplyArea_Byte(Enum_Gen1AdvArea Area, byte[] Colors)`
für `JARGB1`, `JARGB2`, `JARGB3`, `JAF`, `SelectAll` — das ist derselbe
**Per-LED-Report** (`0x51`/727 Byte, siehe
[`MB_800_HID_PROTOCOL.md`](MB_800_HID_PROTOCOL.md)), den die
Community-Referenz `nmelo/msi-mystic-light-x870e` für eine andere Platine
dokumentiert hat. Das bestätigt unabhängig vom rohen-USB-Reverse-Engineering:
**MSIs eigene Software benutzt den Per-LED-Report produktiv**, nicht nur
als ungetestete Protokoll-Variante.

## Noch offen

- Exakter Aufrufer von `CWebAPI.StartHostServer()` (siehe `ARCHITECTURE.md`).
- IPC-Weg für `"SignalRGB_On"`/`"MSIMLSDK_On"` (vermutlich Named Pipe oder
  Fenster-Message vom MSI-Center-GUI-Prozess an `LEDKeeper2.exe`).
- Passive Verifikation (netstat) auf diesem System, ob 26820/26822/26823
  tatsächlich gebunden sind — noch nicht durchgeführt.
