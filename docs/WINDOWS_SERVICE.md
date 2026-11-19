# Windows service operation

Date: 2026-10-28. Software-tested implementation; real SYSTEM hardware output,
cold boot, standby and a clean-install UAC count remain to be qualified.

## Install and update

Extract the complete Release archive and run **installer/Install.cmd**.
The entry point requests elevation once, installs into
`%ProgramFiles%/Lighting Studio`, creates the automatic LocalSystem service
`LightingStudio`, and registers a Limited interactive logon task for the installing
user. A Start-menu shortcut opens the editor after installation. The editor starts in the tray with `--background`. `-NoEditorAutostart`
omits the editor task; background lighting is independent of that task.
Microsoft Visual C++ x64 runtime remains a prerequisite.

```powershell
./installer/Install-LightingStudio.ps1 -Source ./build/next/Release -WhatIf
./installer/Install-LightingStudio.ps1 -Source ./build/next/Release
```

WhatIf validates runtime inputs before elevation/system changes. Close the installed
editor before updating. Pre-existing untrusted ownership/write ACLs or reparse entries are rejected.
Failed installs terminate with an error; an interrupted
update can leave the service stopped. Correct the reported cause and rerun from
an administrator console. Existing playback data and user projects are retained.

If PawnIO is absent, the installer downloads the unchanged official 2.2.0 installer,
checks SHA-256 `1f519a22e47187f70a1379a48ca604981c4fcf694f4e65b734aaa74a9fba3032`,
and invokes `-install -silent` from the already elevated parent. The flags were
verified against the pinned installer binary's embedded usage text; `/S` is not its CLI. `-PawnIOSetup <path>`
accepts the same official file for offline setup. No unsigned/custom driver or
certificate is installed. Driver repairs and protected updates can require UAC
again. The exact one-prompt flow still needs observation on clean Windows, including
reboot/policy conditions. Shared PawnIO device permissions are not changed.

Start-Lighting.cmd (development launcher) starts the newest local build of build/r4, build/routing
and build/aurora; without one it starts the installed editor. Older build/dist packages are no
longer used as fallbacks.
Current portable builds also detect a registered service and route to it, preventing
a second direct hardware writer. Installed mode fails closed when service access
fails. Older portable binaries cannot provide that protection; close them before
installing. Without service installation, portable direct RAM output still requires
an administrator process.

## Playback and editing

The service reuses LightingDocument, the renderer and output schedulers, validates
a complete replacement before atomically saving
`%ProgramData%/LightingStudio/playback.json`, and advances its own animation clock.
No project means no color writes. Closing the editor leaves the last confirmed
project running. The bounded one-day animation clock wraps for continuous operation.

Editor changes coalesce before upload. Reopening restores the service snapshot
instead of overwriting it with a stale remembered project file. A service snapshot
opens unnamed and does not rewrite a user file. Explicit recovery/new edits while
connecting take precedence. Discovery follows snapshot restoration to avoid an
older automatically discovered layout replacing active playback. Pending edits
retry after transport errors; close flushes final commands and warns on failure.

Scan hardware runs through the service without another UAC prompt. Supported
integrated devices use existing detection/geometry. Fans and strips on headers
still need configured type/count/order; those are generally not electrically
identifiable. Privileges cannot add unsupported device protocols.

Pause selected / Resume selected affect channel rows. Stop all output closes
connections and suspends retries; Resume all output reconnects. Pause/stop persist
with the project and survive edits/restarts. Stops retain last applied device colors.
Identity checks and latest-frame scheduling remain in force. Missing/faulted devices
retry every five seconds. Suspend releases sessions; resume reopens/reverifies them.
Conflicts are detected per device, not by program name. Before each connection
attempt the service probes every configured HID endpoint it does not hold itself
and has discovered itself as a supported lighting device (never a bare path from
the project) with an exclusive open. A busy endpoint gets a 5 s real-time USB trace
(Microsoft-Windows-USB-UCX and -USBHUB3) at most once a minute. Only a process other
than Windows or Lighting Studio that dispatches a HID SET_REPORT to that device in
the trace blocks it; holding the device open or reading from it does not (MSI
Center's `CC_Engine_x64.exe` only reads the MSI controller). The trace names the
sender even when it hides its handles, as SignalRGB does (verified on the owner's
system 2026-11-16). The event layouts are undocumented and were observed on Windows
11 build 26300; if a trace yields no device events, detection is reported as
unavailable in Hardware and nothing is blocked. Known gaps: writes through interrupt
transfers are not evaluated (SignalRGB writes the ROCCAT Vulcan that way, so that
keyboard is not blocked), a program that writes only occasionally (LEDKeeper2 on
"Apply") is usually missed in 5 s, and RAM (SMBus) and GPU endpoints are not
covered. The periodic guard does not probe an endpoint the service already drives; the
editor's conflict dialog checks every HID device the service discovered, with or without a
project and including the driven ones (the trace drops the service's own writes), without
the busy pre-check, so a program that opens the device only per write is not missed. Writes are
matched to the physical USB device (instance ID via the HID collection's parent), not only VID/PID,
and a process created after the last captured write (reused PID) gets no writer proof. With a
guard, an output is opened only after a scan probed its endpoint. An empty
inventory or a failed trace is shown in the dialog, never treated as "no conflict". The service never stops
another application on its own; it stops one only on an explicit request from the
editor's conflict dialog (see Diagnostics and removal). RAM discovery and endpoint
failures are visible in Hardware.

Hardware discovery and the conflict check run on a worker thread, one scan at a
time, so the pipe keeps answering while a scan takes seconds. Status commands
and project commands reply immediately (a project command still starts a fresh scan);
scan and output-enable commands reply after a scan that started after the command. A
scan still running after 12 s is abandoned: its callers get the reply with an error,
and no new scan starts until the abandoned one returns. The deadline stays below the
editor's 15 s request timeout so that reply arrives; a scan already queued behind a
running one can still exceed it, and the error then shows in the next status poll. Connections are still opened on the service
thread, between two scans.

A device open or HID call can hang inside the driver without timeout (2026-11-17: the MSI
controller never answered `HidD_GetSerialNumberString`). Stopped output workers are never
joined on the service thread while alive: after one bounded second a hung worker is set
aside, its endpoint reports **Not responding** and is not reopened until the call returns, and
the pipe keeps answering. A worker that hung earlier is not waited for again. On service stop a
still-hung worker is abandoned, and a discovery scan still running after 10 s too. A hung discovery
scan (hidapi also reads serial numbers while enumerating) delays scan replies by at most the
12 s scan deadline; meanwhile no devices are rescanned or reopened.

## Security boundary

Named-pipe requests authenticate the installation owner's Windows SID before JSON
parsing, reject network logons, cap messages at 32 MiB, time out connections and
limit concurrency. Clients verify server PID against SCM and its executable path.
Commands are limited to status/snapshot, project, scan, playback, output enable,
channel pause, identify (2 s flash of one known software component; the service
expires it itself and never persists it), and the conflict list with its stop
request. A stop request names a process by PID and creation time only; the service
acts on it only if its last device detection listed that process. Any process of
the owner's account can send that request, so the listed process must also either
run as the owner without elevation (no rights gained), have been seen writing to one
of our devices, or be a known lighting program with its own verified stop path.
Family members with higher rights are refused with "stop it manually". No client file/DLL/module/register/shell API exists.

Program Files and state ACLs protect runtime/configuration from normal-user writes.
The six installed module DLLs (`msi_gen1`, `peripherals`, `auxiliary`,
`asrock_polychrome`, `lenovo_motherboard` and `hp_omen`, each with the
`_module.dll` suffix) are hashed in protected service.json and
verified before loading; unexpected module DLLs fail closed. Editor plugins are
not copied into privileged execution. SYSTEM already has PawnIO access under the
existing device ACL. The signed driver and pinned module remain unchanged.

The first implementation runs the existing Qt renderer/document validator and
trusted hardware modules inside LocalSystem. This avoids a second renderer and
preserves current output behavior, but makes parser/decoder/module code part of
the privileged attack surface. This is not a full security audit or separate
low-privilege renderer. Further isolation can narrow this boundary later.

## Diagnostics and removal

The service retains healthy output workers when other saved devices are offline.
A project command replies once the project is saved and loaded; the hardware scan follows
and its result appears in the next status poll.

Five-second retries target missing or faulted output scopes rather than reopening
every controller. Global discovery, conflict-guard or connection-preparation errors
still stop all workers; they do not bypass the output guard.

The installed editor shows the conflict dialog whenever its window is opened while
another program writes to one of the configured devices: on a start from the Start
menu and on **Show editor** from the tray, not on the silent logon start. While the
editor is open, **Resolve conflicts...** in Devices opens the same dialog and then
rescans; Hardware names the writers and points to that button. Each row names a
writer and the devices it writes to, followed by the processes of the same program:
its parent and child processes inside the same install folder (never Windows images
or Lighting Studio), topmost first, so a launcher stops before the program it would
restart. Program names never make a process a conflict. While a device is busy the
dialog waits for one 5 s trace. The service performs the stops, because the unelevated editor cannot stop services
such as `Mystic_Light_Service` (which restarts LEDKeeper2) or `SignalRgb.Service`.
The editor first posts a normal close request to the program's windows; the service
then stops a process that hosts its own service through the service manager and
terminates any other. Known programs keep their dedicated paths (SignalRGB service,
`Mystic_Light_Service` before LEDKeeper2). A row the service may not stop (a related
process with higher rights that was not seen writing) is marked "administrator approval
required to stop": Terminate then runs `lighting_service_host.exe --stop-process <pid>
<started>` through UAC, one approval per process. The helper re-reads the live process,
refuses Windows images and Lighting Studio, and stops it like the service would
(exit 0 stopped, 1 failed, 2 bad arguments, 3 identity changed, 4 refused). After a
successful stop (by the service or through UAC) the service reopens all outputs once,
1.5 s later, so each device is initialized again; a stopped program can leave a device in
its own mode (Roccat Vulcan flickered after SignalRGB was stopped, 2026-11-17). Every stop is for now only: start types,
autostart entries and scheduled tasks are never changed, so the other software
returns after a reboot or when its own launcher starts it. The portable editor
still uses its name list.

The portable startup dialog offers a separate **Stop SignalRGB service
(administrator)** action. It reads the actual executable path from the Windows
service configuration, not a fixed installation directory, and validates the
exclusive process for the fixed service name `SignalRgb.Service`. Close the
SignalRGB application first: it can start the service again. The action launches
only Windows' system `sc.exe` with `stop SignalRgb.Service` after UAC consent.
It checks the command result, service state and disappearance of the original
service process, including a short stable-stop observation. Restarts, denied
access, cancellation, query errors and timeouts remain visible. It never force
kills the service, disables a task, changes its start mode or grants permissions.
Canceling the dialog cannot recall a stop already approved through UAC. Automatic
startup and vendor tasks may start SignalRGB again later.

Fake service tests are not proof of an actual UAC stop. Microsoft documents the
[service access rights](https://learn.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights)
and the [ShellExecuteEx runas/handle contract](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow).

Hardware shows service/per-channel failures. Startup failures reach SCM and, when
writable, `%ProgramData%/LightingStudio/service-error.log` (administrator-readable).
Administrators can inspect Get-Service LightingStudio and sc.exe qc LightingStudio.
## Removal

Remove Lighting Studio in Windows Settings > Apps, or run `installer/Uninstall.cmd` from the
package. Both start `Uninstall-LightingStudio.ps1`, which requests elevation once and removes the
LightingStudio service, the `Lighting Studio Editor` logon task, the Start menu shortcut,
`%ProgramFiles%/Lighting Studio`, `%ProgramData%/LightingStudio` and the Apps entry.

It also deletes the editor settings (`HKCU/Software/Lighting Studio/Editor`), copies every `*.json`
found anywhere below `%LOCALAPPDATA%/Lighting Studio` (unrenamed, with its subfolders) to
`%USERPROFILE%/Lighting Studio - Backup - yyyy_MM_dd`, and then deletes that folder, other files
included, without asking. The result is shown last in a highlighted box: the backup path, or that no
file was found. A failed copy leaves the folder and the settings untouched. This runs with the rights
of the user who starts the uninstaller, not in the elevated step; a portable copy shares these settings.
Started from a console that already has administrator rights, the uninstaller leaves these files and
settings alone and names their folder in the box, because copying them with an administrator token is unsafe. It
refuses to start while any copy of the editor runs, portable ones included.
Saved projects elsewhere (for example in Documents) are not touched.

The only question is whether to remove the PawnIO driver too (`-RemovePawnIO`; `-Quiet` skips it). It
is removed through PawnIO's own registered `uninstall.exe -uninstall -silent` and is shared: OpenRGB,
Fan Control, LibreHardwareMonitor and ZenTimings use it too.

Saved project files are never deleted. The uninstaller refuses to start while the editor runs, so its
own Save/Discard prompt is never skipped. Steps that fail are listed and the script exits non-zero.
The removal is covered by a parse/WhatIf test only; it has not been run against a real
installation yet.

## Verification boundaries

windows_service_contract uses fake transports and a child host in --simulate mode.
Simulation refuses SYSTEM and SCM mode and cannot discover/open hardware. Tests
cover malformed/bounded commands, atomic rejection, playback/pause persistence,
clock wrap, close-time synchronization, stale-file protection, conflicts, faults,
suspend/resume, native authenticated pipes, large snapshots and process restart.
service_installer_plan checks PowerShell parsing, missing-input rejection and WhatIf.
These are not evidence of physical colors, SYSTEM device compatibility, installer
ACL behavior, pre-login NVAPI support, actual reboot/resume or sustained frame rate.
