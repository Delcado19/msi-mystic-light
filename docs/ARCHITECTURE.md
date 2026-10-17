# Mystic Light — Gesamtarchitektur

Stand: 2026-10-17. Alle Angaben per statischer Analyse (ILSpy-Dekompilat,
.NET 4.8, keine Laufzeitbeobachtung) ermittelt. Pfade im Format `Datei:Zeile`
verweisen auf die decompilierte Quelle im Scratchpad der jeweiligen Session
(nicht Teil dieses Repos) — zur Nachvollziehbarkeit, nicht als Kopie.

## Komponenten

| Datei | Typ | Rolle |
|---|---|---|
| `Mystic_Light_Service.exe` | Windows-Dienst (.NET 4.8) | Startet/überwacht `LEDKeeper2.exe` und `muachost.exe` über den Task Scheduler |
| `LEDKeeper2.exe` | WPF/WinForms-App (.NET 4.8) | Haupt-App: Geräte-Treiberlogik, Netzwerk-Server, Profile |
| `MysticLight_AllDevice.dll` | .NET-Bibliothek | Eigenständige Geräte-Treiber-SDK, teilweise Code-Duplikat von LEDKeeper2s `MSI_LED`-Namespace |
| `MLModule.dll` | .NET-Bibliothek (trotz Namens **managed**, nicht nativ) | Netzwerk-Kern: HTTP-SDK, UDP-Pairing, UDP-Party-Sync |
| `SyncData.dll` | .NET-Bibliothek | Profil-Datenmodell + einfache AES-Verschlüsselung für lokale Config-Dateien |
| `Lib\MsiHid_GameSync.dll` | native DLL | Generischer USB-HID-Zugriff (P/Invoke-Ziel von `HID_Basic`) |

## Mystic_Light_Service.exe

Läuft als `LocalSystem`-Dienst. Umgeht die Windows-Session-0-Isolation nicht
per Token-Duplizierung, sondern über die Windows-Task-Scheduler-COM-API
(`ITaskService`/`ITaskDefinition`, `TASK_LOGON_S4U`), um `LEDKeeper2.exe` im
Desktop des angemeldeten Nutzers zu starten:

- `CreateTask_Keeper` registriert den Task "MSI Task Host - LEDKeeper2_Host",
  `Path = AppDomain.CurrentDomain.BaseDirectory + "LEDKeeper2.exe"` (fest,
  aus dem eigenen Installationsverzeichnis — keine externe Eingabe).
- Ein zweiter Task ("MSISW_Host") startet `muachost.exe` aus
  `Environment.GetFolderPath(SpecialFolder.SystemX86)`.
- `OnCustomCommand(int Value)` (Werte 201–204) steuert Start/Stop/Monitor
  der Tasks — angesprochen vom MSI-Center-GUI-Prozess.

**Geprüft und verworfen**: `Class_CallExternal.StartProcessByUserUAC`
(klassische `OpenProcessToken`/`DuplicateTokenEx`/`CreateProcessAsUser`-Kette
mit `lpApplicationName=null`, `applicationName` als rohe `lpCommandLine`) ist
im Dienst definiert, aber **ohne Aufrufstelle** — toter Code in diesem Build.
Kein aktiver Angriffsvektor über diesen Pfad.

## LEDKeeper2.exe

Enthält den eigentlichen Geräte-Treiberbaum (`MSI_LED`-Namespace, ~80
Gerätetreiber-Klassen: Tastaturen, Mäuse, Headsets, Mainboards, DRAM,
Monitore, Netzteile, Lüfter) sowie Drittanbieter-Protokoll-Implementierungen
(SteelSeries, Corsair Vengeance DRAM, GALAX, G.Skill, Patriot, Kingston,
Apacer, Micron). Details zum Mainboard-Protokoll der "800er"-Serie (betrifft
u.a. das B850-Board aus `OpenRGBResearch/msi-b850`) in
[`MB_800_HID_PROTOCOL.md`](MB_800_HID_PROTOCOL.md).

Netzwerk-Funktionen in LEDKeeper2.exe (`RGBControlClass.cs`): siehe
[`NETWORK_PROTOCOL.md`](NETWORK_PROTOCOL.md) — das ist die SignalRGB-Brücke,
die intern per-LED-Reports an die Board-Treiber weiterleitet.

## MLModule.dll — Netzwerk-Kern

Trotz `.dll`-Namenskonvention keine native Bibliothek, sondern eine
gewöhnliche .NET-Assembly (`"managed": true`, Ghidra lehnt sie deshalb
korrekt als Ziel ab). Enthält drei unabhängige Server, siehe
[`NETWORK_PROTOCOL.md`](NETWORK_PROTOCOL.md) für Details:

1. TCP 26820 — lokale HTTP-"Web API" (`CWebAPI.cs`), **keine Authentifizierung**.
2. UDP 26822 — Mobile-App-Pairing (`CBaseServer.cs`/`CBaseFN.cs`).
3. UDP 26823 — Mehr-PC-"Party Sync" (`CSyncServer.cs`/`CSyncFN.cs`).

Offen: `CWebAPI.StartHostServer()` hat innerhalb von `MLModule.dll` selbst
keine gefundene Aufrufstelle — wird vermutlich aus `LEDKeeper2.exe` oder
`Mystic_Light_Service.exe` heraus aufgerufen (Cross-Assembly-Aufruf, mit
reiner Textsuche in den decompilierten `.cs`-Dateien nicht sichtbar, da
`ilspycmd` je Assembly separat decompiliert). Nicht abschließend verifiziert
— vor einem produktiven Nachbau mit `dotnet-trace`/Prozessmonitor am
echten System bestätigen.

## SyncData.dll

Reines Datenmodell (7 Typen, kein nennenswerter Logikanteil: Enums für
Stil/Zyklus/Gerätenamen/Chipsätze, `DataBase`-Klasse). `C_Encrypt.cs`
implementiert AES-Verschlüsselung für lokale Konfigurationsdateien
(Profile/Monitor-Configs) — **IV wird per MD5 aus demselben Schlüssel
abgeleitet wie der AES-Key selbst** (kryptographisch unsauber, da IV und Key
voneinander ableitbar sind), aber nur zur Obfuskation lokaler Dateien
verwendet, kein Netzwerk-Geheimnis. Kein weiterer Handlungsbedarf für das
eigenständige-App-Vorhaben.

## HID-Passwort-Mechanismus (`HID_Basic.init`)

`MysticLight_AllDevice.dll` und mehrere `HID_Basic`-Varianten in
`LEDKeeper2.exe` (`MsiHid/HID_Basic.cs`, `MsiHid_GM41W/HID_Basic.cs`, u.a.)
definieren `init(string psw)` — ein 7-Byte-"Passwort", das vor dem eigentlichen
HID-Zugriff an die native `MsiHid_GameSync.dll` übergeben werden kann.

**Befund: in keiner der geprüften Aufrufer-Dateien (gesamtes `LEDKeeper2.exe`
und `MysticLight_AllDevice.dll`) wird `init(...)` tatsächlich aufgerufen.**
Totes/vestigiales Feature. Deckt sich mit der bereits in
`OpenRGBResearch/msi-b850` dokumentierten Beobachtung, dass echte
`hidapi`-Sends ohne jeden Init-Aufruf erfolgreich waren — für eine
eigenständige App ist dieser Schritt mit hoher Sicherheit **nicht**
erforderlich, zumindest nicht für die hier geprüften Geräteklassen.
(Nicht geprüft: `HID_Basic_KB.cs`, `HID_Basic_Mouse.cs`, `HID_Basic_MNT.cs`,
`HID_Basic_PSU.cs` — Grep lief über das ganze Verzeichnis, aber keine dieser
Dateien selbst enthält einen `init(`-Aufruf an eine andere Instanz; falls doch
irgendwo ein Keyboard/Maus-spezifischer Pfad das Passwort braucht, wurde das
hier nicht einzeln nachvollzogen.)
