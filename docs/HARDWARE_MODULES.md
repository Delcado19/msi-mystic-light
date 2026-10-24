# Hardware modules

The app loads local Qt driver plugins and validates independent JSON device
profiles. Bundled plugins are `msi.gen1` and `lighting.peripherals` (Roccat
Vulcan/Kone, Razer Basilisk V3, SteelSeries Apex 3). No device is opened at startup.

See [SignalRGB repository research](SIGNALRGB_REPOSITORY_RESEARCH.md) for the
2026-10-23 review of all 16 public projects and the master/Development/beta
split. The next download increment should prioritize data-only components
and profiles. Native modules are one backend, not a requirement for every
new product. A script-driver host remains an option to evaluate separately;
the current loader does not execute SignalRGB JavaScript or QML.

## Responsibilities

- The host inventories hardware, resolves competing RGB applications, selects
  an exact device/interface and profile, and owns the transport. Descriptor discovery and profile candidates are available through `--scan`;
  the startup conflict dialog is implemented. The offline editor can explicitly
  scan/select a staged profile/endpoint pair without connecting. Missing board information does not match a board
  profile. Multiple matching candidates require explicit selection.
- A device profile describes identity selectors, channel capabilities, LED
  capacity, and protocol addresses. Capacity is not an attached LED count.
- A driver plugin validates its protocol-specific profile fields and turns
  native effect settings or RGB samples into hardware reports.
- Component definitions describe attached products and LED geometry, not a
  controller's USB protocol. Their data-only parser and channel mapping are implemented; an editor and
  persisted channel assignments are implemented. They do not require a native plugin.
- Effects and UI must use the generic `IDriver` and `RgbColor` types, not MSI
  protocol enums. Native effect IDs and supported channel modes come from the
  selected profile; the plugin validates them again before writing.

## Build and inspect

Use the README's build instructions. The executable, `modules/` directory,
`profiles/`, and `components/` directories are placed under `build/ninja/Release/` for the
validated Release build. The same layout is generated per configuration for
multi-configuration generators.

```bat
build\ninja\Release\msi_mystic_light.exe --help
build\ninja\Release\msi_mystic_light.exe --module-dir C:\trusted\modules --profile-dir C:\trusted\profiles
```

Normal startup first checks process conflicts, then validates local catalogs and
opens the offline editor. Use `--editor` to skip the conflict gate for offline
editing, `--inspect` to validate catalogs without a process dialog, or `--scan`
for read-only hardware discovery. Invalid module/profile/component files produce diagnostics and a nonzero exit
status. Loading modules, validating profiles and constructing drivers must
not open or write devices. Enumeration is explicit through `--scan`. Explicit driver operations can write;
production integration remains gated by the existing hardware safety rule.

## Adding support

For another board using an existing qualified protocol, add a profile based
on `profiles/msi-mpg-b850-edge-ti-wifi.json`. Supply the exact supported model
and identity selectors, verified channels and limits. Adding a profile is
not evidence that the hardware has been tested. Unverified protocol features
must remain unavailable or explicitly Unknown.

For a new protocol:

1. Implement `IDriverModule` and `IDriver` from `src/modules/IDriverModule.h`.
2. Export the module using `Q_PLUGIN_METADATA` and `Q_INTERFACES`, as in
   `modules/msi-gen1/MsiModule.cpp`.
3. Embed metadata containing a unique `id`, integer `apiVersion: 1` (feature reports) or `2` (output reports), and a
   nonempty release `version`. The interface IID is
   `org.msi-mystic-light.DriverModule/1.0`.
4. Validate profile addresses/capabilities without touching devices. Use the
   host-supplied `IHidDevice`; do not open an independent competing handle.
5. Add profiles, a CMake MODULE target, and transport-double tests. A locally
   built plugin can then be placed in the configured module directory.

No module registry switch statement needs editing for a new driver ID.
Each module receives one connection session. Its transport must outlive the
`DriverInstance`. Destroy/recreate the driver when the device reconnects;
cached firmware state cannot survive a disconnect. Driver instances keep
their plugin loader alive, including after registry destruction.

## Profile schema 1

Required fields: `schemaVersion: 1`, nonempty `id`, `driverId`, `name`, a
`match` object, and a nonempty `channels` array. Profile files are limited
to 1 MiB and must contain valid JSON objects.

`match` requires numeric `vendorId` and `productId` (1..65535). Optional
selectors are `usagePage`, `usage` (0..65535), `interfaceNumber` (0..255),
`boardManufacturer`, and `boardModel`. Unknown selector names are rejected
to prevent typos silently broadening matches. Strings match case-insensitively
after trimming outer whitespace; missing identity fields do not satisfy them.
The supplied MSI profile's board strings are exact profile selectors, not
evidence that system identification was run in this change. If firmware
reports another spelling, review the identity before adding a supported alias.
The supplied profile does not pin an unverified HID interface/usage value.

Each channel requires a unique `id`, `maxLeds` (1..65535), boolean `direct`,
`nativeModes` (unique string IDs), and an `address` object. Optional `fixedLeds`
must be 1..maxLeds. Its absence means the actual LED count is unspecified.
At least one mode must be available. The owning driver applies narrower
protocol limits and validates addresses before driver creation.

The MSI module accepts 18-zone Gen1 profiles with at most 120 LEDs per
channel. Direct mode is enabled only for the documented ARGB/EZ zone-area
pairs. Historical Lighting Studio work found that IO COVER did not respond
to per-LED `0x51`; this profile conservatively exposes its native modes only.
The older worktree's whole-zone software fallback is not ported here.
Direct-mode headers are armed to Advanced via transient `0x50` before `0x51`.
Full `0x50` reports initially set unconfigured zones to Off, then preserve
successfully applied state. No module path persists settings to board NVRAM.

## LED discovery

`discoverLeds()` distinguishes Unknown, Unsupported, Available and Failed.
Available results may report actual counts per channel; they are not the
capacity values from a profile. The MSI module currently returns Unknown
without I/O. ARGB Gen2 capability must be researched separately from the
historically named Gen1/Gen2 USB report paths. No speculative probes were
added to resolve this question.

## Compatibility and trust

API versions 1 and 2 use a Qt/C++ plugin boundary. API 2 adds output-report
transport support after the existing feature-report vtable slot. Feature-only
API 1 modules remain loadable; old hosts reject API 2 peripheral modules. Build modules with the matching
app SDK, compatible MSVC runtime/toolchain, architecture and Qt build.
The IID/API metadata gate is checked before instantiation; Qt performs its
own plugin compatibility checks on load. This is not a toolchain-independent
C ABI, nor a security sandbox. A malicious native module runs with host
permissions. Only put trusted libraries in the selected directory.

Repository installation is deferred: it needs a released module catalog,
matching app/API/platform requirements, authenticated package integrity,
staged installation and rollback. Do not load code directly from a branch,
claim a plain checksum authenticates a publisher, or auto-download plugins
until that path exists. Local loading is currently the only supported route.

## Validation

CTest builds and loads the real MSI plugin with a fake HID transport. Tests
cover profile parsing/matching, invalid addresses, incompatible API metadata,
missing interfaces, duplicate IDs, absent modules, driver lifetime, no-I/O
construction/discovery, transient reports, Advanced arming, retained native
zones, capacity guards, and failed-write retry/state behavior.
The application smoke test checks the generated module/profile layout.
These tests do not qualify real devices or automatic hardware detection.

## Component catalog schema 1

Startup reads `components/*.json` beside the executable, or the directory
specified by `--component-dir`. This boundary accepts JSON data only and does
not fetch repositories, execute scripts, or import SignalRGB files. The bundled
`generic.strip-8` is an original generic example, not a detected vendor product.

Each definition requires `schemaVersion: 1`, a stable nonempty `id`, `name`,
`ledCount` (1..65535), `mapping`, and `positions`. Both arrays must have exactly
`ledCount` entries. `mapping[logicalIndex]` is the zero-based wire index and
must be a permutation of 0..ledCount-1. Each position is a pair of finite 2D
coordinates in product-local units; the renderer will place products separately.
Files are limited to 1 MiB. Malformed files and duplicate IDs fail the entire
catalog import with a filename diagnostic; there is no implicit override order.
An empty directory is a valid empty catalog. Caller-supplied directories are
local data sources, not authenticated repositories.

`channelMapping` accepts product IDs in physical chain order, including repeated
instances of one product. It offsets each product's wire mapping, rejects unknown
IDs and chains exceeding `maxLeds`, and requires an exact total for channels with
`fixedLeds`. An empty chain is allowed only when no fixed count is specified.
It returns logical-to-channel-wire indices without sending frames. Definitions
and assignments express user configuration, not detected topology. Parallel
splitters that mirror a signal must not be represented as independent sequential
LEDs. The offline editor now persists explicit per-channel Native/Software
assignments, native parameters and physical instance order, with sampled
software-chain RGB output. See LIGHTING_EDITOR.md. Transport output, remote
installation, source provenance and catalog override layers remain separate work.

Component tests cover permutations, geometry, malformed/missing fields, file
limits, duplicate catalog IDs, repeated products, capacity and fixed-count
checks. App tests verify the bundled catalog and missing-directory diagnostics.

## Hardware discovery

Run the executable with `--scan` to read OS board identity, enumerate HID
endpoints and list all matching profile/endpoint pairs. Without `--scan`,
startup validates local catalogs and opens the offline editor. Startup and scan do not open devices or send
probe/lighting reports. Descriptor usage/interface values are retained;
missing interface numbers remain -1. Empty paths are excluded.

Windows board identity comes from the read-only BIOS registry values
`BaseBoardManufacturer` and `BaseBoardProduct`. Linux reads `board_vendor`
and `board_name` under `/sys/class/dmi/id`; other platforms return unknown.
Missing board fields cannot satisfy required selectors. Firmware strings
are matched using the profile's existing trim/case-insensitive rules.

Matching preserves every candidate, including multiple interfaces for one
profile and multiple profiles for one endpoint. There is no automatic
first-match selection or inferred interface qualification. Profiles must
provide verified selectors where available. The editor offers explicit staged
candidate selection; explicit connection after conflict resolution and report consent is implemented. `HidApiDevice::openPath` is
implemented and tested with doubles but is not called by startup or scan.
The legacy VID/PID open remains for existing callers; production integration
must use the selected exact path. Re-enumerate after unplug/replug.

On Windows CMake copies the matching HIDAPI DLL beside the app; set
`HIDAPI_RUNTIME_DLL` if it cannot be found beside the import library or in
its sibling bin directory. The Windows build also copies Qt runtime DLLs and the Windows platform plugin
beside the app. A distributable installer remains pending.

## Startup process conflicts

Normal startup presents known potential lighting writers by exact executable
name, PID, accessible path and match reason. It does not infer actual device
ownership from a process name. Selection supports multiple rows. `Terminate`
affects only selected rows; `Terminate all` requests the listed rows. Both
first request normal window closure and, after 1.2 seconds, force the same
remaining verified processes. The explanation warns about unsaved work before
these buttons are used. `Ignore` explicitly continues; `Cancel`, Escape and
window close exit startup. No termination happens merely by opening the dialog.

Windows snapshots use Toolhelp. Each actionable process must have an accessible
image path and creation time; the backend checks both again using the handle
that will be terminated to prevent PID reuse from changing the target. Self,
unknown executable names, missing metadata and changed identities are rejected.
Known service-owned helpers LEDKeeper2/LightingService stay listed but require
manual application/service shutdown. There is no elevation, service disable,
autostart change, or attempt to kill a generic vendor host.

The wait is asynchronous so Cancel remains available; cancellation prevents the
pending force step. The list is rescanned after attempts and has a Rescan button.
Denied access and scan failures are shown. Remaining/restarted processes require
another explicit action, Ignore, or Cancel. A failed scan does not silently pass.
The Windows backend is implemented; other operating systems display an explicit
unsupported-scan error with Ignore/Cancel rather than claiming no conflicts.
`--inspect` and `--scan` intentionally bypass the gate because neither opens or
writes devices; future connection code must not use these paths to skip it.

Qt UI tests use fake processes for all close/terminate operations. They cover
selection, all, denied access, restarts, cancellation, no-conflict continuation,
scan errors, exact names and identity guards. The real Windows scan is tested
read-only; actual process shutdown is not exercised by the suite.

## Output-session ownership

The host-side `OutputSession` backend owns an explicitly supplied open transport
and its `DriverInstance`, coalesces staged frames and caches successful native
settings. Failure closes the session without retry; explicit reconnect uses a
fresh driver. OutputScheduler now keeps session creation, writes and teardown on its worker,
coalesces frames and checks stop requests between channel calls. The live app
connects this backend only after exact-path discovery and explicit report consent.
See [OUTPUT_SESSION.md](OUTPUT_SESSION.md) for lifecycle, validation and remaining
authorization and remaining physical qualification work.

Live output uses the validated automatic exact-path connection flow described in [OUTPUT_SESSION.md](OUTPUT_SESSION.md). Physical protocol qualification remains pending.
