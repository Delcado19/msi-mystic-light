# PawnIO integration and service feasibility

Date: 2026-10-27. Investigation only; no driver, device ACL, startup setting,
application code or hardware output was changed.

Follow-up (2026-10-28): the first implementation uses a combined privileged
renderer/output service. The narrower PawnIO-only broker below remains an
isolation recommendation, not a claim about the delivered boundary. See
[Windows service operation](WINDOWS_SERVICE.md).

## Conclusion

Keep the official signed PawnIO driver and the pinned signed SMBus module.
Adapt Lighting Studio's user-mode boundary: a small privileged hardware broker
owns PawnIO handles and validates bounded RAM commands. A LocalSystem broker is
consistent with the installed device ACL; LocalService is not granted access by
default. This is source/installation evidence, not a successful service runtime test.
An independently installed renderer/output host can use this broker without
requiring an elevated editor. PawnIO alone does not implement saved projects,
RGB device discovery, animation, IPC authorization or reconnect policy.

## Exact evidence

Official sources cloned read-only for investigation under ignored build/research:

- PawnIO: `9d52965895588b0ffa3703b72eec75ba19f4ccc0`.
- PawnIO.Modules: `24425a5299e84410751da882c121f322fb030b82`.
- PawnIOLib redirect repository: `f7a71a0cd611122c10bcdafe4ba5817dc520aff4`.
  Its implementation now lives in the main PawnIO repository.
- PawnIO.Setup: `c14a8567b881a92e54d26fe00d78491f67928a58`.
  This checkout hosts releases, not installer implementation; silent installation
  switches, child elevation and reboot behavior were not established from it.

Installed driver: Windows DriverStore pawnio.inf_amd64_a72a2f969b8b7496/PawnIO.sys.
SHA-256: `fca6e7d58b0cf38dbb913a2b9e532f48629145d395f454b16a9f58e97b8d3940`.
`sc query PawnIO` reports RUNNING; `sc qc PawnIO` reports KERNEL_DRIVER,
DEMAND_START. Demand start is the configuration, not proof it must be started
manually at every boot: this is a root-enumerated PnP device (`Root\PawnIO`).
Installed INF DriverVer is 03/11/2026,2.2.0.0.
Authenticode verification returned Valid, Microsoft Windows Hardware Compatibility
Publisher. This does not prove equivalence to the current public source snapshot.

## Access control and process boundary

Installed INF and upstream PawnIO/PawnIO.inf.in agree:
`D:P(A;;GA;;;SY)(A;;GA;;;BA)` grants SYSTEM and built-in Administrators access.
Do not confuse driver-device access with permission to control its SCM service.
PawnIOLib/PawnIOLib.cpp::pawnio_open_nt requests GENERIC_READ | GENERIC_WRITE
with NtOpenFile on the named device. It does not show UAC, start a service,
load a driver or grant privileges. Driver IRP_MJ_CREATE itself returns success;
the device ACL is the observed external access boundary. IOCTLs use
FILE_ANY_ACCESS; handing a live handle to an untrusted client would therefore
undermine the broker boundary. Never export that handle or a generic execute API.

A SYSTEM broker can retain the original driver ACL. Expose authenticated local
IPC with only device enumeration/status and validated RGB frames for qualified
identities. Keep arbitrary module paths, Pawn function names, bus addresses,
register writes, executable paths and shell commands out of the client protocol.
Protect service binaries, dependencies and configuration. Do not load arbitrary
editor driver plugins into the privileged process. Service SID restrictions,
shared mutex access and the actual effective token still require runtime testing.

## Signed modules and customization

PawnIO/src/vm.cpp verifies a module against embedded trusted public keys before
constructing its VM. A locally modified module is not accepted merely because
our executable is signed or elevated. PAWNIO_UNRESTRICTED bypasses verification
in a different build; it is not the production integration path.
Official module contribution guidelines require upstream acceptance, documented
IOCTLs, supported-hardware checks and LGPL-2.1 licensing. Upstream controls when
accepted modules are built, signed and released. A custom driver fork would also
need its own acceptable Windows driver signing/deployment path.

Current SmbusPIIX4.bin remains pinned to upstream 0.2.11 (42,404 bytes,
SHA-256 91f9b4b1c39e3d399ce48477a89d8f6bd3e58a2241064a4daec3a1513dff56e5).
Compared the bundled source archive's SmbusPIIX4.p and sleepmode.inc against the
current checkout: identical lines after newline normalization. No newer behavior
is assumed for other files or the installed driver.

The module already exposes controller identity, port selection, SMBus transfers,
clock frequency and sleep mode. It validates read/write direction and 7-bit bus
addresses, but is a generic bus transport, not an ENE RGB-only authorization layer.
Our broker must preserve the narrower existing PCI 1022:790b, port 0, RAM
0x71/0x73, AUDA0-E6K5-0101, eight-LED checks and prohibit SPD/flash/remap operations.
Future devices need explicit protocol qualification, not an arbitrary scan sweep.

## Performance and recovery

The module defaults to AlwaysBusy waits. ioctl_set_sleep_mode supports ShortBusy
and AlwaysSleep without rebuilding it. Whether a different mode reduces CPU cost
without harming frame timing requires measurement on the target board.
The current ENE host performs 4 transfers for direct activation and 16 for eight
LED payloads per frame, plus 2 port selections (22 calls per module, excluding
first-frame readback). Do not promise a frame rate without timing actual transfers.
Batching several validated ENE operations in a signed module could reduce kernel
round trips, but requires upstream work and hardware validation. Start with the
existing interface; measure before introducing this dependency.

The module documents the shared Access_SMBUS.HTP.Method mutex requirement;
Lighting Studio already holds Global\Access_SMBUS.HTP.Method across complete
operations and restores port selection. A service changes creator/token context,
so cross-session mutex permissions must be tested. Its lock cannot coordinate
programs that ignore the convention. Suspend/resume, lost handles and reconnect
must reopen/reverify hardware and restore volatile direct mode before output.

Current runtimeAvailable checks the library ABI version, not actual driver
reachability. Keep library/driver/permission/identity failures distinct in service
status. Do not report RAM ready solely because the DLL exists.

## Installation and licensing boundary

The official module-usage wiki distinguishes GPL-2.0-or-later driver source with
exception, LGPL-2.1 library/module source, and proprietary official binary editions;
it expressly permits redistribution of the installer. This is not blanket
permission to repackage modified official driver binaries. Review exact installer
terms before bundling. The current project redistributes only the signed module
and corresponding source, not PawnIO's installer/driver/library.

One elevated application installer can provision our service and prerequisites;
actual one-prompt behavior remains to be tested against the official installer,
reboot conditions and system policy. An installed authorized service can handle
later supported-device scans without requesting UAC again.

## Coverage and next gate

REA open_binary identified the installed native x64 PE and SHA-256.
No retained Evidence ID was supplied by that operation. binary_session initially
reported no configured Ghidra/IDA provider. The user's Trinity checkout identified
the existing Ghidra 12.1.3 installation under LOCALAPPDATA/Programs/Ghidra and
Temurin JDK 21.0.12.101. A direct analyzeHeadless.bat invocation then successfully
imported and analyzed the installed driver without executing it. REA's missing
environment configuration did not mean Ghidra was absent.

Retained local evidence: build/research/pawnio-ghidra.log,
pawnio-installed-decompiled.txt, ghidra-scripts/PawnIoEvidence.java, and
Ghidra project PawnIO-Installed under build/research/ghidra-project.
The export contains 477 function entries. Selected reviewed paths:

- 0x140001e80: entry registers AddDevice and IRP dispatch handlers.
- 0x1400019c0: IoCreateDevice and named PawnIO symbolic links.
- 0x140001bd0: create/close/load/execute/version IRP dispatch; version 0x20200.
- 0x140010ea0 -> 0x140010fe0: module-load wrapper and checked loading path.
- 0x140010fe0 -> 0x140003320: SHA-256 calculation.
- 0x140010fe0 -> 0x140003480: trusted-key signature check. Failure returns
  before allocating and initializing the VM; no unconditional bypass appears
  on this reviewed path.
- 0x140003480: imports RSA public key and calls BCryptVerifySignature.

This confirms the installed build's relevant signature boundary, not whole-source
binary equivalence or a complete driver security audit. Ghidra lacked PDB symbols
and dependency images, reported pcode warnings at 0x1400027e0/0x1400031b0 and
several indirect-jump/type-recovery warnings. Decompiled parameter types are
inferred, including incomplete IoCreateDevice prototypes; source and installed
INF corroborate the contract. No runtime tracing is claimed.
Initial sandbox CIM/security-module queries failed; an elevated read-only signature
check succeeded. SCM/registry/INF reads supplied configuration and running status.
No hardware probing, module execution, process termination, installation, access
policy modification or startup modification occurred. No application tests were
rerun because executable code was unchanged.

Next gate: implement a minimal fixed-command broker, then explicitly qualify
SYSTEM access, mutex behavior, driver availability after cold boot and resume,
and physical RGB output. Only then connect automatic project playback and editor.

## Primary sources

- https://github.com/namazso/PawnIO/blob/9d52965895588b0ffa3703b72eec75ba19f4ccc0/PawnIO/PawnIO.inf.in
- https://github.com/namazso/PawnIO/blob/9d52965895588b0ffa3703b72eec75ba19f4ccc0/PawnIOLib/PawnIOLib.cpp
- https://github.com/namazso/PawnIO/blob/9d52965895588b0ffa3703b72eec75ba19f4ccc0/PawnIO/src/vm.cpp
- https://github.com/namazso/PawnIO.Modules/blob/24425a5299e84410751da882c121f322fb030b82/SmbusPIIX4.p
- https://github.com/namazso/PawnIO.Modules/wiki/Contribution-guidelines
- https://github.com/namazso/PawnIO.Modules/wiki/Using-PawnIO-Modules
- https://pawnio.eu/
