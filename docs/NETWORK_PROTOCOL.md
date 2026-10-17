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

## Live verification (2026-10-17): the whole MLModule network layer is dead code

A full `netstat -ano` (TCP and UDP) shows nothing at all on 26820 or 26823,
and the only activity on 26822 is a **TCP** listener owned by a process
called `MSI.TerminalServer`. Tracing why led to the real explanation, not
just "unrelated process by coincidence":

**`CBaseServer.Start()`, `CWebAPI.StartHostServer()`, and `CSyncServer.Start()`
have no call site anywhere** — not in `LEDKeeper2.exe`, not in
`Mystic_Light_Service.exe`, not in `MysticLight_AllDevice.dll`, and not in
`API_Mystic Light.dll` (see below), which is the one other assembly that
actually does `using MLModule;`. The `WebAPI` registry key that
`CWebAPI.StartHostServer()` creates on first run doesn't exist on this
system either, confirming it has never run. All three servers in
`MLModule.dll` are dead/superseded code, same pattern as the unused
`HID_Basic.init` password and `StartProcessByUserUAC` found earlier.

**What's actually live instead**: `API_Mystic Light.dll`
(`C:\Program Files (x86)\MSI\MSI Center\Mystic Light\API_Mystic Light.dll`,
alongside sibling plugins `API_Nanoleaf.dll`, `API_PhilipsHue.dll`,
`API_PSUMonitor.dll`) implements `PluginClass : IPlugin_SDK` — this is a
plugin hosted in-process by MSI Center's current `MSI.TerminalServer.exe`
(the process actually bound to TCP 26822). `IPlugin_SDK.Transfer_Command
(byte[] SourceData)` forwards to `C_Feedback.DataResponse(byte[])`, a
simple 2-byte-command dispatcher (`CMD_UI_Get_List_PartItem = {1,1}`,
`CMD_UI_Set_RegKey = {2,1}`, etc.) that reads/writes `CLEDParser.List_PartItem`
and various registry keys under `Component\Mystic Light\...`. This is the
real, current GUI-to-backend channel — not MLModule's HTTP/UDP servers.

**Resolved (2026-10-17)**: decompiled `MSI.TerminalServer.exe` and its
shared library `CS_CommonAPI.dll` (both outside `Mystic Light\`, under the
main MSI Center install) to find the real transport. Port 26822 **is**
real and MSI-defined — `CS_CommonAPI.EX_TerminalServer.CommunicationPort
= 26822` — but it is **not** the transport for `Transfer_Command`/RGB
actions. It's a small custom TCP RPC protocol for MSI Center's own
"Terminal Server" role: software module download/install,
`ProcessWatcher` start/stop, display-settings/platform-info sync, toast
notifications (see `CS_CommonAPI.DataResponse.Receive`'s command list in
`MSI.TerminalServer.exe`: `ClearCache`, `ReleaseServer`, `DownloadModule`,
`RunSetupModule`, `ProcessWatcher_Start/Stop`, `UpdatePlatformInfo`, etc.)
— a generic system-utility channel, unrelated to Mystic Light specifically.

Protocol shape: each call is a **new, short-lived TCP connection** —
connect, send a framed message (`[0x10 fixed verify header][4-byte
SourceID][4-byte DestID][payload]` when routed through a `SourceID`, or
just `[4-byte DestID][payload]` otherwise), read the full response, then
immediately `Shutdown()`/`Close()`. No persistent connection, no
keep-alive — explaining why the earlier live `pktmon` capture caught
**zero packets**: it wasn't a capture failure, the specific GUI actions
tested (zone select, color change, profile switch) never needed this
channel at all, since they don't match anything in `DataResponse.Receive`'s
command list.

**Resolved: the real plugin host is `MSI.CentralServer.exe`**, not
`MSI.TerminalServer.exe` — found by decompiling it and reading
`C_DynLoad_SDK.cs`:

```csharp
Assembly assembly = Assembly.LoadFile(Path_SDK_Root + "Mystic Light\\API_Mystic Light.dll");
IPlugin_SDK plugin_SDK = (IPlugin_SDK)assembly.CreateInstance("API_Mystic_Light.PluginClass");
plugin_SDK.Transfer_StructData(PlatformInfo);
plugin_SDK.Transfer_ToAPI(FeedBackHandler);
plugin_SDK.MainEntry();
// later, per command:
struct_IPlugin_SDK._IPlugin.Transfer_Command(CommandData);
```

`Assembly.LoadFile` loads `API_Mystic Light.dll` directly into
`MSI.CentralServer.exe`'s own process/AppDomain, and `Transfer_Command` is
a plain in-process virtual method call on the loaded plugin object — no
network, no IPC, no sockets at all. This fully explains the earlier
zero-packet capture result: there was never anything to capture, by
design, not because the test missed it. `MSI.CentralServer.exe` dynamically
loads/unloads each MSI Center component this way based on registry
version keys (`C_DynLoad_SDK.LoadComponent`); it wasn't holding
`API_Mystic Light.dll` loaded at the time this was checked (no running
process had it loaded), consistent with on-demand loading rather than an
error.

**Practical takeaway for the standalone-app goal**: this network layer
(dead or not) was never the path to actual hardware control anyway — every
real color change happens through `MysticLight_AllDevice.dll`/`MSI_LED`'s
device driver classes writing raw USB HID reports (see
`MB_800_HID_PROTOCOL.md`). The HTTP SDK on 26820 would have been a
convenient *existing* entry point if it were alive; since it isn't, raw
HID is the only real option regardless.

## Still open

- Exact caller of `CWebAPI.StartHostServer()` (see `ARCHITECTURE.md`).
- IPC path for `"SignalRGB_On"`/`"MSIMLSDK_On"` (likely a named pipe or
  window message from the MSI Center GUI process to `LEDKeeper2.exe`).
