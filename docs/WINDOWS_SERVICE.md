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

Start-Lighting.cmd prefers the installed editor, then the new Service package.
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
Conflicts are checked before connection and periodically; the service never kills
another RGB application. Stop it explicitly, then rescan/resume. RAM discovery
and endpoint failures are visible in Hardware.

## Security boundary

Named-pipe requests authenticate the installation owner's Windows SID before JSON
parsing, reject network logons, cap messages at 32 MiB, time out connections and
limit concurrency. Clients verify server PID against SCM and its executable path.
Commands are limited to status/snapshot, project, scan, playback, output enable and
channel pause. No client file/DLL/module/register/shell API exists.

Program Files and state ACLs protect runtime/configuration from normal-user writes.
The three installed official module DLLs are hashed in protected service.json and
verified before loading; unexpected module DLLs fail closed. Editor plugins are
not copied into privileged execution. SYSTEM already has PawnIO access under the
existing device ACL. The signed driver and pinned module remain unchanged.

The first implementation runs the existing Qt renderer/document validator and
trusted hardware modules inside LocalSystem. This avoids a second renderer and
preserves current output behavior, but makes parser/decoder/module code part of
the privileged attack surface. This is not a full security audit or separate
low-privilege renderer. Further isolation can narrow this boundary later.

## Diagnostics and removal

Hardware shows service/per-channel failures. Startup failures reach SCM and, when
writable, `%ProgramData%/LightingStudio/service-error.log` (administrator-readable).
Administrators can inspect Get-Service LightingStudio and sc.exe qc LightingStudio.
To remove automatic operation from an administrator console:

```powershell
Stop-Service LightingStudio
sc.exe delete LightingStudio
Unregister-ScheduledTask -TaskName 'Lighting Studio Editor' -Confirm:$false
```

These commands retain installed files/playback data and shared PawnIO. User projects
are never deleted. The installed copy retains service mode while service.json
exists; a portable copy can use direct output after the service is removed.

## Verification boundaries

windows_service_contract uses fake transports and a child host in --simulate mode.
Simulation refuses SYSTEM and SCM mode and cannot discover/open hardware. Tests
cover malformed/bounded commands, atomic rejection, playback/pause persistence,
clock wrap, close-time synchronization, stale-file protection, conflicts, faults,
suspend/resume, native authenticated pipes, large snapshots and process restart.
service_installer_plan checks PowerShell parsing, missing-input rejection and WhatIf.
These are not evidence of physical colors, SYSTEM device compatibility, installer
ACL behavior, pre-login NVAPI support, actual reboot/resume or sustained frame rate.
