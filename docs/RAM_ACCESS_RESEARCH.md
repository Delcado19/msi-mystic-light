# Windows RGB access investigation

Current decision (2026-10-24): the user has approved lightweight direct PawnIO
integration. Earlier exclusions and recommendations below are historical. The
implementation now uses installed PawnIOLib plus the signed AMD module, without
OpenRGB or a helper process. See ../third_party/pawnio/README.md.

Date: 2026-10-24. Scope: installed SignalRGB 2.5.80, installed MSI Center
Mystic Light, and local OpenRGB source at commit
`547ca943d1507f5005bbffb59e00dc777131c66b`.

Constraints: no expenditure, no OpenRGB/PawnIO runtime dependency, no Windows
test-signing mode or weakening of system protections. No driver was installed,
started or invoked, and no device color/register writes were performed.

## Findings

| Target | Observed access | Implication |
| --- | --- | --- |
| SignalRGB | Running `SignalRgbDriver` service; `System32/drivers/SignalRgbDriver.sys`, 55,768 bytes; Authenticode status Valid, SignalRGB signer | A small existing kernel driver exists, but no independent supported client contract or redistribution permission was established. |
| OpenRGB | `i2c_smbus/Windows/i2c_smbus_pawnio.cpp` calls `pawnio_open`, `pawnio_load`, and `pawnio_execute`; AMD uses `SmbusPIIX4.bin` | Copying the ENE controller protocol does not remove the need for a Windows bus transport. This transport is excluded by user instruction. |
| MSI/ENE | `SB_SMBUS_SDK.dll` exports `SB_SMS_GetCtrlCountEx`, QuickCommandEx and Byte/Word/Block Read/WriteEx; binary contains AMD SMBus class names and `\\.\EneTechIo` | A concrete bus API candidate exists, but signatures, initialization behavior and platform compatibility remain unverified. |
| MSI alternative | `Lib/SMBus_Engine.dll` exports initialization, release, byte/block reads/writes and bus reset functions; `MSIEneHal.dll` exports DRAM initialization functions | Another candidate exists; exported names alone do not establish a safe callable API. Do not call reset/initialization blindly. |
| AMD chipset | AMD SMBUS PCI `1022:790B`; installed `oem87.inf`, AMD Interface 2.0.0.29; matching service section has empty `AddService` | This installed INF supplies identification without a functional SMBus service. It is not a usable user-mode SMBus API. |
| RTX 5080 AERO | Existing app backend uses NVIDIA NVAPI I2C through the installed graphics driver | Retain this path; no additional kernel driver/signing project is needed. Physical lighting validation remains outstanding. |
| MSI board/IOShield | Existing app backend uses HID and volatile MSI zone reports | Retain the direct backend; RAM access should not replace it. |

### SignalRGB protocol and driver evidence

Installed source reference:
`%LOCALAPPDATA%/VortxEngine/app-2.5.80/Signal-x64/Plugins/Asus/ENE_RAM.js`.
It declares SMBUS, samples the effect using `device.color`, and delegates bus
operations to host-provided `bus.WriteWord/WriteByte/WriteBlock` methods.
It recognizes `AUDA0-E6K5-0101` as V2, direct mode register `0x8020` and color
buffer `0x8100`, matching the local OpenRGB ENE controller implementation.
This establishes agreement on the controller protocol, not a callable Windows
transport. Its scan can write/remap addresses; it was not executed.

Ordinary offline string inspection of SignalRgb.exe found an embedded resource
path `:/files/Resources/Driver/SignalRgbDriver.sys` and the diagnostic
`New SignalIO driver doesn't require Winring0...`. The installed driver contains
`\Device\SignalIo`, `\DosDevices\SignalIo`, `IoCreateDeviceSecure`,
`IoValidateDeviceIoControlAccess`, and SignalRgb.exe/SignalRgbService.exe names.
These are static observations. Whether it enforces client identity, its IOCTL
layout and whether third-party use is supported are unknown. No access-control
bypass or driver client was attempted.

### MSI deployment limitations

`SB_SMBUS_SDK.dll`, `Lib/SMBus_Engine.dll`, and `MSIEneHal.dll` are PE32 x86.
They cannot be loaded directly into this x64 process. Using them would require
a separate x86 process or another compatible interface, plus a supported driver
and an understood API. This is an additional dependency, not a standalone DLL fix.

`EneIo64.sys` contains the matching `EneTechIo` device names and has a Valid
Authenticode result, ENE TECHNOLOGY INC. signer. No matching running ENE service
was found in the current inventory. Running MSI NTIOLib services and MSIO were
found, but no call chain from those services to this RAM controller was proven.
Valid signatures do not prove compatibility, security or redistribution rights.

The public Mystic Light SDK expressly requires MSI applications to be installed.
It is distinct from the private lower-level DLL exports examined here.

## Recommendation

Keep HID for motherboard/peripherals and NVAPI for the qualified GPU. Do not
introduce a common replacement driver for devices that already have direct access.

For RAM, a small native ENE controller is feasible once a supported bus transport
exists. No zero-dependency, zero-cost Windows transport was established by this
investigation. The next useful technical step is static call-chain/ABI analysis
of MSI's bus engine and the existing SignalIO driver, followed by checking the
supported-use/deployment contract. Prefer an already installed, supported driver
interface over an entire RGB application, if one can be established. Do not
promise that either proprietary driver may be redistributed or called independently.

The public MSI SDK is an integration option only if MSI application dependency
is accepted; SignalRGB's public API controls its running application and is not
evidence of an independent SMBus API. Neither is the desired standalone result.
Do not fall back to old WinRing0/generic vulnerable drivers or disable protections.

## Evidence and limitations

REA `open_binary` identified SignalRgb.exe as native x86-64 PE. The native analysis
provider is unavailable: Ghidra is not configured, IDA is not configured, and
Hopper is unsupported on Windows. Therefore no REA decompilation, driver dispatch
analysis or complete call-chain analysis was performed. The export/PE/string
observations above came from read-only Python standard-library inspection; service
and signature observations came from CIM and Get-AuthenticodeSignature.
No analysis engine was installed. This is a bounded static comparison, not proof
of working RAM output or a complete reverse engineering of the applications.

SHA-256 identities:

- SignalRgb.exe: `cd5d6ae579afb4eb2cf0055cfa4ade62308625d47099535bcb4ddbb242b58ed8`
- SignalRgbDriver.sys: `90b3295e16016f32d5335cdf09804d0f81f94f6b49ffa555e8fb60ab43a7b8e6`
- SB_SMBUS_SDK.dll: `0311a189945f00e4a98403e1d58cef8cd99414702c355c44bc621be95fa82f33`
- SMBus_Engine.dll: `5506f54ec6731c601cd17c8a6419774491b4bc1b65bb5e8d2e4293edbc05a045`
- MSIEneHal.dll: `c9abf27ce175ec176bdb6ea1c5664b16786b07a5bda4b5e026a70332fa95e867`
- EneIo64.sys: `9ee33ffd80611a13779df6286c1e04d3c151f1e2f65e3d664a08997fcd098ef3`

Official references checked:

- https://docs.signalrgb.com/developer/plugins/determining-device-type/
- https://docs.signalrgb.com/quick-start/how-to-install/
- https://docs.signalrgb.com/developer/signalrgb-api/lighting/
- https://storage-asset.msi.com/files/pdf/Mystic_Light_Software_Development_Kit.pdf
- https://learn.microsoft.com/en/windows/security/threat-protection/windows-defender-application-control/microsoft-recommended-driver-block-rules

No binary or third-party implementation was copied into the app during this work.

## SignalIO driver follow-up: static dispatch analysis

The same driver SHA-256 was examined with the already installed Microsoft
`dumpbin.exe` 14.51.36260.0 (`/headers /imports /disasm`). This works without a
REA deep provider. Local raw disassembly: `build/signal-driver-disasm.txt`.
Addresses below use preferred image base `0x140000000`; they are not live kernel
addresses. The PE timestamp is July 23, 2026, not an independently verified release date.

The driver is not a generally usable third-party SMBus interface:

- Initialization at `0x140001143` registers create dispatch `0x140001C40`;
  `0x14000114E` registers device-control dispatch `0x140001E30`.
- Create dispatch defaults to `STATUS_ACCESS_DENIED` (`0xC0000022`), retrieves
  the caller image using `SeLocateProcessImageName`, and compares its suffix
  against a two-entry table at `0x140007000`. That table points to
  `\SignalRgb.exe` (`0x1400054C0`) and `\SignalRgbService.exe` (`0x1400054E0`).
- Only a matching name reaches `0x1400017B0` (call at `0x140001D02`). This routine
  obtains the current process image file, opens the path, compares file identity,
  and calls the PE/signature validation path at `0x14000378C` via
  `0x1400019C8`. The success status is required before allocating per-handle
  caller context at `0x140001D1D` onwards.
- The validation path calls `BCryptVerifySignature` at `0x140003AF5` through
  the thunk at `0x140003B95`; it tests the returned status at `0x140003B06`.
  Its public-key import path includes `BCryptImportKeyPair` at `0x140002E3C`.
  The exact accepted signer/key policy was not fully reconstructed, so this
  does not assert that a generic Authenticode certificate would be accepted.
- Device-control dispatch checks the stored process against
  `IoGetCurrentProcess` (`0x140001E8E` to `0x140001E9A`) and denies mismatches.
  Thus transferring a handle to another process is not an independent-client API.
- The device security descriptor includes authenticated-user read/write, but
  that does not override the create-dispatch checks above.
- Hardware paths contain port IN/OUT instructions, an MSR read and PCI config
  access, with port/buffer validation. The reviewed dispatch is a restricted
  low-level hardware interface, not a simple exported ENE/RAM API.

Conclusion: using the existing SignalIO driver as an ordinary backend for this
app is not a supported or viable integration established by these artifacts.
It has explicit caller gating beyond a filename string. No renaming, patching,
injection, signature bypass, handle transfer experiment or hardware IOCTL was
attempted. Obtain a supported vendor integration if this route is pursued;
otherwise continue investigating MSI's separate interface. The public SignalRGB
app API remains a different, application-dependent option.

Limitations: static control-flow interpretation, not runtime proof or a complete
security audit. The accepted certificate/key policy and every hardware operation
were not reconstructed. App source and runtime behavior remain unchanged.

## MSI NTIOLib follow-up

Offline Microsoft dumpbin analysis identified a more promising installed-driver
candidate. This supersedes the earlier uncertainty about whether MSI has a
non-ENE bus-engine route; it does not establish successful hardware access.

- `Lib/SMBus_Engine.dll` does not import DeviceIoControl. Its exported
  `SMBusInitialization` at preferred VA `0x10007380` accepts two stack arguments
  (`ret 8`). Constructor `0x100082D0` stores the first argument as a callback
  table; initialization `0x100086A0` calls a function at table offset `0x38`,
  compares the result against Intel vendor `0x8086`, and selects different bus
  implementations. Non-Intel selection exists, but the complete AMD device/base
  discovery path has not yet been validated against this board.
- `Lib/MBAPI_x86.dll` contains references to `SMBusInitialization`,
  `SMBus_Engine.dll`, and `NTIOLib_MysticLight`. These references identify the
  integration owner; they alone do not prove every runtime call edge.
- `Lib/Driver_Engine.dll` exports initialization/release, PCI configuration,
  I/O port byte/word, physical memory, CMOS and MSR operations. It imports
  DeviceIoControl and driver service management. Its initialization can have
  driver/service side effects; do not load/call it as a read-only diagnostic.
- Installed `Lib/NTIOLib_X64.sys` names `\Device\NTIOLib_MysticLight` and
  `\DosDevices\NTIOLib_MysticLight`. Its security descriptor string is
  `D:P(A;;GA;;;SY)(A;;GA;;;BA)` (SYSTEM and administrators).
- Preferred VA `0x1400010B8` registers a shared dispatch at `0x1400010F0` for
  create/close/device-control. The create branch `0x140001348` updates an open
  count and completes successfully. Unlike reviewed SignalIO create dispatch,
  this path contains no caller-image or publisher-signature check. This is a
  bounded observation, not a claim that all security restrictions were audited.
- The kernel dispatch exposes the matching low-level I/O/PCI paths and a
  device initialization control. No controls were invoked; no packet client,
  bus scan or hardware read/write was run.

Practical direction: a native x64 adapter to the already installed MSI driver
could avoid the x86 DLL stack and new signing expenditure. It would still depend
on that vendor driver being installed and permitted to load, and likely require
an elevated process or a narrowly scoped privileged helper. It is not yet a
portable independent solution or a recommendation to redistribute NTIOLib.
Before implementation, validate AMD SMBus discovery, locking/timeouts and narrow
ENE-only operations; assess the exact driver's support/security status. Never
expose the driver's general memory/PCI/MSR operations through the app.

No running MSI Center GUI is referenced by the reviewed kernel create path.
That supports investigating GUI-independent use, but independence from every
MSI service and possible bus contention remain untested. Existing MSI software
may be using the same bus. Do not terminate it automatically.

Additional identities:

- Driver_Engine.dll: `3a558050d1b82fe30be75aa23338ae2a3c86e72be65496facb18e1838a138b6b`
- MBAPI_x86.dll: `c4309d8fb1a6ef93bb2c8788f358cccbd21c49c65715508b60f520d6c37f27b8`
- NTIOLib_X64.sys: `cd0dc469de6a4ff744f8b4c051068a3c7873694cea77d9ac5693a22ea1e988a7`

Raw local analysis files are under build/: msi-smbus-disasm.txt,
msi-smbus-imports.txt, msi-driver-api.txt, msi-driver-disasm.txt,
msi-ntio-api.txt and msi-ntio-disasm.txt. No original DLL/SYS was modified.

## SignPath follow-up (2026-10-24)

SignPath Foundation offers free signing for accepted OSS applications; acceptance
is discretionary and requires maintained/released OSS and verifiable source builds.
This can be considered for Lighting Studio's own EXE/DLLs after qualifying the
release workflow. It is not confirmed as a free replacement for Microsoft's
kernel-driver signing process. The existing PawnIO path needs no new app-owned
kernel driver. No application/contact/signing integration has been performed.

Sources: https://signpath.org/terms and
https://learn.microsoft.com/en-us/windows-hardware/drivers/install/kernel-mode-code-signing-requirements--windows-vista-and-later-
