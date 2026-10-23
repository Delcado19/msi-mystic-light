# Implementation status

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

## Next steps (updated)

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
