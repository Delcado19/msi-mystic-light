# OpenRGB SDK client — cross-platform (esp. Linux) hardware access

Date: 2026-10-29. Source study only (OpenRGB is GPL-2.0-or-later and fully
public; no reverse engineering needed, no binary opened, no device touched).
Local reference checkout: `build/research/openrgb` (gitignored), master
`aac7e551` — the commit previously pinned in `RAM_ACCESS_RESEARCH.md`
(`547ca943...`) no longer exists in upstream history, so master's current
tip was used instead. Decision context: the user runs 3 machines (Windows
desktop with this board, a Linux desktop, a Linux laptop) and wants broad
hardware support, not just the MSI B850. User-approved direction: on
Linux, this app becomes an **OpenRGB SDK client** rather than reimplementing
every vendor's native bus driver a second time. Windows keeps the existing
native backends (HID, NVAPI, PawnIO) — see `HARDWARE_MODULES.md`.

## Why a client, not a native Linux driver stack

OpenRGB already ships **198 device controller implementations** under
`Controllers/` (`ls Controllers | wc -l`), including nine distinct MSI
controllers alone (`MSIMotherboardController`, `MSIGPUController`,
`MSIKeyboardController`, `MSILaptopController`, `MSIMonitorController`,
`MSIRGBController`, `MSIVigorController`, `MSIClutchController`,
`MSI3ZoneController`). Reimplementing this breadth natively per platform
is not a reasonable increment. OpenRGB's own cross-platform split already
exists at `i2c_smbus/{Linux,Windows,MacOS}/` (Linux: `i2c-dev`; Windows:
PawnIO/WinRing0-class transports, matching what `RAM_ACCESS_RESEARCH.md`
already found independently for our own PawnIO path). Running OpenRGB as
a background server on the Linux boxes and only consuming its device/zone/
LED list over the SDK protocol gets this app broad hardware coverage on
Linux for free, at the cost of a runtime dependency on an installed,
running OpenRGB process there.

## Wire protocol (`NetworkProtocol.h`)

- TCP, default port **6742** ("ORGB" on a phone keypad), default bind
  `127.0.0.1`. Max packet size 8 MiB.
- Every packet: `NetPacketHeader { char magic[4]="ORGB"; uint dev_id;
  uint pkt_id; uint pkt_size; }`, followed by `pkt_size` bytes of payload.
- Current protocol version **6** (version negotiated per connection, see
  below); versions are additive — v1 added vendor string, v2 profiles,
  v3 mode brightness, v4 zone segments + network plugins, v5 zone/
  controller flags + rescan, v6 server name + client/server feature flags.
- Status codes on ack: `OK=0, ERROR_GENERIC=1, ERROR_UNSUPPORTED=2,
  ERROR_NOT_ALLOWED=3, ERROR_INVALID_ID=4, ERROR_INVALID_DATA=5`.

### Handshake (`NetworkClient::ConnectionThreadFunction`, `NetworkClient.cpp:1707`)

1. TCP connect.
2. `NET_PACKET_ID_REQUEST_PROTOCOL_VERSION` (40) → wait up to 1s; no
   reply means assume protocol version 0 (an old/minimal server).
   Negotiated version = `min(client, server)`.
3. `NET_PACKET_ID_SET_CLIENT_NAME` (50) + `SET_CLIENT_HOSTNAME` (54).
4. If negotiated version ≥ 6: `NET_PACKET_ID_SET_CLIENT_FLAGS` (52) —
   advertise `NET_CLIENT_FLAG_SUPPORTS_RGBCONTROLLER` etc.
5. `NET_PACKET_ID_REQUEST_CONTROLLER_COUNT` (0), then
   `NET_PACKET_ID_REQUEST_CONTROLLER_DATA` (1) per device index.

### Relevant packet IDs for a read/write lighting client

| ID | Name | Use |
|---|---|---|
| 0 / 1 | REQUEST_CONTROLLER_COUNT / DATA | enumerate devices + full `RGBController` description |
| 100 | DEVICE_LIST_UPDATED | server hotplug/rescan notification |
| 120–124 | GET_I2C_BUS_INFO / HID_DEVICE_INFO / USB_DEVICE_INFO / SERIAL_PORTS / USB_SERIAL_PORTS | raw bus inventory, if we ever want it independent of a detected controller |
| 140 | REQUEST_RESCAN_DEVICES | trigger re-detection |
| 1050–1052 | RGBCONTROLLER_UPDATELEDS / UPDATEZONELEDS / UPDATESINGLELED | push color frames — this is the actual output path |
| 1100–1103 | SETCUSTOMMODE / UPDATEMODE / SAVEMODE / UPDATEZONEMODE | select/persist a native hardware mode instead of streaming |

Client never needs the ProfileManager/PluginManager/SettingsManager/
LogManager ranges (150+/200+/250+/300+) for a lighting consumer role.

## Data model (`RGBControllerInterface.h`) — what the server actually exposes

A detected device serializes to one `RGBController` description:

- **Identity**: `name`, `vendor`, `description`, `version`, `serial`,
  `location`, device-specific `configuration` (JSON).
- **`device_type`** enum: `MOTHERBOARD, DRAM, GPU, COOLER, LEDSTRIP,
  KEYBOARD, MOUSE, MOUSEMAT, HEADSET, HEADSET_STAND, GAMEPAD, LIGHT,
  SPEAKER, VIRTUAL, STORAGE, CASE, MICROPHONE, ACCESSORY, KEYPAD, LAPTOP,
  MONITOR, UNKNOWN` — directly comparable to our own `ComponentDefinition`
  categories; this is the enum to map onto our existing `HardwarePicker`
  product categories.
- **Zones** (`zone`): `name`, `type` (`SINGLE/LINEAR/MATRIX/LINEAR_LOOP/
  MATRIX_LOOP_X/Y/SEGMENTED`), `leds_count`/`min`/`max`, optional
  `matrix_map` (2D layout, for keyboards/matrices), `segments` (sub-ranges
  within one zone — e.g. per-key-group), and **per-zone modes** (since
  protocol v5/v6; a zone can run a different native mode than the device).
- **Modes** (`mode`): `name`, device-specific `value`, `flags` bitfield
  (`HAS_SPEED`, `HAS_DIRECTION_LR/UD/HV/DIAG`, `HAS_BRIGHTNESS`,
  `HAS_PER_LED_COLOR`, `HAS_MODE_SPECIFIC_COLOR`, `HAS_RANDOM_COLOR`,
  `MANUAL_SAVE`, `AUTOMATIC_SAVE`, `REQUIRES_ENTIRE_DEVICE`), plus
  min/max for speed/brightness/colors — **this is the direct equivalent of
  our profile `nativeModes` + parameter bounds**, already a superset of
  what our own schema captures today.
- **LEDs** (`led`): `name` + device-specific `value` only — no per-LED
  geometry/coordinates in the base protocol. **Physical LED position is
  not part of what OpenRGB's server provides**; that stays our own
  `ComponentDefinition.positions` concern regardless of backend.
- **Flags** (`controller_flags`): `LOCAL/REMOTE/VIRTUAL/HIDDEN`,
  `RESET_BEFORE_UPDATE`, manual-configurability/configured bits for
  name and device-specific config.

This confirms the finding already in `HARDWARE_MODULES.md`: capacity/mode
metadata is what the server (and vendor table) knows; **attached-LED
count is still "Available" (if the specific controller probes it) or
unspecified** — the client inherits exactly the same discoverable-vs-
table-driven distinction our own MSI module already has for `discoverLeds()`.

## Mapping onto this app's architecture

- Add a new match `transport: "openrgb-sdk"`, analogous to the existing
  `nvapi://` and `pawnio` transport selectors in `HARDWARE_MODULES.md`.
  An endpoint identity would be `openrgb-sdk://<host>:<port>/<controller-index>`
  (host likely `127.0.0.1` for same-machine, or a LAN host for the second
  Linux box if it ever needs to be controlled from elsewhere — scope that
  separately, do not assume network exposure is wanted).
- One new driver module (`lighting.openrgb-client` or similar) that:
  speaks the handshake, lists controllers, maps `device_type`/`zone`/
  `mode` onto our `IDriver`/`RgbColor`/profile types, and writes frames
  via `UPDATELEDS`/`UPDATEZONELEDS`.
- Keep this strictly additive: existing Windows HID/NVAPI/PawnIO modules
  are untouched; the OpenRGB client is one more `IDriverModule`, selected
  only when its transport matches and a local/remote OpenRGB server
  answers the handshake.
- **Open question for implementation, not answered by this research**:
  does the app *launch* OpenRGB (process spawn — a hardware-safety-rule
  action, requires explicit confirmation per existing policy) or only
  *connect* to an already-running instance the user starts themselves?
  Recommend the latter as the default, matching the existing "no
  automatic process termination/launch" posture in `HARDWARE_MODULES.md`'s
  conflict handling.

## License note (not a legal determination)

OpenRGB is GPL-2.0-or-later. We are **not copying OpenRGB source** —
only implementing our own client against a documented wire protocol
(magic bytes, packet IDs, struct layout), the same way `signalrgb-python`,
the Home Assistant integration and the SignalRGB-to-OpenRGB bridge already
do from outside OpenRGB's codebase (see `SIGNALRGB_TOPIC_RESEARCH.md`).
Verify this reasoning against the project's own `LICENSE` before shipping
a client module; this file states facts about the protocol, not a
license compatibility ruling.

## Deeper pass (2026-10-29, same day): SignalRgb.exe native binary + OpenRGB current-master re-check

Attempted full Ghidra decompilation of the installed `SignalRgb.exe`
(SHA-256 `cd5d6ae5...`, confirmed unchanged from `RAM_ACCESS_RESEARCH.md`).
REA's `open_binary`/`binary_overview` auto-selected Ghidra 12.1.3 (now
registered, unlike the 2026-10-24 pass) but the headless analysis of this
binary **timed out twice** at REA's fixed 330s per-call budget
(`provider_timeout` on the `list_documents` step); the second attempt also
left a `cleanup_incomplete` (Ghidra's own process held `ghidra.log` briefly
after being killed, resolved on its own moments later — no orphaned process
remained). The target is **118,443,584 bytes**, far larger than the 477-
function PawnIO.sys driver Ghidra handled successfully on 2026-10-27; a full
auto-analysis of a binary this size needs longer than REA's per-call ceiling
allows, and REA does not resume across calls (`import_mode:
ephemeral-source-immutable`) — each retry restarts analysis from zero.
Most of this binary is Qt/QtWebEngine (Chromium) UI shell, not hardware
logic — confirmed by `dumpbin /imports` showing heavy `QWebEngineView`/
`QWidget`/`QPainter`/`QWindow` symbol traffic. The user approved the
disproportionate cost anyway; see "Full decompile, targeted" below for
the self-run `analyzeHeadless.bat` pass (outside REA's timeout, ~24
minutes) and its findings.

Instead, cheap static evidence (dumpbin + raw string scan, no Ghidra, no
execution) answered the actually-relevant question:

- `dumpbin /imports` on `SignalRgb.exe`: **no static import** of `hid.dll`,
  `setupapi.dll`, or `winusb.dll`. Only generic `CreateFileW/A`, `ReadFile`,
  `WriteFile`, `DeviceIoControl` are statically imported (consistent with
  the already-documented ad-hoc `\\.\SignalIo` open+IOCTL path).
- A raw byte-pattern scan of the same binary **does** find `HidD_GetAttributes`,
  `HidD_SetFeature`, `HidD_GetFeature`, `HidD_GetPreparsedData`,
  `SetupDiGetClassDevsA/W`, `SetupDiEnumDeviceInterfaces`,
  `SetupDiGetDeviceInterfaceDetailA`, `WinUsb_Initialize`,
  `WinUsb_ControlTransfer`, `WinUsb_ReadPipe`/`WritePipe`, etc. as string
  data, with none of those three DLLs in the static import table. This
  means the host resolves them **dynamically** (`LoadLibrary`+
  `GetProcAddress`, not a compile-time import) — the generic
  HID-enumerate/HID-feature-report/WinUSB-bulk-transfer layer that backs
  the JS `bus.*` API is present but intentionally not visible in the plain
  import table. No `ENE_RAM`/`AUDA0` strings are in this binary (that
  logic lives only in the already-reviewed `ENE_RAM.js` plugin, confirming
  the plugin/host split: generic HID/WinUSB I/O is native-host code, device-
  specific register maps are JS). No device was opened, no IOCTL sent.

### OpenRGB current-master re-check (`build/research/openrgb`, `aac7e551`)

Re-verified the upstream bug reported in `OPENRGB_UPSTREAM_DRAFT.md`
(GitLab issues #4935/#5429) against the freshly cloned current master —
**still present, unfixed**:

- `Controllers/MSIMotherboardController/MSIMotherboardControllerDetect.cpp:85,99`:
  `unsigned char second_buffer[761];` ... `hid_send_feature_report(dev,
  second_buffer, 761)` — still the literal `761`.
- `Controllers/MSIMotherboardController/MSIMotherboardCommon.h:247-257`:
  `struct FeaturePacket_PerLED_761` is `report_id + fixed1 + hdr0 + hdr1 +
  fixed2 + fixed3 + hdr2` (7 bytes) `+ colors[NUM_LEDS_761]`, and
  `#define NUM_LEDS_761 720` (line 123) → `sizeof() == 727`, matching our
  own documented report `0x51` (727 bytes) exactly. The detection probe's
  hardcoded `761` still does not match the struct it is supposed to be
  probing with.

## Full decompile, targeted (2026-10-29, same session)

The 118MB `SignalRgb.exe` full Ghidra auto-analysis (run directly via
`analyzeHeadless.bat`, outside REA's 330s per-call budget) completed after
~24 minutes (exit code 0). A second, fast pass reopened the same project
with `-noanalysis` and ran a custom post-script
(`build/research/ghidra-scripts/SignalRgbEvidence.java`) that scans all
439,560 defined-data strings for HID/SetupAPI/WinUSB/SignalIo markers,
resolves the 9 functions that reference the 51 matching strings, and
decompiles only those — avoiding a full-binary pseudocode dump. Raw output:
`build/research/ghidra-signalrgb/signalrgb-evidence.txt` (not committed,
gitignored `build/`). No device was opened, no IOCTL sent, no process
other than the offline analyzer ever ran.

**Confirmed: SignalRGB's native hardware layer is built on the same
open-source USB/HID libraries OpenRGB uses, not a from-scratch stack:**

- `hid_send_feature_report` and `hid_get_report_descriptor` are present as
  named functions with decompiled bodies that match the public **hidapi**
  library's Windows backend line-for-line (buffer-padding to the cached
  report length, `HidD_SetFeature` call through a dynamically-resolved
  function pointer, `FormatMessageW`-based error string, and
  `hid_winapi_descriptor_reconstruct_pp_data` for the preparsed-data path).
  This is the identical open-source `hidapi` C library OpenRGB also bundles
  (`hidapi_wrapper/` in the OpenRGB tree, and this project's own
  `third_party`/HID backend) — not a reimplementation.
- A second function contains the literal string `"winusbx_init"` with the
  log message `"WinUSB DLL is not available"` — `winusbx_init` is an
  internal symbol name from **libusb**'s Windows backend
  (`windows_winusb.c`). SignalRGB also statically links libusb for
  WinUSB-class devices, the same library OpenRGB uses for several of its
  controllers.
- The HID/WinUSB DLLs are loaded through `LoadLibraryW(L"hid.dll")` /
  `LoadLibraryW(L"cfgmgr32.dll")` / a `"WinUSB"`-named loader, each followed
  by `GetProcAddress` per function with a null-checked fallback chain
  (plain name, then `...A`, then `...W` suffix attempts — Windows `HidD_*`
  functions have no real A/W variants, so this is defensive/generic FFI
  glue, not evidence of a second hand-rolled HID implementation).
- A separate, generic `SetupDiGetClassDevsW`/`SetupDiEnumDeviceInfo`-based
  device-class enumeration helper (`FUN_1400035b0`) is reused for arbitrary
  device-interface GUIDs, including the private `\Device\SignalIo` /
  `\DosDevices\SignalIo` interface already mapped in `RAM_ACCESS_RESEARCH.md`
  — one generic enumerator, not a SignalIo-specific one.

**Practical implication for this project**: there is no secret native
SignalRGB device-access protocol to port — it is hidapi + libusb underneath,
exactly the same building blocks already available to us directly (our own
HID backend, and OpenRGB's SDK client approach decided earlier in this
research). The only SignalRGB-specific part worth anything is the *device
database* (JS plugins, already reviewed) and the *kernel driver*
(`SignalIo.sys`, already mapped — caller-gated to `SignalRgb.exe`/
`SignalRgbService.exe`, not independently usable).

## Still open

- No Linux hardware inventory yet (`scripts/hw_inventory_linux.sh` written,
  not yet run on either Linux machine — see chat for the pending request).
- No OpenRGB instance was installed, started, or connected to during this
  research; the handshake/packet sequence above is read from source only.
- GPU (`i2c_amd_gpu.h`/`i2c_intel_gpu.h`) and DRAM controller breadth in
  `Controllers/` was not enumerated in detail — only counted and spot-checked
  for the MSI family already relevant to this project. Revisit once the
  Linux inventory identifies which actual GPU/RAM/peripheral vendors are
  present on those two machines, so only the relevant OpenRGB controllers
  need to be cross-checked against real VID/PID/PCI-subsystem IDs.
