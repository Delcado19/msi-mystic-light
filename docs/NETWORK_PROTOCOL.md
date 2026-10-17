# Mystic Light — local network interfaces

All findings from static decompilation (`ilspycmd`), no runtime observation
unless noted otherwise. Source: `MLModule.dll` (network core) and
`LEDKeeper2.exe\MSI_LED\RGBControlClass.cs` (SignalRGB bridge).

## 1. TCP 26820 — local HTTP "Web API" (`MLModule.dll:CWebAPI.cs`)

```
HostServer.Prefixes.Add("http://localhost:" + ServerPort + "/");
HostServer.AuthenticationSchemes = AuthenticationSchemes.Anonymous;
```

Request format: `GET /DeviceType=MSI_KEYBOARD&SetColor=255,0,0`
(query-string-as-pseudo-path, `&`-separated). Actions: `DeviceType`,
`DeviceIndex`, `GetDeviceInfo`, `GetStyleInfo`, `SetStyleID`, `SetColor`,
`SetBright`, `SetSpeed`, `SetPW`, `SetServerPort`.

**No authentication** — `SetPW` is registered as a valid parameter name but
has **no** `case` handler in the request parser's switch statement
(dead/unfinished feature). Any local process can control all devices
without restriction. Port is configurable via the registry
(`WebAPI\ServerPort`).

**For a standalone app: this is the primary, most stable entry point** —
a documented, SDK-like protocol, no need to reverse-engineer raw USB traffic
as long as MSI Center/the service runs in the background.

## 2. UDP 26822 — mobile app pairing (`CBaseServer.cs`/`CBaseFN.cs`)

Pairing protocol for the MSI mobile app. Message format:
`<Code>#<Field1>#<Field2>#...` (`#`-separated, UTF-8, plaintext).

- `A201` → server assigns a session ID `CAPI.GetMD5(DateTime.Now.Ticks.ToString())`.
  **Not a secret** — just a timestamp hash, transmitted in plaintext.
- `A202`/`A301` use this ID to "authenticate" subsequent commands.
- Anyone on the same broadcast segment who sniffs traffic or responds
  faster than the real app can impersonate a mobile device.

## 3. UDP 26823 — "party sync" between multiple PCs (`CSyncServer.cs`/`CSyncFN.cs`)

Analogous to (2), synchronizes RGB effects between PCs on the same network.
Also plaintext, no authentication beyond a locally generated `CSyncData.ID`.

All three servers automatically open a `netsh advfirewall` rule
(`CNetwork.cs`) for inbound traffic on startup — a silent firewall opening.

## 4. UDP 61337 — SignalRGB bridge ("LEDKeeper 2" service)

**This was classified as a dead end in
`OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md`** ("no listener on port 61337,
despite multiple restarts"). The code does in fact exist —
`LEDKeeper2.exe\MSI_LED\RGBControlClass.cs`, `UdpListener` binds
`IPAddress.Any:61337`. Most likely reason for the earlier negative result:

```
App.cs: if (RGBCtrlObj != null) {
            ...
            if (CRegistry.GetKeyValue("LED", "SignalRGB_Switch") == "ON")
                RGBCtrlObj.StartServer();
        }
```

`StartServer()` is only called **once, at process startup of
`LEDKeeper2.exe`** (plus via two internal string commands `"SignalRGB_On"` /
`"MSIMLSDK_On"`, whose call path — likely IPC from the MSI Center GUI
process — was not traced further here). So the registry switch is only read
at **process start**. Restarting MSI Center (GUI) or the
`Mystic_Light_Service` Windows service does not restart the already-running
`LEDKeeper2.exe` process itself (it runs as its own scheduled-task process,
not a child process of the service) — exactly the behavior already observed
in PROTOCOL_NOTES.md ("restarting the GUI does not restart the actual
service").

**Next reasonable test** (not yet performed, pure process management, no
device access): set `SignalRGB_Switch=ON`, then **kill `LEDKeeper2.exe` via
taskkill** (not just MSI Center/the service) and let the scheduled task
restart it — port 61337 should then be bound.

### Protocol (confirmed via code, extends the `Msi.js` reconstruction)

`RGBControlClass.cs` processes incoming packets and routes them, among
others, to `MSI_800sLed.Gen1_Adv_ApplyArea_Byte(Enum_Gen1AdvArea Area, byte[]
Colors)` for `JARGB1`, `JARGB2`, `JARGB3`, `JAF`, `SelectAll` — this is the
same **per-LED report** (`0x51`/727 bytes, see
[`MB_800_HID_PROTOCOL.md`](MB_800_HID_PROTOCOL.md)) that the community
reference `nmelo/msi-mystic-light-x870e` documents for a different board.
This independently confirms, outside of raw-USB reverse engineering:
**MSI's own software actively uses the per-LED report**, not just as an
untested protocol variant.

## Still open

- Exact caller of `CWebAPI.StartHostServer()` (see `ARCHITECTURE.md`).
- IPC path for `"SignalRGB_On"`/`"MSIMLSDK_On"` (likely a named pipe or
  window message from the MSI Center GUI process to `LEDKeeper2.exe`).
- Passive verification (netstat) on this system of whether
  26820/26822/26823 are actually bound — not yet performed.
