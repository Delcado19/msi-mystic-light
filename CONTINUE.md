# Implementation status

## GUI output, channel scopes and session completion (2026-10-23)

- Explicit conflict/discovery/report-consent output gate; exact-path worker-owned
  transports, revision invalidation, presence polling and explicit reconnect.
- Schema 9 layer channel targets and per-channel rate/offset; targeted preview.
- Atomic autosave/recovery, last project/window restore, single instance and tray.
- Windows HIDAPI imported target now models the runtime DLL transitively; deployment
  regression checks compare app/test DLL hashes with the SDK. Start-Lighting.cmd
  selects the complete Release folder instead of the obsolete root executable.
- Release and Debug: 14/14 CTest passed in each configuration. Native editor 27/27 and output UI 6/6 passed
  using fake transports. No physical HID writes or process termination performed.
- Freshly zipped/extracted Release --inspect exited 0 with only Windows on PATH.
- Physical calibration/device behavior and clean-machine VC runtime remain unqualified.


## Current remaining qualification

- Verify output/calibration on each real controller and peripheral using the
  explicit report-consent gate. Fake transport tests do not establish physical
  color, LED mapping, unplug timing or firmware behavior.
- Check the complete portable folder on a clean Windows installation with the
  supported Visual C++ runtime; the current package was tested on this workstation.
- Repository downloads/script-driver compatibility and automatic attached-LED
  discovery remain separate work, not features of the current application.

## Historical implementation log

The entries below describe earlier increments. Their pending/unimplemented and
uncommitted statements are historical; the current status above supersedes them.

## Worker output scheduler (2026-10-23)

- Added OutputScheduler: QObject control/status with one standard-library worker,
  a single latest-frame mailbox and 33 ms waits after completed iterations. No
  per-submission GUI event queue, idle frame replay or catch-up burst.
- Factories create sessions/modules/transports on the worker. Start returns a
  generation; stale submissions are rejected after explicit restart. Start/stop
  are owner-thread operations; submit/status are thread-safe.
- Stop is nonblocking, discards pending work and checks cancellation between
  channel driver calls. In-flight factory/module calls may complete. Destruction
  joins and can wait for an OS call. Teardown finishes on the worker before
  terminal status is published. No automatic retry after faults.
- Tests use gated fake factories/transports and the actual MSI plugin to cover
  latest frames, blocked I/O responsiveness, pacing, cancellation, restarts,
  invalid factories/frames, write faults and worker-thread resource destruction.
- Validation: Release and Debug passed 12/12 CTest entries. Scheduler checks
  include owner-thread delivery of status notifications and blocked-I/O pacing.
- Still pending: GUI conflict/confirmation gate and exact-path connection,
  enumeration-based unplug handling, physical calibration/qualification,
  per-layer targets/independent timelines and session/tray behavior.

## Output-session backend (2026-10-23)

- Added prepareOutput(document, composedFrame), grouping sampled software chains
  and native commands by exact endpoint/profile without driver calls.
- Added an explicitly connected, single-thread-owned OutputSession. The host
  supplies an already-open transport after its conflict/confirmation gate.
  Connecting creates a driver without reports. stage replaces the single pending
  frame; flush is explicit and synchronous. No worker/timer or GUI connection yet.
- Validate every frame before writes, including target/profile, channel uniqueness,
  capabilities/counts and native options. Invalid replacement clears stale pending
  work. Cache native settings only after success; software transitions invalidate
  that channel cache. On false/exception/closed transport, discard remaining work,
  destroy driver before transport, close and enter Faulted. No automatic retry.
- Explicit reconnect uses a fresh transport/driver and clears all applied caches.
  Disconnect/destruction cancels pending work without off/restore commands.
- Tests use the actual MSI module with recording fake transports only, comparing
  serialized 0x51 bytes and injecting failures at arming, color and native writes.
- Validation: Release and Debug each passed 11/11 CTest entries. Recording-transport
  tests also cover the composed masked/translucent document through sampling,
  preparation and actual MSI serialization, plus fixed counts and fault retention.
  No physical device I/O was performed.
- Still pending: GUI conflict/confirmation gate and exact-path connection, worker
  scheduling, enumeration-based unplug detection, calibration/physical output
  qualification, per-layer channel targets and session/tray behavior. Native-only
  idle sessions cannot detect a physical unplug from isOpen alone.

## Staged channel assignment and native settings (2026-10-23)

- Added profile/endpoint/channel selection, Native/Software dialog, physical
  component-chain append/remove/reorder and persisted native parameters.
- One instance belongs to one channel; capacity, fixed counts, direct support,
  profile conflicts, duplicate assignments and supported native modes are checked
  before state/history changes. Assigned instance deletion/capacity-breaking
  geometry edits fail safely. Saved assignments remain editable without a scan.
- Schema 8 stores assignments and native settings; schemas 1..7 migrate with no
  assignments. Unsupported profiles or malformed assignments preserve live state.
- Added sampleChannels(frame): one shared composed frame, local wire order once,
  physical chain concatenation, opaque RGB over black. Native channels are absent.
- Validation: Release and Debug each passed 10/10 CTest entries; native Windows
  editor passed 25/25 Qt Test cases. Inspected the native channel dialog screenshot.
  Checks include chain order/permutations, alpha, native exclusion, invalid
  capacity/fixed counts, conflict rejection, atomic load, schema 7 migration,
  undo/redo and configuring an existing assignment through fake discovery.
- Staging only: no devices opened/written, no firmware mode switched by preview
  or edits. Remaining: per-layer channel targets/independent timelines, hardware
  connection, conflict/confirmation gate, reconnect, scheduling/calibration,
  output qualification and session/tray behavior.

## Component authoring and portable geometry (2026-10-23)

- Added Create/Edit geometry table and JSON import in Layout. Existing IDs are
  immutable; accepted edits affect all instances and are undoable. Invalid input
  remains in the dialog. Import shares the bounded schema-1 catalog parser.
- Schema 7 embeds document-owned definitions, including unused custom products.
  Projects reopen independently of installed catalogs. Schemas 1..6 still read
  their geometry from the original startup catalog. Duplicate embedded IDs and
  newer fields under older schemas are rejected atomically.
- Definitions participate in undo/dirty state, and the catalog picker follows
  create/import/edit/load/undo. Limits: 256 definitions, 65535 LEDs/definition,
  existing 32 MiB project limit. Editing does not change shipped catalog files.
- Validation: Release and Debug each passed 10/10 CTest entries; native Windows editor passed
  23/23 Qt Test cases, including active-cell commit and portable catalogs.
  Inspected native component dialog and Layout screenshots. No hardware opened.
- Remaining application work: channel assignment/native controls, reconnect and
  output scheduling, real hardware output/qualification, session restore/tray.

## Layout transforms (2026-10-23)

- Added independent X/Y scale and rotation around component origins, numeric
  Layout controls, undo/redo, and shared preview/hit-test/sampling coordinates.
- Schema 6 persists transforms; schemas 1..5 retain original geometry.
- Release and Debug each passed 10/10 CTest entries, including geometry,
  sampling, migration/rejection, persistence and Layout controls/undo checks.
- Native Windows editor checks passed 21/21 Qt Test cases with SDK DLLs absent
  from PATH. No physical devices were opened or written.
- Sampling now validates in-memory component permutations/lengths before indexing.
- Still pending: component authoring/import UI, channel assignment/native controls,
  reconnect/output scheduling and real hardware qualification.

## Component sampling backend (2026-10-23)

- Added per-placement RGB sampling of a shared composed canvas in local wire order.
- Layout and sampling share the same LED coordinate conversion. Transparency
  composites over black; out-of-canvas positions are black; repeated instances
  retain separate UUID keys. No channel assignment or hardware output is added.
- Release and Debug each passed 10/10 CTest entries. Sampling tests cover reversed
  wiring, alpha, fractional coordinates, repeated instances, canvas boundaries,
  huge origins, invalid frame size and undo.

## Completed: accepted offline effect authoring scope (2026-10-23)

- Built on published 0ab3ff6. Implemented drift X/Y, sinusoidal zoom, sine warp,
  exponential breathe, user color cycles, image hue grading and deterministic
  motion trails. Added four seeded particle sources: Rain, Orbit, Fireflies,
  Burst; count up to 400, editable seed/speed/radius and both colors.
- Added on-canvas move/centered resize/rotate grips and image crop gestures,
  precise normalized crop controls and full-source reset. Original pixels are
  retained. Escape/tab/history changes cancel unfinished gestures; a completed
  gesture is one undo step. Contextual properties keep the layer list readable.
- Added optional linked masks, following manual geometry and animated affine/
  warp transforms. Mask editing pauses/resets time and shows base mask geometry
  without the selected layer's animated warp/link projection. Masks default to
  unlinked canvas coordinates. Geometry edits resample linked masks; undo restores
  the exact prior raster.
- WebP now works alongside PNG/JPEG/GIF/BMP. Built the unmodified Qt ImageFormats
  6.8.3 plugin and its bundled libwebp at pinned commit
  3b6b1fb96041f1bc85a8f42593bef2ee7bcdca6c. Release/Debug plugin deployment,
  reproducible build helper, third-party notices/license texts and replacement
  instructions are included. Source/plugin build directories remain ignored.
  GIF and animated WebP intentionally import their first frame only.
- Schema 5 persists all new settings, retaining schemas 1..4 migration. A shared
  advanced parameter table drives UI ranges, parsing, validation and storage.
  Invalid typed crop/color values cannot escape Qt slots or change the document;
  malformed projects preserve the live state and history.
- Validation: Release Ninja and Debug Ninja Multi-Config each passed 10/10 CTest
  entries. Native Windows tests passed 19/19 Qt Test cases (including setup/
  cleanup), with Qt SDK removed from PATH. Deployed --inspect exited 0 without
  SDK PATH/plugin overrides. Inspected native particle/transform screenshots.
  Native composed frame mean across two runs: 19.75..22.375 ms for one 400-particle layer with seven
  trail taps, below the 33.3 ms target; this is not a 64-layer performance claim.
  Build helper was re-run successfully against the pinned source checkout.
- The accepted image/shape/gradient/motion/color/particle editor increment is
  complete. Separate application work remains: component geometry import/edit
  UI, layout size/rotation, channel assignment/native controls, reconnect logic,
  renderer-to-wire sampling and real hardware output. No devices were opened or
  written. Keyframes, multistop gradients and animated-image playback are not
  claimed as delivered features. See docs/LIGHTING_EDITOR.md for exact semantics.


## Completed: animation preview increment (2026-10-23)

- Published embedded image effects as d216b2c and verified origin/master.
- Added per-layer spin, pulse and procedural hue rates with undoable controls.
  Absolute-time evaluation uses the existing source/mask/transparency/blend path;
  canvas-space masks remain fixed. Image hue grading remains pending.
- Added an explicitly started, initially paused preview with time entry and
  elapsed-time playback. It does not dirty the document or add undo entries.
  Mask editing pauses playback. Schema 4 stores rates; schemas 1/2/3 still load.
- Validation: Release and Debug passed 10/10 CTest entries each; native Windows
  editor passed 16/16 Qt Test cases. Tests cover timed rotation/pulse/hue,
  legacy schemas 1/2/3, rate round trips, invalid edits, playback without undo
  changes, and automatic pause for mask editing. No hardware was opened.
- Pending: drift, zoom, warp, user color cycles, keyframes, particles/trails,
  WebP, free crop/transform handles, mask linking and hardware output.


## Completed: embedded image effects (2026-10-23)

- Added bounded PNG/JPEG/BMP/GIF import into an Image layer with a white mask.
  JPEG orientation is applied; GIF imports the first frame. WebP remains pending:
  the installed Qt SDK has no codec. JPEG/GIF runtime plugins ship with the app
  and editor test executable in both build configurations.
- Fit and centered fill/crop preserve aspect ratio; existing numeric geometry,
  masks, transparency, blending, duplication and undo apply to images. Original
  decoded pixels remain available when cropping or changing effect types.
- Schema 3 embeds lossless PNG pixels and fit/fill settings; schemas 1/2 still
  load. Import limits: 16 MiB input, 4096 pixels per side, 16 megapixels total
  per document. Project saves reject outputs above the 32 MiB load limit before
  replacing an existing file. Failed import/load preserves live state/history.
- Release and Debug each passed 10/10 CTest entries, including image format
  imports, fit/crop alpha, embedded round trip after deleting the source, undo,
  corrupt image rejection and UI fit controls. See docs/LIGHTING_EDITOR.md.
- Native Windows editor checks passed 15/15 Qt Test cases; inspected the image
  editor screenshot. Graphify updated with the known partial-parser warnings.
- Remaining: WebP codec, free crop/transform handles, mask linking, animation,
  particles, channel targets and hardware output. No devices opened or written.


## Completed: gradient and shape effects (2026-10-23)

- Published the previous offline editor/module/discovery/startup increment as
  `4df635c` on origin/master and verified the remote hash.
- Added five gradient types (linear, radial, conical, stripes, waves) and nine
  shape types (circle, ring, star, heart, triangle, hexagon, diamond, cross,
  moon). Static Color remains the default. All use the existing layer masks,
  transparency and blend modes; shape edges are antialiased.
- Effect creation/type selection, two gradient colors including alpha, numeric
  center/size/rotation and stripe/wave repetitions are editable in Effects.
  One shared type table supplies stable IDs and labels for UI and persistence.
- Project schema 2 stores effect parameters and rejects unsupported types.
  Schema 1 still loads as Static Color with existing layer/mask state preserved.
  Failed migrations/parameter validation leave the live document untouched.
- Masks currently remain in canvas coordinates when effect geometry changes.
  Direct transform handles, mask linking, multistop gradients, images, motion,
  particles, channel targets and hardware output remain pending. See
  `docs/LIGHTING_EDITOR.md` for exact controls and format boundaries.
- Validation: Release Ninja and Debug Ninja Multi-Config each passed 10/10
  CTest entries. Native Windows editor tests passed 14/14 Qt Test cases
  (including setup/cleanup); inspected the gradient/shape editor screenshot.
  Tests cover every effect type, alpha interpolation, mask composition, numeric
  controls, schema-1 migration, schema-2 round trips and atomic rejection of
  unknown effects/invalid geometry. No devices were opened or written.

## Completed: first offline editor increment (2026-10-23)

- Normal startup now opens a Qt Lighting window after the existing conflict
  gate/catalog checks; `--editor` opens it without the conflict gate because no
  editor operation opens/writes devices. `--inspect`/`--scan` still exit.
- Separate Layout and Effects tabs retain independent viewport/selection state.
  Layout adds/moves/removes catalog instances and displays wire indices. Effects
  has Static Color layers, rename/visibility/reorder/duplicate/delete, dedicated
  transparency and Normal/Multiply/Screen/Overlay composition over real RGBA.
- Each layer has an independent grayscale mask, thumbnails/editing targets,
  soft brushes, gradients, fill/invert/bypass, and undo/redo. A stroke is one
  undo step; Escape/tab switching cancels an unfinished stroke.
- Schema-1 project save/load uses atomic QSaveFile writes and validates bounded
  PNG masks/IDs/properties/catalog references before replacing live state.
  Failed loads retain the document; unsaved changes prompt on close/open.
- Explicit read-only scan exposes all profile/exact-endpoint candidates without
  auto-selection. Selection is staged only: no connection or channel assignment.
- See `docs/LIGHTING_EDITOR.md` for controls, file format and limits. Effects
  currently cover the whole offline canvas; per-channel targeting/native-mode
  UI, transforms, image/motion/particle effects, hardware output and automatic
  last-project restoration remain pending. Historical notes below that say
  no editor exists or startup always exits are superseded by this entry.
- Validation: Release Ninja and Debug Ninja Multi-Config each passed 10/10
  CTest entries. Native Windows editor tests passed 11/11 Qt Test cases
  (including setup/cleanup); inspected legible Layout/Effects screenshots.
  Deployed Release `--inspect` exited 0 with Qt SDK paths removed. Graphify
  updated; existing partial parser limitations remain. No real process
  termination or hardware output was part of these checks. This increment
  includes the previously local module/catalog/discovery/startup work below.

## Reference reuse review (2026-10-23)

See `docs/REFERENCE_REUSE_REVIEW.md` for pinned source inspection of CompGen,
Lumen, LightScript Workshop, Virtual RGB Rig, SignalBridgePlugin and Lueur.
Recommended next work: independent Qt scene/layer/layout implementation,
typed effect controls, validated external component import, shared sampling,
latest-frame scheduling, explicit reconnect handling and QSaveFile persistence.
Lumen's target masks do not implement our requested paintable soft masks.
These are recommendations; script hosting, 3D, audio and optional external
backends remain separate decisions. No production code or dependencies added.

## Accepted effect authoring follow-ups (2026-10-23)

The user accepted SignalForge-inspired image import/cropping, shapes, gradients,
particles, motion controls and color animation. Integrate all as effects in the
shared layer/mask editor. Delivery order: editor/Static Color, gradients/shapes,
images, motion/color, then particles/trails. See the design spec's accepted
effect authoring scope. These remain planned; no editor code was added.
SignalRGB export and upstream project compatibility are not part of this scope.

## Updated editor requirement (2026-10-23)

- Layout, lighting effects and canvas share one Lighting category, with separate,
  mutually exclusive Layout and Effects editing views. A viewport may be reused;
  simultaneous component placement and effect editing are explicitly rejected.
  All editors use Photoshop-style tools and contextual properties; Effects
  provides layer editing. Retain selection/zoom/pan independently for each view.
- Each new effect layer owns one effect instance immediately; use Static Color
  as the initial default. Include ordering, visibility, opacity, duplication,
  deletion and undo/redo in the first editor increment.
- Component placement belongs to Layout; canvas/layer/mask editing belongs to
  Effects. An optional Effects LED overlay is read-only. Preserve
  independent channel targets and explicit Native / Software modes; firmware
  modes cannot be composited as software layers. Preview stays hardware-free.
- Every effect layer also owns an initially white grayscale mask. Include
  mask thumbnails/selection, soft grayscale brushes, gradients, fill/invert,
  bypass and undo/redo in the first editor increment. Mask coverage multiplies
  effect alpha and layer opacity, revealing underlying effects continuously.
  Duplicate masks independently; persist them when document saving is added.
- Each layer has a separate Transparency slider (0% visible, 100% transparent)
  and a blend-mode selector: Normal, Multiply, Screen and Overlay. Blend modes
  combine the selected layer with the composite below it. Include these in
  the first editor increment; this supersedes the earlier blend-mode deferral.
- The latest clarification supersedes simultaneous shared-canvas editing;
  separate views are required within one category. The layer stack remains
  part of the first editor increment. See the unified editor contract in the design
  spec. Documentation only; editor UI and rendering remain unimplemented.

## Completed: first v1 increment (2026-10-23)

- Added the C++17 CMake project and a Qt6 executable scaffold.
- Implemented `BuildReport0x50` and `BuildReport0x51` as pure serializers,
  following `docs/MB_800_HID_PROTOCOL.md`; neither performs hardware I/O.
- Report `0x50` defaults to transient settings (`Store=0`). Invalid lighting
  modes, color counts, brightness, and speed throw `std::invalid_argument`.
- Report `0x51` rejects unknown area addresses and more than 240 LEDs.
  The upcoming driver must enforce each board channel's narrower limits.
- Added protocol tests and an offscreen application startup smoke test.

## Validation

Built with MSVC 19.51, Ninja, and shared Qt 6.8.3 x64 on Windows.
Both CTest entries passed. The protocol suite has five substantive test
slots covering native record boundaries, option packing, invalid native
settings, per-LED layout/padding, and packet capacity limits.
No device was opened and no USB reports were sent.
CTest verification ran outside the agent sandbox: inside it, Qt's offscreen
platform plugin was not discoverable and the app test hit its 15-second
timeout. Direct application startup also passed outside CTest.

## Completed: HID transport (2026-10-23)

- Added `IHidDevice`, the owning `HidApiDevice` adapter, and test-only
  `FakeHidDevice`. Failed fake opens leave the fake closed, correcting the
  inconsistent example in the historical plan.
- HIDAPI initializes once per process before wrappers finish construction;
  its shutdown follows wrapper destruction. Closing one wrapper does not
  shut down another. Wrappers cannot be copied.
- Closed handles, null/empty buffers, oversized buffers, short writes and
  HIDAPI errors return failure. Reopening closes the previous handle first.
- Added HIDAPI SDK discovery through `hidapi_ROOT`; validated against the
  existing x64 HIDAPI 0.14 SDK beside OpenRGB, without copying vendor code.
- Built the production transport static library. Four CTest entries passed
  outside the sandbox: protocol, backend doubles, initialization failure in
  a separate process, and Qt app startup. No devices were opened.
- Backend tests compile the real adapter with replacement HIDAPI functions;
  actual HIDAPI DLL loading and real-device behavior remain unverified.

## Completed: local hardware module framework (2026-10-23)

- Added a versioned Qt plugin contract and loader, generic RGB/native effect
  types, and optional LED discovery with an explicit Unknown status.
- Added strict device-profile parsing and descriptor matching. The MSI module
  validates protocol addresses and receives a host-owned transport.
- Built `msi.gen1` as a separate DLL with a B850 Edge TI JSON profile. No
  registry code changes are needed to load another module ID. Profiles stay
  separate from executable drivers and attached-product definitions.
- App startup now checks local modules/profiles and reports errors, then exits.
  It does not open devices, enumerate hardware, terminate processes, or send
  USB reports. The default Release output is `build/ninja/Release/`.
- Used the existing Lighting Studio worktree as a protocol reference for
  Advanced arming and the IO COVER direct-mode limitation; did not merge it.
  This checkout's driver has not been tested on real hardware.
- Module integration tests use actual DLL loading with a fake transport.
  Repository downloading/installation remains deferred.
- Validation: all six CTest entries passed in both Release (Ninja) and Debug
  (Ninja Multi-Config), using MSVC 19.51 and shared Qt 6.8.3. This includes
  a second, independent test-only protocol with no host dispatch changes.
- Updated Graphify; its parser reports partial extraction for Qt-heavy headers
  and test macros. Compiler/CTest results are the validation source, not graph
  completeness. Working changes have not been committed or pushed yet.

## Historical next steps (superseded)

SignalRGB group research is recorded in `docs/SIGNALRGB_REPOSITORY_RESEARCH.md`.
All 16 public projects were inventoried; the central repository's newer beta
branch, component definitions, CI, exact MSI driver, repository add-on entry
points and translation UI sources were inspected. Prioritize a data-only
component/profile catalog for repo downloads; native DLLs are not required
for every product. Explicit lifecycle/conflict and endpoint requirements must
be addressed before stabilizing the module SDK. A script host is still a
design option, not an implemented compatibility layer. B850 LED detection
remains unqualified; public source helpers do not prove a working board path.

Startup conflict-dialog and discovery requirements were added to the v1
design on 2026-10-23. Implement that gate before application integration:
Terminate all / selected Terminate / Ignore / Cancel, with no automatic
process termination. HID enumeration, interface selection, exact-path open,
and board-profile identification are still missing from the current backend.

Continue with Components/Channels, HID enumeration and exact-path opening,
then board-profile identification and startup conflict handling. The MSI
driver now lives in a module; the real transport is still disconnected from
the application. Research actual attached-LED discovery before declaring it
unsupported. See `docs/HARDWARE_MODULES.md` for the extension contract.
The device model, effects, renderer, persistence, tray, autostart, production
integration and hardware verification remain unimplemented on this branch.
Lighting Studio is a subsequent increment, not part of this scaffold.

Differences from the historical implementation plan:

- HIDAPI discovery was added with the transport rather than the scaffold.
- The real protocol suite replaces the arithmetic-only scaffold test.
- Encoders reject invalid settings instead of silently masking their bits.
- Plan checkboxes are retained as historical instructions, not an up-to-date
  completion ledger; use this file for current progress.

Before the first real-device run, follow Task 12's safety gate: show the
exact outgoing bytes and intended zone changes, and obtain explicit user
confirmation. Passing serializer tests does not qualify hardware behavior.

## Completed: local component catalog and channel mapping (2026-10-23)

- Added schema-1 data-only component definitions with stable IDs, LED geometry,
  and strict permutation/array validation; bounded 1 MiB reads and filename
  diagnostics. Catalog imports reject duplicate IDs and fail atomically.
- Added channel-chain mapping for repeated products, capacity limits and exact
  fixed counts. This represents configured topology, not physical detection.
- Added an original generic 8-LED strip, generated into the app layout, and
  `--component-dir` startup validation. No upstream assets were copied.
- UI assignments, persistence, repository downloads/updates and LED rendering
  remain pending. No hardware operations were added.

- Windows test targets now copy their CMake-resolved runtime DLLs beside each
  executable, including Qt6Test/Qt6Core and matching Debug variants. This fixes
  direct test launches without the Qt SDK bin directory on PATH. Application
  packaging and platform-plugin deployment remain separate work.

## Editor reference (2026-10-23)

See docs/SIGNALFORGE_EDITOR_RESEARCH.md for the SignalForge UI/source review.
Use its preview-first layout and shared-renderer principle as design references;
retain independent channel modes and add component/LED layout inspection.
No upstream implementation was copied and no editor UI has been added.

## Completed: read-only hardware discovery (2026-10-23)

- Added HID descriptor enumeration, copied endpoint paths/interface/usage data,
  and exact-path opening in the real backend. Enumeration never calls open or
  sends reports; exact-path opening is only exercised against doubles.
- Added OS board identity (Windows BIOS registry, Linux DMI files), conversion
  to profile identity and matching that retains all candidates. The app offers
  `--scan`; duplicate profile IDs are rejected. Missing identity does not match
  required board selectors. No automatic interface selection was added.
- Windows app builds copy the matching HIDAPI runtime DLL. Interactive endpoint
  selection, conflict resolution, device connection and hardware writes remain
  pending. Earlier notes describing enumeration as missing are superseded here.

## GitHub topic triage (2026-10-23)

See docs/SIGNALRGB_TOPIC_RESEARCH.md. Highest-value additional references are
CompGen for component authoring and SignalBridgePlugin for a possible script
backend; Lumen has concrete conflict-process detection. No dependency installed.

## Latest verification (2026-10-23)

- Release Ninja and Debug Ninja Multi-Config: 8/8 CTest entries passed,
  including enumeration/exact-path doubles and ambiguous candidate matching.
- Live read-only `--scan` succeeded: Windows reported Micro-Star International
  Co., Ltd. / MPG B850 EDGE TI WIFI (MS-7E62); one MSI profile/endpoint pair
  matched. No device was opened and no feature/output reports were sent.
- Graphify updated (502 nodes); Qt-heavy headers/test macros still cause partial
  parsing warnings. Compiler and test results are authoritative.
- Changes remain uncommitted/unpushed. Next implementation: interactive exact
  candidate selection and startup process-conflict handling before connection.

## Completed: startup conflict gate (2026-10-23)

- Added exact-name Windows process snapshots with PID, accessible path and reason.
  Normal startup opens the conflict dialog before loading driver modules. Empty
  scans continue automatically; failures require retry, explicit Ignore or Cancel.
- Added multiselect Terminate, Terminate all, Ignore, Cancel and Rescan. Normal
  windows receive a close request first; remaining selected processes can be
  forced after 1.2 seconds. Cancel prevents the delayed force action. Snapshot
  creation time/path are checked again using the target handle to guard PID reuse.
- Known service-owned helpers require manual stop. No automatic kill, elevation,
  service configuration or autostart changes. Non-Windows backend is explicitly
  unsupported for now; users may Ignore or Cancel that startup check.
- `--inspect` validates catalogs without the dialog; `--scan` remains read-only.
  Tests use those flags to avoid interacting with the user's actual applications.
- Windows builds now deploy app Qt runtime DLLs and the Windows platform plugin.
  No actual process was terminated, and no device was opened or written.
- Validation: Release and Debug CTest each passed 9/9 entries. UI actions used
  fake processes only; the actual Windows process snapshot was tested read-only.
  Inspected a generated dialog screenshot. `--inspect` also passed with Qt SDK
  paths removed, using the deployed Windows platform plugin and local DLLs.
- Graphify updated; existing Qt parsing limitations remain. Next: endpoint/profile
  selection UI and initial editor preview. Real termination and hardware output
  remain unqualified; changes remain local and uncommitted.
