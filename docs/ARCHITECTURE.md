# Mystic Light — Overall Architecture

As of 2026-10-17. All findings from static analysis (ILSpy decompilation,
.NET 4.8, no runtime observation). Paths in `File:Line` format refer to the
decompiled source in the scratchpad of the respective session (not part of
this repo) — for traceability, not as a copy.

## Components

| File | Type | Role |
|---|---|---|
| `Mystic_Light_Service.exe` | Windows service (.NET 4.8) | Starts/monitors `LEDKeeper2.exe` and `muachost.exe` via Task Scheduler |
| `LEDKeeper2.exe` | WPF/WinForms app (.NET 4.8) | Main app: device driver logic, network servers, profiles |
| `MysticLight_AllDevice.dll` | .NET library | Standalone device driver SDK, partially a code duplicate of LEDKeeper2's `MSI_LED` namespace |
| `MLModule.dll` | .NET library (**managed** despite the name, not native) | Network core: HTTP SDK, UDP pairing, UDP party sync |
| `SyncData.dll` | .NET library | Profile data model + simple AES encryption for local config files |
| `Lib\MsiHid_GameSync.dll` | native DLL | Generic USB HID access (P/Invoke target of `HID_Basic`) |
| `API_Mystic Light.dll` | .NET library, MSI Center plugin | Current, live GUI-to-backend bridge (`IPlugin_SDK`), hosted in-process by `MSI.TerminalServer.exe` (part of the main MSI Center install, not `Mystic Light\`) — see `NETWORK_PROTOCOL.md` |

## Mystic_Light_Service.exe

Runs as a `LocalSystem` service. Does not bypass Windows Session-0 isolation
via token duplication, but via the Windows Task Scheduler COM API
(`ITaskService`/`ITaskDefinition`, `TASK_LOGON_S4U`) to start `LEDKeeper2.exe`
in the logged-in user's desktop session:

- `CreateTask_Keeper` registers the task "MSI Task Host - LEDKeeper2_Host",
  `Path = AppDomain.CurrentDomain.BaseDirectory + "LEDKeeper2.exe"` (fixed,
  from the service's own install directory — no external input).
- A second task ("MSISW_Host") starts `muachost.exe` from
  `Environment.GetFolderPath(SpecialFolder.SystemX86)`.
- `OnCustomCommand(int Value)` (values 201-204) controls start/stop/monitor
  of these tasks — driven by the MSI Center GUI process.

**Checked and dismissed**: `Class_CallExternal.StartProcessByUserUAC`
(classic `OpenProcessToken`/`DuplicateTokenEx`/`CreateProcessAsUser` chain
with `lpApplicationName=null`, `applicationName` used as raw
`lpCommandLine`) is defined in the service, but **has no call site** — dead
code in this build. No active attack vector through this path.

## LEDKeeper2.exe

Contains the actual device driver tree (`MSI_LED` namespace, ~80 device
driver classes: keyboards, mice, headsets, mainboards, DRAM, monitors, PSUs,
fans) plus third-party protocol implementations (SteelSeries, Corsair
Vengeance DRAM, GALAX, G.Skill, Patriot, Kingston, Apacer, Micron). Details
on the "800-series" mainboard protocol (affects the B850 board in
`OpenRGBResearch/msi-b850`, among others) are in
[`MB_800_HID_PROTOCOL.md`](MB_800_HID_PROTOCOL.md).

Network functions in LEDKeeper2.exe (`RGBControlClass.cs`): see
[`NETWORK_PROTOCOL.md`](NETWORK_PROTOCOL.md) — this is the SignalRGB bridge
that internally forwards per-LED reports to the board drivers.

## MLModule.dll — network core

Despite the `.dll` naming convention, this is not a native library but an
ordinary .NET assembly (`"managed": true`, Ghidra correctly rejects it as a
target for this reason). Contains three independent servers, see
[`NETWORK_PROTOCOL.md`](NETWORK_PROTOCOL.md) for details:

1. TCP 26820 — local HTTP "Web API" (`CWebAPI.cs`), **no authentication**.
2. UDP 26822 — mobile app pairing (`CBaseServer.cs`/`CBaseFN.cs`).
3. UDP 26823 — multi-PC "Party Sync" (`CSyncServer.cs`/`CSyncFN.cs`).

Resolved (2026-10-17): `CWebAPI.StartHostServer()` has **no call site
anywhere** — checked across `LEDKeeper2.exe`, `Mystic_Light_Service.exe`,
`MysticLight_AllDevice.dll`, and `API_Mystic Light.dll` (the one other
assembly that references the `MLModule` namespace at all). All three
`MLModule.dll` servers (`CWebAPI`, `CBaseServer`, `CSyncServer`) are dead
code, superseded by the `API_Mystic Light.dll` plugin architecture — see
`NETWORK_PROTOCOL.md` for the full explanation.

## SyncData.dll

Pure data model (7 types, negligible logic: enums for style/cycle/device
name/chipset, a `DataBase` class). `C_Encrypt.cs` implements AES encryption
for local configuration files (profiles/monitor configs) — **the IV is
derived via MD5 from the same key used for AES** (cryptographically unsound,
since IV and key are derivable from each other), but only used to obfuscate
local files, not a network secret. No further action needed for the
standalone-app goal.

## HID password mechanism (`HID_Basic.init`)

`MysticLight_AllDevice.dll` and several `HID_Basic` variants in
`LEDKeeper2.exe` (`MsiHid/HID_Basic.cs`, `MsiHid_GM41W/HID_Basic.cs`, etc.)
define `init(string psw)` — a 7-byte "password" that can be passed to the
native `MsiHid_GameSync.dll` before actual HID access.

**Finding: none of the checked caller files (all of `LEDKeeper2.exe` and
`MysticLight_AllDevice.dll`) actually call `init(...)`.** Dead/vestigial
feature. This matches the observation already documented in
`OpenRGBResearch/msi-b850` that real `hidapi` sends succeeded without any
init call — for a standalone app this step is very likely **not** required,
at least not for the device classes checked here. (Not checked:
`HID_Basic_KB.cs`, `HID_Basic_Mouse.cs`, `HID_Basic_MNT.cs`,
`HID_Basic_PSU.cs` — the grep ran across the whole directory, but none of
these files themselves contains an `init(` call to another instance; if
some keyboard/mouse-specific path does need the password, that was not
individually traced here.)
