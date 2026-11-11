## Reliability and layer-design work (2026-11-11; current)

Current work record: docs/COLLABORATION_FIXES.md. Layer proposal:
docs/LAYER_MODEL_PLAN.md (plan only; no layer-schema/creation-rule change).
Codex and Claude reviewed bounded individual decisions. Healthy service output
workers now survive offline endpoints; recovery scheduling is bounded during
continuous edits; the complete header image is fitted without crop/fade.
The portable startup dialog has a separate fixed-name SignalRGB administrator
stop action using the actual SCM installation path and system sc.exe. It does
not disable tasks/startup, force-kill the service or elevate the editor.
Full Release build passes. The full CTest run passed 21/22 gates; the startup
test double had an invalid pending-plus-error combination. After correcting the
fake, that gate passes too (47.66s). All 22 gates therefore have passing results;
there was no second full-suite run. Full editor gate: 77.84s. Native header and
three startup-state checks: 8 QtTest results, zero skips. Header also passes at
150%/200% scaling. The complete logo is visible in the inspected native screenshot
docs/images/reliability-header.png. Graphify AST updated with existing-style parser
and stale-label warnings; no LLM relabeling was run.
Local package: build/dist/LightingStudio-ReliabilityFix/msi_mystic_light.exe (GS-06-R1).
Archive: build/dist/LightingStudio-ReliabilityFix-windows-x64.zip.
ZIP SHA256: f5b0a2cbb4d3ed16f8ad5791636f294aa8f03b9412c051f0c25460abe15d4b73.
Archive CRC, extracted editor/host equality and SDK-free packaged --inspect pass.
No installed service/editor was updated. Source commit/push does not change that
installation state or qualify the untested real UAC/hardware behavior.
No live RGB output or real SignalRGB stop has been performed in this work.

## Hardware migration inventory created (2026-10-30)

See docs/hardware-inventory/README.md and FAMILIES.md. The pinned Git-object
inventory contains 198 OpenRGB controller directories (188 without coverage,
nine partial source-family overlaps, one excluded DebugController), 2183 detector
registrations / 1975 model labels, 440 SignalRGB JS files and 218 components.
186 JS files have candidate USB-pair overlaps; 154 have unresolved identity/name
exports. These counts do not establish unique device models or future DLL count.
Priorities: active Roccat/Razer/SteelSeries first; motherboard HID and owner AGON
next; broader transports later. Metadata index includes 489 SteelSeries descriptor
files, three Roccat module candidates and one Razer/G-Menu ASAR each.

The SignalRGB clone was sparse and missing promisor objects; fetched the pinned
9511979fa21a0a00a35a236d0aac8c438179094f objects before complete extraction.
HP Omen 30L still has no match in that full pinned public plugin tree. Do not
interpret working-tree file absence as absence from a source snapshot.
Manufacturer catalogs/capabilities remain undecoded. REA reported Razer unpacked
companion integrity disagreement and an initial G-Menu timeout. Follow-up analyses
produced no usable result before caller termination; no capability claims follow.
No application, device, audio setting or driver installation was changed.

Reproduce reference JSON/CSV: python scripts/inventory_hardware.py.
Validation: all 13 docs/tooling tests pass, including five inventory parser/data
consistency cases; git diff --check passes. Graphify refreshed with existing
parser/label-staleness warnings. No product code or shipped binaries changed.
Inventory tests: python -m unittest discover -s tests/docs -p test_hardware_inventory.py.

## Independent driver migration started (2026-10-30)

Owner authorized migration after the discussion-only strategy. First increment:
HP Omen 30L, independent hp.omen30l API-2 output-report module, separate profile,
seven single-color zones, Canvas output and seven native modes. Reference and
SignalRGB snapshot search are pinned in docs/HP_OMEN_MIGRATION.md. No matching
SignalRGB implementation was found in the available snapshot; this is not a
claim about all releases. Hardware is explicitly unverified in the profile name.
Existing hardware profiles and project schema remain unchanged. No hardware I/O.

Installer/module delivery correction: align the six-module portable catalog,
installer copy/hash lists and protected service allowlist. Installer is validated
with WhatIf only; no installed service was changed. Existing installed trees need
the matching host/modules/installer update together.

Validation: Release 21/21 non-editor gates plus editor 1/1 (64.85 seconds),
Debug HP module 1/1, package/inventory tests 5/5. Archive CRC, extracted bytes,
Release editor/host/module equality, SDK-free --inspect and authenticated simulated
host smoke pass. Physical HP output and Linux compilation remain unverified.
Local archive: build/dist/LightingStudio-HardwareMigration01-windows-x64.zip
SHA256: b52a04fa1f7dc925ca06b9fa7595c8445c560a9e2787faa35d8b135fc1ef7575
Portable folder: build/dist/LightingStudio-HardwareMigration01
Start-Lighting.cmd remains on the previously selected startup hotfix; this new
migration package is explicit and does not silently replace an installed editor.

## Hardware strategy recorded (2026-10-30; discussion only)

The owner selected independent hardware modules using OpenRGB and SignalRGB as
reference sources, without requiring either application at runtime. For an exact
model supported by both, inspect both implementations and document differences.
Research also covers installed Roccat/Razer/SteelSeries software for additional
models and the full settings surface, beyond RGB. See
docs/HARDWARE_SUPPORT_STRATEGY.md for scope, evidence and qualification rules.
Additional owner-reported source: AGON monitor utility G-Menu at C:\G-Menu,
including its limited LED presets and other exposed monitor settings. Exact model,
transport and capabilities remain to be investigated.
Exact Roccat Swarm, Razer App Engine/Synapse and SteelSeries GG entry points and
arguments are now recorded in HARDWARE_SUPPORT_STRATEGY.md. The owner also wants
THX Spatial Audio to work; investigate the installed audio path and integration
options separately from hardware lighting modules before promising support.
This supersedes treating an OpenRGB SDK server as the default Linux strategy.
No new implementation, vendor analysis or hardware access was started by this
documentation decision.

## GS-06 startup hotfix (2026-10-30; supersedes previous readiness claim)

The owner reported no GUI. Windows recorded c0000005 in Qt6Widgets.dll during
initial toolbar layout. The owner's sanitized geometry/state fixture reproduced
the startup failure without loading a project or discovering/writing hardware.
Deferred toolbar moves avoided the crash but left the artwork unmanaged; those
attempts were discarded. AppSession now writes/restores window-state version 5,
rejecting older dock/toolbar layouts once. Geometry, explicit panel visibility and
project/recovery data remain independently restored. No personal settings or
project files were edited. Current version-5 arrangements persist normally.

Tests cover the recorded startup, first run and old toolbar state. The recorded
case asserts visible full-width artwork after exposure, not merely process survival.
The package whitelist also includes ASRock/Lenovo modules required by the current
profile catalog; missing modules caused the initial archive --inspect smoke to fail.
Corrected delivery: build/dist/LightingStudio-IceRefined-StartupFix/msi_mystic_light.exe
Archive: build/dist/LightingStudio-IceRefined-StartupFix-windows-x64.zip
Final hotfix validation:
- Recorded-state, first-run and legacy-toolbar startup: 5/5 native, zero skips.
- Full native editor: 147/147, zero skips, 67.951 seconds; screenshot reviewed.
- Release CTest: 21/21, 106.57 seconds; Debug: 21/21, 312.89 seconds.
- Package tests: 4/4. ZIP CRC, extracted bytes and tested editor/host equality pass.
- Packaged --inspect and owner-authenticated simulation host pass without Qt SDK.
- Direct packaged GUI launch reaches Restore failed with deliberately empty device
  profiles. Automated acknowledgement did not dismiss that modal; full main-window
  exposure in that exact executable smoke remains unverified. The sanitized native
  AppSession test does expose and capture the corrected window. No hardware writes.
- The original crashed PID 15156 was absent outside the sandbox. Its stale lock was
  moved into build/crashed-15156.application.lock; personal project/settings untouched.
- Graphify AST refresh completed with existing parser/label-staleness warnings.
ZIP SHA256: 42cb13e78fc67fc4b6a4cfd21f1a4bc541bd27d9c47d305d5a5e2146bc0e3a09
msi_mystic_light.exe SHA256: 07c3db614f758dbd0cfcfa3a82043cc25a2f6c27b6c24a307a0bd959035c3d50
lighting_service_host.exe SHA256: 07fd309706826a9e869a9c1ccd6ea1d3faca8cb7f2766ab5a3880cba4455f59a

## GS-06 Ice refinement completed and packaged (2026-10-29)

The owner requested completion after the cleanup/compact interruption. GS-06 is
now locally qualified and packaged, including the larger faded artwork, continuous
effect/mask gesture preview, one Layers row marker, Plasma/Kaleidoscope spatial
controls, independent DNA Wave width, wider Visor band and authorized natural
Lightning/Fireworks renderer refinement. The previous RAM startup fix is retained.
No remaining work is required for this authorized refinement/package scope.

Final gate results:
- Release CTest: 19/19, 98.78 seconds; offscreen editor body 64.31 seconds.
- Debug CTest: 19/19, 298.06 seconds; offscreen editor body 256.54 seconds.
- Full native Windows editor: 145/145, zero skips, 62.998 seconds.
- Native scaling 1.5x and 2x: 6/6 each, including live drag and legacy layout.
- Final settled native captures: 7/7, with wide/compact, new spatial controls,
  About, restored legacy toolbar geometry and matched natural-effect renders.
- Archive CRC, every extracted byte, editor/host equality with tested Release,
  personal-project exclusion, SDK-free --inspect and authenticated simulation-host
  smoke passed. git diff --check passed. No physical hardware output was written.

Completion found and corrected one real compatibility issue: restoring a pre-banner
Qt window state could place the unknown artwork toolbar beside existing controls.
restoreWorkspacePanelPreferences now repositions artwork onto its own full-width
row while retaining other saved dock preferences. legacyArtworkToolbarRestore
reproduced the failure before the fix and passes native/offscreen/scaling checks.
The offscreen drag fixture now fits the camera and asserts a real gesture starts,
avoiding transparent particle centers or navigation interactions as false proof.

The expanded editor suite genuinely exceeded its prior 60-second Release and
240-second Debug limits. Bounded timeouts are now 120/360 seconds, respectively;
complete runs pass within them. Initial timed-out runs are not counted as passes.
Existing offscreen font/plugin warnings and the non-failing invalid-image libpng
diagnostic remain visible in logs; native screenshots use real Windows fonts.

Ready editor:
  build/dist/LightingStudio-IceRefined/msi_mystic_light.exe
Ready archive:
  build/dist/LightingStudio-IceRefined-windows-x64.zip
ZIP SHA256: e54fcd35ea2132deeaf7c5bcb5c067f40ec8d72307a0ef88ed876b11abe28ad7
Editor SHA256: 2188698804c1d332fea0532b5cac35fae29828d0bef04218b3579f33dfa325d8
Host SHA256: 8a50971e9135a45c1430f07c0e5c3497f6a544732cd8e728e0472b25fed10e11

Start-Lighting.cmd prefers an installed service editor, then the qualified GS-06
folder, retaining GS-05 as fallback. Use the explicit GS-06 path if an older
installed editor wins. Editor and host must be updated together for schema 22;
this finish did not install/update any service. Older project files retain neutral
spatial defaults; no personal project was rewritten. Keep older file copies for
older application versions, which cannot read newly saved schema-22 files.

Evidence: build/refinement-full-native.log;
build/{next,multi}/Testing/Temporary/LastTest.log;
build/refinement-scale-{1.5,2}.log; build/refinement-final-captures.log;
build/refinement-legacy.log. New native images were inspected and promoted into
docs/images, including natural-effects-before/after.png. Design/research notes,
README, portable notes and code comments are synchronized. Graphify AST refreshed
with its existing partial-parser limitations; no paid semantic labeling was run.

No commit, push, service installation, user-process termination or physical device
write. Real lighting output, screen-reader runtime and Windows contrast themes
remain unqualified. Earlier unreproduced Rotation disappearance and the separate
split-region/unified-selection workflow remain outside this completed refinement.

## Historical handoff before requested build cleanup / compact (superseded, 2026-10-29)

Owner requested build-folder cleanup, then context compact, while GS-06 refinement
was in progress. Preserve all local uncommitted work; do not restart from master.
Latest fully qualified portable is GS-05 at build/dist/LightingStudio-Ice; GS-06
exists in build/next/Release but is NOT finally qualified or packaged yet.

Authorized scope already implemented locally:
- Larger artwork toolbar (180 DIP desktop / 100 DIP compact or short windows),
  native lower alpha fade, plain navigation underneath, full image still in About.
- One selection marker per Layers row instead of repeated cyan Source/Mask seams.
- Removed the shared automatic pause for effect/mask gestures. Canvas pointer math
  captures gestureSeconds_ while live preview advances; explicit Stop remains.
- Schema 22 adds patternFrequency, patternContrast, patternWarp, patternWavelength
  with neutral old-file defaults. Plasma/Kaleidoscope expose the first three;
  DNA exposes horizontal Wave width independently of amplitude (now up to 45%);
  Visor Band width now reaches 50%. Existing bounds/validation remain intact.
- Owner explicitly authorized Lightning/Fireworks renderer changes after research.
  Lightning: bounded 36-segment growing channels, primary/secondary branches,
  bright core/halo and two return-stroke envelopes. Fireworks: spherical projection,
  drag/gravity, ten tapered trail samples, heads/halos, coherent burst color.
  New Fireworks defaults to 320 particles; UI maximum 400; saved counts unchanged.
- New artwork reduced visible inspector height: primary particle fields were moved
  above palette details, and switching Source/Motion/Mask returns scroll to top.

Evidence and current gates:
- build/effect-reference-before.log passed 3/3. Original matching render sheet is
  build/research/effect-references/app-before.png.
- build/verify-refinement.cmd passed 8/8 after the continuous-drag fixture used
  actual particle corner grips (transparent particle centers are not painted hits).
  This covers continuous preview on Ripples plus seven other effects, explicit
  Stop/cancel, new controls, save/load, old schema defaults, visuals and patterns.
- First full native run (build/refinement-full-native.log) failed five cases:
  three obsolete eraser-pause expectations, old Fireworks default 120, and primary
  particle controls below the visible area. All were addressed; the actual UI
  fix moves particle fields above the palette rather than weakening visibility.
- Latest build/refinement-affected.cmd passed 9/9, including every affected eraser
  row, extended effects/reset and advancedControls. Latest product build is this
  corrected build. git diff --check passed before this handoff.
- Full Debug run was deliberately interrupted because it used the pre-correction
  test binary. It is NOT a passing current Debug gate. No user process was killed.
- Required next: run build/verify-refinement-full.cmd (full native, then Release
  CTest); run build/verify-refinement-debug.cmd (Debug build + CTest). Inspect and
  resolve actual failures; old expectations may need contract updates only when
  justified. Do not bypass behavior checks or claim final success before gates.
- Still needed: final 1.5x/2x native checks; verify the new artwork remains its own
  row after restoring legacy saved toolbar state; inspect wide/compact UI again.
  Source screenshots taken immediately after dynamic control changes looked
  compressed, but allowing the Qt layout event cycle produced correct scrollable
  controls. Final captures must follow settled layout.

Research already completed:
- Inspected NWS/Marcelo Saba 4000-fps sequence (public GIF 285x255), publisher
  branching figure 675x921, David Russell Lightning photo 6000x3081 (20 s exposure,
  CC0), and Berit fireworks 4320x3240 (2 s, CC BY 2.0). Actual downloads and sampled
  GIF frames stay under build/research/effect-references.
- docs/EFFECT_VISUAL_REVIEW.md has sources, comparison and honesty boundaries.
  Its images/natural-effects-before.png and -after.png still need promotion from
  the matching native sheets after final renderer checks. Research photographs
  are NOT application assets and must not be bundled without attribution.
- Native screenshots already inspected: refined-banner.png, refined-plasma.png,
  refined-kaleidoscope.png, refined-dna.png and lightning-fireworks-reference.png
  under build/next/editor-artifacts/windows. Promote final reviewed captures only.

Remaining finish work after tests:
- Update docs/design/README.md and docs/EDITOR_UX_REVIEW.md to current GS-06;
  current README, LIGHTING_EDITOR, PORTABLE_README and asset provenance already
  have GS-06 text. Recheck it against final behavior; do not preserve stale claims.
- Update Start-Lighting.cmd only after preparing a separately named qualified
  GS-06 package, e.g. LightingStudio-IceRefined. README already names that intended
  path, so it must be made real before completion or corrected.
- Package editor AND host together (schema 22), verify CRC/extracted bytes and
  binary equality, run the existing SDK-free inspection/simulation-host smoke.
- Refresh Graphify AST (known partial-parser warnings; no paid semantic labeling).
- Record final results/hashes here. No commit/push/service install or hardware
  write authorized. Do not edit default.lighting.json. Earlier Rotation and
  split-region/unified-selection work remain separate.

Cleanup completed: removed 39 obsolete dist entries, freed 671.5 MiB, zero
failures. Manifest: build/cleanup-2026-10-29.json. GS-05 ZIP SHA256 still matches
its checksum file; current Release/Debug binaries, Qt plugin inputs and reference
research remain present.

Cleanup scope: remove obsolete portable dist folders/archives, retaining GS-05
Ice, its preceding Stealth-Hotfix, and the original 0.1.0 release archive. Keep
build/next, build/multi, Qt WebP build inputs, tests/logs/captures/research and
current helper scripts. Older package paths in historical entries may cease to
exist intentionally; do not treat those entries as current launch instructions.

## Ice image palette and expanded pattern limits (GS-05, 2026-10-29)

Owner supplied a new ice-mountain/aurora Lighting Studio panorama and explicitly
requested its colors/image plus larger Ripples limits and a check of other effects.
The image is copied byte-for-byte into assets/branding/lighting-studio-ice-banner.png
and embedded in branding.qrc. Header/inline preview use its mountain panorama under
a navy scrim; About shows the full image. The compact LS identity stays readable.
Theme roles now use deep navy, ice-white text and cyan focus/selection. Preview
Play/Stop and hardware status keep semantic colors; effect/output colors are not
themed. Current build identity is GS-05. No image generation or new dependency.

All creatable native patterns had UI speed caps below the supported renderer range;
Ripples was limited to 30 cycles/min and 8 repetitions. The shared UI now exposes
-600..600 cycles/min (-10..10 internally) and 1..64 repetitions across applicable
effects. Defaults, zero/reverse behavior, renderer validation and file schema are
unchanged. Other parameters were reviewed and their bounded practical ranges are
retained; no new particle/site/grid workload limits were introduced. High pattern
speed and repetitions are already accepted by current persistence. Regression
coverage checks every creatable pattern, undo/redo, save/load, actual high-limit
Ripples rendering and values outside the existing validation bounds.

Design-taste-frontend app direction: data-terminal, seed 1783922600, with existing
layout/system font and the explicit owner palette retained. docs/design/README.md
records the native adaptation, critique and contrast. Browser HTML inspection is
not applicable to Qt widgets; native captures/pixel checks are the evidence. Full
wide, compact, inline, About and Ripples-control captures were inspected and copied
to docs/images. No stock imagery or invented data was introduced.

Validation: Release CTest 19/19 (84.87 s), full native editor 141/141 with no skips,
focused final Release 8/8, targeted Debug chrome/limits 4/4 and full Debug output UI
16/16. Final 1.5x/2x native chrome/color/workspace checks each passed 5/5. Full
Debug CTest was not rerun for this change. Capture-only fixture additions were
rechecked by the final focused Release runs. Logs: build/ice-full-native.log,
build/ice-focused.log, build/ice-debug-{editor,output}.log,
build/ice-scale-{1.5,2}.log and build/ice-captures.log. Existing libpng Read Error
diagnostic remains non-failing; no claim is made about its origin.

Prepared editor: build/dist/LightingStudio-Ice/msi_mystic_light.exe
Archive: build/dist/LightingStudio-Ice-windows-x64.zip
ZIP SHA256: 7c679f1c70beabf14ac1ea4d76227e91530301719fab10866565c58515f40cfb
Editor SHA256: 0767e2dc69ae5fb7eb284f885a9e4971ab7416e1bf56910819939367f284c304
Host SHA256: 2350c7299382434f1719ef24fbc76360aef3446c3622c8692baeece56f43f6ed
CRC, extracted bytes, tested-binary equality, user-project exclusion, SDK-free
inspection and authenticated simulation-host smoke pass. Start-Lighting.cmd
prefers an installed service editor, then Ice, preserving the earlier hotfix and
all prior fallbacks. Launch the explicit Ice path if an older installation wins.
The deferred-RAM-start fix is retained. No service install, physical output write,
user-project edit, commit or push. Hardware output, screen reader and Windows
contrast themes remain outside the validation. Separate selection/Rotation work
listed below remains open.

## Inline header and delayed RAM startup hotfix (2026-10-29)

Owner reported a truncated red header rule in Layers and PawnIO RAM stopped on
startup even when elevated; manual Resume starts the RAM. Both mechanisms were
reproduced before editing: stealthStudioChrome failed its inline-rule check and
delayedDiscoveryStartsMissingOutput failed to reach Running with a healthy peer.

Qt saved layouts can join Preview to History. Preview now uses shared header
painting only while on the History row, continuing the panorama and red rule;
its separate-row compact appearance is retained. Native inline and split captures
were inspected, including the new docs/images/stealth-inline-header.png.

GUI output starts before the queued active RAM scan. Passive authorization skips
unverified RAM while HID connects. The old hardwareDiscovered handler only
refreshed labels, and mixed recovery only retried scopes that already connected.
Completed discovery now starts missing configured endpoints while automatic output
is enabled. Healthy workers are retained; paused channels and global Stop remain
respected. No PawnIO protocol, mutex permissions or privilege requirement changed.
The new test reproduces delayed discovery, checks the peer is not reauthorized,
and verifies discovery does not resume output after global Stop. Existing mixed
recovery tests continue to cover per-channel pause and fault retry behavior.

Verification: native Windows editor 140/140; focused header at 1.5x and 2x each
3/3; Debug header 3/3 and complete Debug output UI suite 16/16, all without skips.
Logs: build/header-regression-before.log, build/ram-start-before.log,
build/header-regression-after.log, build/ram-start-after.log,
build/header-ram-full-native.log, build/header-scale-{1.5,2}.log,
build/header-ram-debug-{editor,output}.log. Release CTest passed 19/19 in 66.36 seconds; git diff --check passed.
Graphify AST was refreshed with its existing partial-parser warnings; no semantic
labeling was run.

Prepared editor: build/dist/LightingStudio-Stealth-Hotfix/msi_mystic_light.exe
Archive: build/dist/LightingStudio-Stealth-Hotfix-windows-x64.zip
ZIP SHA256: aa47e2dbe8c8f3fb491d9a5fcc9d5c38e297539c5202ee2f22b90d346b57fd58
Editor SHA256: 8beeae434e1c22e3bbda9c0ae45682bbcebb14895ee98c3fc764441a1d9dfb92
Host SHA256: 29d702e0eec17b0143f4b123107f78bc83c95a7725758b7dfe7452c14a4b8712
CRC, extracted bytes, tested-binary equality, personal-project exclusion,
SDK-free inspection and isolated simulation-host smoke all pass. Starter prefers
installed service editor, then this hotfix, retaining previous portable packages.
No user process was terminated, service installed, personal project changed,
commit or push made, or real-device output written. Live process inventory via
CIM was denied; physical RAM auto-start remains for owner validation. The service
path already reconnects after its scan and was not changed by this GUI fix.

## Stealth redesign completion (GS-04, 2026-10-29)

The owner explicitly selected the latest Stealth implementation over the earlier
Glacier reference. The interrupted code already contained graphite/red theme roles
and a focused stealthStudioChrome regression; the old handoff, package and design
documents still described cyan GS-03. This section supersedes those theme claims.

Completed the local finish: GS-04 build identity in window titles and tests,
Stealth documentation, inspected native wide/compact and aligned-color captures,
and a separately named portable package. Retained the shared semantic theme,
responsive panels, keyboard access, exact numeric values, independent palette/hue
controls and preview/output separation. Existing comments describe theme/output
separation and shared panorama coordinates; no new behavior guard was needed.
Two stale pixel expectations for the scrollbar and canvas header were updated to
the selected colors. Initial native run had 138 passes and those two failures;
the corrected native run passed 140/140 without skips. The obsolete local Glacier
runner also named a removed test; final checks use stealthStudioChrome instead.

Prepared editor: build/dist/LightingStudio-Stealth/msi_mystic_light.exe
Archive: build/dist/LightingStudio-Stealth-windows-x64.zip
ZIP SHA256: 4b8f534668c07d0921b7f1c84e7d201e8fb09c271744d7986561ca9ced69bf04
Editor SHA256: 0f13332f9551d8f7aa6eae1947148534ac88f5a38ec34d82b9ec7d8bd83f60c1
Host SHA256: d085029d1d983901a411b7cf241aac15c92c20960155776b3c957112d1c7aedc
Archive CRC, every extracted byte, editor/host equality with the Release build,
and absence of personal projects were verified. SDK-free packaged --inspect and
owner-authenticated simulation-host status passed without hardware access.
Start-Lighting.cmd prefers the installed service editor, then Stealth, retaining
GamingStudio and older fallbacks. Launch the explicit Stealth path if an older
installed editor takes precedence. Earlier packages are preserved.

Native evidence: build/gaming-studio-full-native.log (140/140),
build/stealth-scale-1.5.log and build/stealth-scale-2.log (6/6 each),
build/stealth-captures.log (4/4). Tests cover chrome pixels, alignment, panel focus,
compact behavior, rendered-output invariance, geometry and document revision.
The native suite emitted a libpng Read Error diagnostic but completed without
failed cases; its precise origin was not investigated in this finish.
Release CTest passed 19/19 in 66.11 seconds; Debug CTest passed 19/19
in 180.69 seconds (editor body 142.33 seconds). Full gate logs are
build/next/Testing/Temporary/LastTest.log and
build/multi/Testing/Temporary/LastTest.log. git diff --check passed.
Graphify AST was refreshed; 11 files retain partial-parser warnings and
semantic community labels remain stale. No paid labeling was run.

No commit, push, service installation, physical hardware write or user-project
change. The split-region/unified-selection workflow and the unreproduced
project-specific Rotation disappearance remain separate open work. Screen-reader
runtime, Windows contrast themes and physical output are not qualified here.

## Editor UX, Pan crash and four sources (2026-10-28)

User reported a crash and rejected the Hand text button and palette snap controls.
Windows events 1000/1001 recorded portable editor c0000005 in VCRUNTIME140.dll at
19:57:01. Minidump exists under LocalAppData/CrashDumps; stack was not decoded.
Independent pre-fix Debug reproduction (build/pan-crash-debug-before.log) confirms
Pan plus document/property refresh indexed an eight-entry hint list as tool 8.
The original Release test returned an invalid hint; Debug aborted in QList::at.
Pan now has an explicit bounded hint, exclusive icon state, hidden drawing fields,
menu shortcut H, focus-loss Space reset and minimap priority. This is a proven
mechanism; exact attribution of every possible crash remains outside that evidence.

All snapping is under Align / Snap; the duplicate per-palette control is removed.
Native UI audit drove source descriptions/key controls first, inline opacity,
clearer panel menus, smaller strokes, navigator label space, and palette Ctrl-arrow
editing. A hidden startup screen anchor was also found visually to send sources
outside the view; startup fits until visible, later mode switches retain the
captured anchor. See docs/EDITOR_UX_REVIEW.md for before/after evidence and scope.

Aurora, Lava Lamp, Checkerboard, Kaleidoscope append schema-19 IDs with deterministic
native rendering and contextual controls/categories. Descriptions/reset defaults
share source metadata. No upstream code/assets or new dependencies. Existing
projects remain readable; no personal project, service or hardware was changed.
Final design review also found hidden-panel preferences lost after restoreState.
AppSession persists window/panels separately from temporary dock visibility;
LightingWindow restores editing preferences, preserving compact-mode intent.
Legacy sessions fall back to saved Qt visibility. Native and offscreen restore/
mode-switch regressions pass. The offscreen test explicitly requests wide mode
because Qt restoreGeometry otherwise clamps to its tiny virtual monitor.
The expanded fifteen-source Debug suite takes about three minutes; its timeout
is now 240 seconds (Release remains 60), avoiding the former 180-second cliff.
Final native Windows editor passed 136/136 with no skips. Release CTest passed
all 19 tests across the full run and corrected offscreen wide-mode rerun. The
expanded Debug editor passed in 187.87 seconds under its 240-second budget; the
full Debug CTest passed 19/19. A QT_SCALE_FACTOR=1.5 native run passed
Pan/design/Hue checks (Windows constrained oversized test geometry to its monitor).
Primary/compact editor, centralized snapping and fifteen-pattern gallery captures
were inspected and copied into docs/images. Both portable binaries match tested
Release. SDK-free --inspect and authenticated simulation-host smoke passed without
hardware access. ZIP SHA256:
ca0c80b84055fd849c1b14115bec293313c75286e6749df38fbe61043ed87623.
Original pre-update EXEs, including the crashed portable editor, remain as
.before-ux-polish backups. No service install, hardware writes, user-project
changes, process termination, commit or push. Prior uncommitted work is preserved.

## Authoring controls, pan and two patterns (2026-10-28)

Hex editing now preserves modified focused text during preview refresh and commits
on Return. Regression covers the overwritten-edit trigger. The existing source
replacement selector lives in a labeled collapsed disclosure. Source menus group
organic/noise and scenes separately. Color stop snapping is renamed and explained.
Voronoi speed now -30..30; Radar labels/tooltips explain signed clockwise sweep
revolutions. Flame detail extends to 16, height to 200; Fireplace fades its source
footprint to transparency independently of the final palette color.
Hand (H) supports left-drag camera pan in both contexts; workspace margin pan is
also available. Smaller drawn grips/gradient guides retain usable hit targets.
Plasma and Ripples are native deterministic patterns reusing palette/geometry/masks.
Schema 18 adds source IDs; older source/parameter defaults stay compatible.
Earlier rotation disappearance remains unresolved without a reproducing project.
Release CTest passed 19/19; full native Windows editor passed 133/133 with no
skips. The camera-switch regression initially exposed dock-width relayout drift;
camera transfer now preserves the captured screen anchor after layout settles.
Native eleven-pattern gallery and soft Fireplace output were inspected. Portable
editor/host hashes match Release; SDK-free inspection and simulation-host smoke
checks passed. Prior EXEs are retained as .before-authoring-controls. ZIP SHA256:
c79d4b48e6c2aa24b36e31c9812f3dc3f41ead865c7657024943a6a2aba186fc.
Debug initially hit its existing 120-second timeout after the new cases passed.
Only the Debug editor timeout was raised to 180 seconds; Release stays at 60.
The failed editor test reran successfully in 126.92 seconds; all 19 Debug tests
are now green across the full run and focused rerun. No hardware writes, service
install or process termination.

## Hue spectrum fix and extended effects (2026-10-28)

Reproduced the user-reported Hue track corruption with a native pixel test: at
359, a sampled track pixel changed from #caff00 to inherited slider fill #55deff.
ColorPicker now overrides both sub-page/add-page backgrounds with transparency;
the full rainbow remains visible and HSV output approaches red (#ff0004 at 359).
Picker numeric suffixes are removed, matching the requested plain values. Existing
live apply/cancel/history behavior remains covered.

Added independent native Voronoi, Fireplace, Radar and Fireworks sources. Voronoi
uses bounded seeded sites with Euclidean/Manhattan/Chebyshev distance. Fireplace
uses coherent seeded noise with detail and height controls and a warm default
palette. Radar starts circular with a moving trail, rings and crosshair. Fireworks
uses four staggered launchers, ballistic sparks, fading and tails through the
existing particle controls. All evaluate from absolute time; no script runtime,
new dependency or upstream effect source was imported. Warm Release renders in
focused tests were approximately 3..13 ms each, not a multi-effect FPS guarantee.

Multizone is an explicit four-region template with ordinary child sources and
linked quadrant masks. One atomic edit creates all five records; a slot-limit
failure preserves state/history. Region names describe position independently
from the selected effect. Source editing, palettes, masks, geometry, save/load and
undo use the normal editor path. Initial splits are fixed; no dedicated split
sliders or upstream scene/contact-blip parity is claimed.

Schema 17 adds the five new procedural parameters. Metadata retains minimum
schema versions, so real schema-16 files without the new keys load with original
defaults. Source creation and Reset to default share effect-specific rate/count
metadata. Editor and host must be updated together. The task did not alter any
personal project or install a service; the earlier Rotation report remains open.

Focused native tests pass for spectrum rendering, RGB output, history, sources,
metrics/seeds, freeze, save/load, invalid inputs, legacy schemas, zone isolation,
capacity rejection and per-effect Reset. Full native editor: 132/132, no skips.
Final Release and Debug CTest each passed 19/19; the Debug editor body took
113.47 seconds under its existing 120-second limit. Both prepared portable EXEs
match the tested Release binaries; SDK-free editor inspection and authenticated
simulation-host smoke checks passed without hardware access. The EditorControls
ZIP was regenerated; prior EXEs remain as .before-extended-effects backups.
No service was installed and no process was terminated.

## Pattern authoring and practical ranges follow-up (2026-10-28)

User reported typed-only repetitions, periodic wave color seams, unclear Stop snap,
rotation disappearance, implicit child sources and excessive generic speed limits.
Repetitions now reuses the standard slider/numeric control. Sine Wave/DNA palettes
close at each period. Color point step explains its percentage snapping, enabled
only with the grid and bypassed by Alt. Native pattern creation uses independent
roots; existing hierarchies and masks are not migrated.

Pattern rate display limits: Pinwheel -60..60; Spiral/Visor/Sine Wave/DNA -30..30;
Pixel Fill -20..20. Other effect controls use practical widths, amplitude, grid,
frequency, motion and particle ranges documented in LIGHTING_EDITOR.md. Suffixes
and range rows are removed from effect controls. Separate slider bounds preserve
saved extreme values and exact project precision until the user edits them.
The toolbar no longer assumes a removed parameterLimits label exists.

Regression tests cover periodic palette boundaries, all rotation angles, direct
Motion rate editing with visible output, root creation beneath a hidden/masked
selection, slider editing and undo, practical limits and preserving legacy high
values. Native source/control/motion screenshots were inspected.

Rotation disappearance remains unresolved: isolated native Motion edits and full
rotations retain the effect. The available recovery snapshot initially contained
only an empty parent and Pixel Fill; it later ceased to exist. No Sine Wave/DNA
project reproducing the report was available, and no user project was modified.
Do not describe that report as fixed based only on the passing isolated tests.

Release and Debug CTest passed 19/19 each. Full native Windows editor passed
130/130 with no skips. Portable editor/host hashes match the tested Release
binaries; SDK-free inspection and simulation-host smoke checks passed. Both EXEs
and the prepared ZIP were updated. Previous EXEs remain as
.before-pattern-controls backups. No app was terminated or service installed.

## Native procedural effect batch (2026-10-28)

Implemented six independent native sources: Gradient Pinwheel, Spiral, Visor,
Sine Wave, DNA and Pixel Fill. They reuse gradient palettes, masks, blending,
geometry, source menus and the shared editor/service renderer. Pattern rate uses
loops/minute with zero and reverse; only applicable controls are visible. Pixel
Fill supports Rows/Snake/Spiral order and square grids, while wave sources expose
width/amplitude/frequency. Pinwheel/Spiral close the palette and center their
existing gradient grips correctly. No upstream effect scripts were imported.

Project schema 16 persists the five pattern parameters. Metadata records their
minimum schema, preserving actual schema 1..15 projects without the new keys.
Older schemas reject the new effect IDs rather than reinterpret them. The
installation owner must update both editor and service together; no service was
installed or hardware written during this work. Personal default.lighting.json
and prior uncommitted work remain untouched.

Completed candidates are struck through in docs/EFFECT_BACKLOG.md. Remaining
noise, full fireworks, Voronoi, screen/audio/sensor and topology-specific sources
remain listed. The supported subset and its limits are documented there and in
docs/LIGHTING_EDITOR.md. Native screenshots were inspected; the contact sheet is
docs/images/native-patterns.png. Tests cover motion/freeze/reverse, the three
fill orders, controls/undo, persistence, invalid input and old-schema defaults.
Release CTest passed 19/19; full Windows-native editor passed 129/129 with no skips.
Debug CTest passed 19/19. Both portable EXEs match the tested Release binaries;
SDK-free editor inspection and authenticated simulation-host smoke checks pass.
Start-Lighting.cmd resolves the updated EditorControls package (no installed
service was present). Previous EXEs are retained beside it with a
.before-native-patterns suffix. No running application was terminated.

## Active-layer picking, canvas labels and transport controls (2026-10-28)

Local follow-up to the fullscreen changes below; no new commit/push was requested.
The service baseline remains 0de3946 on origin/master. Preserve the personal
untracked default.lighting.json and all previous local fullscreen fixes.

Layers already had an activeLayer_ maintained by the navigator, but hitsAt and
marquee ignored it. Both now restrict new picks to that root, so a covering source
in another layer cannot steal selection. Empty clicks keep the active root.
Cross-layer changes remain available explicitly through the navigator. Existing
same-layer overlap menus, covered-selection drags, locks and multi-selection remain.

setShowLabels reached both canvases, but paintComponents skipped captions in
Effects context. Composite now draws the same component captions in world space;
caption clipping stays inside the viewport, including labels outside the effect
raster. Label toggles do not move the camera, edit the document or affect output.
The shared persisted action is named Labels. It now drives canvas header buttons
at top-right, beside fullscreen in Components. Both positions ignore scrollbar
gutters. The View action and temporary L reveal remain; Line uses I.

Preview retains one checkable QPushButton, now icon-only green Play / red Stop.
Accessible names and tooltips expose the current action. Stop freezes time without
resetting it or stopping hardware connections. The fullscreen transport mirrors
both the state and colored icon. UI status says Preview: stopped instead of paused.
ServicePanel still uses the unchanged playPreview object/isChecked contract.

Added activeLayerPickingAndCanvasControls: actual navigator-root selection through
covering objects, empty-click persistence, active-root marquee, selecting another
root, a pixel-region difference proving composite captions were painted, shared
label preference, header anchoring, icon colors/accessibility and stopped time.
Updated old text assertions for the new transport. Release CTest 19/19 (13.44 s),
Debug CTest 19/19 (44.59 s), full native editor 127/127 (15.73 s), no skipped cases.
Native focused cases pass 6/6 each at 125/150/200% (base125%, factors1/1.2/1.6).
Logs: build/editor-controls-{release,debug}.log,
editor-controls-native-full.log and editor-controls-dpi{125,150,200}.log.
An initial focused invocation named a nonexistent holdComponentLabels test; it
was corrected to the actual labelRevealControls name, which passes. No test was
bypassed. Native screenshots of both modes, composite labels and fullscreen were
reviewed; docs/images contains native captures, not offscreen placeholders.

Prepared build: build/dist/LightingStudio-EditorControls/msi_mystic_light.exe.
Start-Lighting.cmd prefers installed editor then this folder, preserving all old
package fallbacks. No live app/service was replaced or installed. Archive:
build/dist/LightingStudio-EditorControls-windows-x64.zip
ZIP SHA256: 30cd5853960bc616b44d9a141b355bb287c44d0f26a608edc34331d8d42f3c88
Editor SHA256: da9f07c252c61f71dce88a2ca46459c988e336fc621a27c6ec243828f54cd489
Host SHA256: acc812a754ee5dccca81b6f4ec6146f409d3ef30d35c9c873268dd1eacf28c9f
Archive CRC, every extracted byte and tested-binary equality verified. Packaged
--inspect and a packaged owner-authenticated simulation pipe pass without Qt SDK
environment. No physical hardware writes; only owned simulation children stopped.
Personal project SHA256 remains
9cc3be379525f95e26d75b9bc27977cac9b3bdb3a6c126ce55fe836473a812a5.
Graphify AST refreshed (known partial-parser limitations); no paid semantic labels.
These GUI checks do not qualify physical output or the earlier service's UAC,
SYSTEM device access, cold boot or standby behavior.

## Components fullscreen and edit-mode visibility fix (2026-10-28)

Owner reported a jumping fullscreen button when canvas scrollbars appear, broken
fullscreen rendering, and inconspicuous Components/Layers controls. This follow-up
is local; no new commit/push was requested. Prior service implementation is committed
and pushed as 0de3946. Do not stage personal default.lighting.json.

Root causes were reproduced in native Windows screenshots: syncScrollbars moved
an otherwise fixed header button by the vertical gutter width; the fullscreen
QVBoxLayout gave its status-header layout roughly half the monitor height because
neither header nor canvas had positive stretch. Move positioning now belongs only
to resizeEvent, and the canvas receives stretch=1. The header stays compact.
Qt 6.8 QBoxLayout addWidget stretch behavior was verified through Context7:
https://doc.qt.io/qt-6.8/qboxlayout.html.

The existing QTabBar editorTarget remains the API/keyboard control, wrapped in a
labeled EDIT MODE group. Icons, stronger selected background/border and tooltips
make Components/Layers prominent. Canvas views and disabled targets retain their
existing behavior; no new editing mode or hardware behavior was added.

Regression assertions cover identical fullscreen-button position before/after
scrollbars, canvas occupying >80% of the fullscreen host, compact status/playback
geometry, restored canvas parent on Escape, and visible labeled/icon mode controls.
Release CTest 19/19 passes (13.38 s). Targeted native Windows cases pass 12/12;
focused fullscreen/mode/scroll tests pass 6/6 each at 125%, 150%, 200% (machine
base scale 125%, factors 1, 1.2, 1.6). Native fullscreen and both target screenshots
were visually reviewed; docs/images includes the reviewed native images only.
Debug CTest also passes 19/19 (42.62 s), recorded in build/fullscreen-debug-ctest.log.
Graphify AST refreshed with its known partial-parser limitations; no semantic LLM
labeling was run. No physical hardware writes or real controller termination.

Prepared build: build/dist/LightingStudio-Fullscreen/msi_mystic_light.exe.
Start-Lighting.cmd prefers an installed editor, then this new portable folder,
retaining Service/Unified/older fallbacks. The existing service install is not
modified automatically. Archive: build/dist/LightingStudio-Fullscreen-windows-x64.zip
ZIP SHA256: 0788053b8dc20a2f6852837764147c742fc0b286ac762d7e1bdd719d621c7d75
Editor SHA256: b1cce135e0ecbe9680d54d507edb41ca5f66df189a7af56e06a5f30c80738a73
Host SHA256: 7f00a8f7496044947593e66ba92f3b0ea47cb92415e2286665756670dc6b9f96
Archive CRC, every extracted byte and binary equality to tested Release checked.
Packaged --inspect exits 0 without Qt SDK environment. Personal project SHA256
remains 9cc3be379525f95e26d75b9bc27977cac9b3bdb3a6c126ce55fe836473a812a5.
Remaining limits: no physical RGB validation or live service installation; this
fix does not qualify the prior service's cold-boot/SYSTEM hardware behavior.

## Windows service implementation (2026-10-28)

Owner authorized implementation, commit and push. Source entry points:
`src/service/`, `installer/Install-LightingStudio.ps1`, and docs/WINDOWS_SERVICE.md.
This supersedes earlier statements that service implementation was deferred.
The prior UI change is already committed/pushed as 39b3a7a.

The new automatic LocalSystem host owns the existing document renderer, scheduler
and qualified HID/NVAPI/PawnIO output. Program Files holds trusted runtime and
three manifest-hashed module DLLs; ProgramData/LightingStudio/playback.json holds
an atomically validated project/playback state. This is a combined privileged
runtime, not the smaller separately privileged PawnIO broker recommended in the
research. Qt document/image parsing and trusted native modules remain in its
privileged attack surface; the limitation is explicit in WINDOWS_SERVICE.md.

Only the installation-owner SID may issue bounded named-pipe commands. Network
logons are rejected before JSON parsing. Clients verify SCM PID and executable
path. No user-supplied module/file/register/shell command exists. Portable copies
of this revision detect the installed service and fail closed instead of racing
its hardware connections. Older binaries do not provide this safeguard.

Editor reopen takes the service snapshot over stale remembered files, without
rewriting user projects; explicit recovery/new edits while connecting win. Initial
discovery follows snapshot restoration. Edits coalesce, failed uploads retry, close
flushes final edits/commands, and failures are visible. Per-channel pause, global
stop, clocks and projects persist. Clock wraps after one day for continuous output.
Suspend releases sessions; resume/retries reverify. Conflicts stop/block output
without terminating other software. No-project startup makes no color writes.

Installer elevates once, protects ownership/ACLs, rejects pre-existing untrusted
writes/owners and reparse entries, creates service plus Limited logon task and a
Start-menu shortcut. It reuses PawnIO or obtains the pinned official 2.2.0 installer.
Static inspection of that exact binary established `-install -silent`, NOT `/S`,
and requireAdministrator manifest. SHA256 matches upstream asset metadata:
1f519a22e47187f70a1379a48ca604981c4fcf694f4e65b734aaa74a9fba3032.
No installer, driver, device ACL, task or service registration was changed live.
The packaged installer passed -WhatIf; this does not establish a real UAC count.

Final production-source gates: Release CTest 19/19 (12.71 s), Debug CTest 19/19
(44.79 s). Native Windows service/UI suite 9/9, zero skipped (see
build/service-native-final.log). After separating native/offscreen screenshot file
names, the focused Debug service/installer gates also pass (4.96 s). New tests cover
atomic invalid-project rejection, protocol bounds, playback/pause persistence,
clock wrap, fake hardware writes, faults/reconnect, suspend/resume, conflicts,
last-edit flush, stale-file protection, owner/host authentication, oversized input,
large project round trips and child-process restart. Installer parsing, expected
silent flags, input rejection and WhatIf run in both configurations; a bounded
Release-shaped fixture is used in Debug because Debug Qt DLL names differ.
Initial failures (temporary-file replacement, a no-op playback test toggle,
Debug installer fixture names) were corrected, not bypassed.

Native screenshot service-artifacts/service-hardware-windows.png was visually
reviewed; offscreen screenshots remain separate. One earlier offscreen font-only
render lacked native glyphs and is not native evidence. Package smoke tests remove
Qt SDK environment: --inspect exits 0, and packaged simulation host responds via
an owner-authenticated pipe. Simulation never scans or opens physical hardware.

Portable ZIP: build/dist/LightingStudio-Service-windows-x64.zip
SHA256: 0fedf3a1599cebf4221c0c15962cafecc8648f58cca9417231319e10ceb3d6f6
Editor EXE SHA256: 35fa6a7d84fd9848b2e28fda330025e2e82a30fd4d3aa1fa03874175313c4ec8
Service EXE SHA256: 69ec7c00b0ffdbd5c100d0f85aed62aaab301a39d09460016bfaedcc86260b1f
All archive CRC/extracted bytes and binary equality to tested Release verified.
Start-Lighting.cmd prefers installed copy then build/dist/LightingStudio-Service.
User default.lighting.json is unchanged, untracked and excluded; its SHA256 is
9cc3be379525f95e26d75b9bc27977cac9b3bdb3a6c126ce55fe836473a812a5.

Graphify refreshed AST with known 11 partial-parser warnings; semantic label
community mappings remain approximate, not compiler evidence. No paid labeling
was run. Global Git ignore permission warnings remain sandbox-only, with no
configuration changes. No live RGB process was terminated; only owned simulation
children were stopped. Remaining qualification: actual SCM install/UAC count,
SYSTEM driver access and mutex ACLs, pre-login NVAPI, cold boot, real standby,
physical colors and large personal-project throughput. Do not claim those from
software tests. Shared PawnIO/driver updates can require future UAC.

## Unified Editor UI audit implementation (2026-10-27)

Owner authorized implementing the design audit; Windows service work is explicitly
next, not included. Owner subsequently authorized committing and pushing this tested
implementation and the retained termination/eyedropper fixes.
See docs/design/UNIFIED_EDITOR_IMPLEMENTATION_2026-10-27.md for the current contract.
This entry supersedes earlier separate Layout/Effects navigation and G-fill behavior.

Header: approved compact branding first, then Editor / Hardware. Components and
Layers select the editing target within Editor. Composite / Effect only / Components
only control visibility independently. Target changes preserve camera screen position,
selection, channel preview and animation time. Tool gutters/options keep stable space;
only relevant tools show. Split hardware preview is optional. View menus preserve
access to canvas views, alignment and snap when toolbar controls overflow.

G creates new objects, including two gradients in one layer. Existing endpoint grips
edit with G or V. Source offers Solid/Gradient shape fill. Gradient texture drift no
longer translates geometry/masks; grip edits remove animated rotation/zoom. Gestures
capture preview playback once and restore it after commit/cancel without resetting
time. Color popup clamps after resize/show/sampling; invalid hex input is visible.
Inspector QLayout::SetMinAndMaxSize prevents field compression at 200% DPI, confirmed
against Qt 6.8 QScrollArea documentation and native screenshots/field bounds.

Shift/marquee selection, synchronized trees, overlap menu, covered-selection dragging,
multi-object transforms, alignment/distribution and smart guides are implemented.
Align treats selected layers/chains as groups and preserves internal spacing. Alt
bypasses grid/guides. L reveals labels; Line uses I. View locks persist in schema 14;
schemas 1..13 remain readable, malformed locks reject atomically, older apps reject
new saves. Component coordinate display preserves exact underlying values.

Final source gates: Release CTest 17/17 (13.46 s), Debug CTest 17/17 with -j2
(47.23 s), native Windows editor 126/126 (14.94 s). Final targeted native workflows,
selection/locks, gradient motion, overlap/guides, picker geometry and compact UI:
9/9 each at 125%, 150%, 200%. Logs: build/next/unified-native-final.log and
unified-dpi{125,150,200}.log. Tests cover field non-overlap/minimum size and global
camera anchors. Earlier red intermediate checks were corrected; none were bypassed.
The libpng Read Error in the full suite belongs to the deliberate corrupt-image test.
Exact desktop-color assertion remains opt-in via STUDIO_TEST_DESKTOP_CAPTURE=1;
this turn checked geometry, accept/cancel and two-screen overlays, not exact desktop
pixel color. No physical RGB writes or real process termination occurred.

Portable: build/dist/LightingStudio-Unified/msi_mystic_light.exe.
Start-Lighting.cmd prefers this build and retains older fallbacks. No live app was
started or stopped. ZIP: build/dist/LightingStudio-Unified-windows-x64.zip.
ZIP SHA256: eeb86d0bb12d44226247843e827136a8a3300adcd3ca49cc3cf59c778a86bdb6
EXE SHA256: b6cb6cc95d21ed602d5a242c1844d838d37d203add10b3bd139092c8c535a50f
CRC, safe extraction paths, personal-project exclusion and all extracted bytes
verified; EXE equals tested Release. Explicit subprocess --inspect returned 0
without Qt SDK PATH/plugin environment. Personal default.lighting.json SHA256 stays
9cc3be379525f95e26d75b9bc27977cac9b3bdb3a6c126ce55fe836473a812a5.
Native screenshots visually checked; docs/images updated, including unified editor
and components. Graphify AST refresh completed with its existing 11 partial-parser
limitations and 9 unsupported file types; semantic labels were not regenerated.
The graph is supplemental to the successful compiler/test gates. Git diff --check
passed; git status reported a sandbox permission warning for the global ignore file
and still listed the repository changes. No global Git settings were changed. Physical hardware behavior and real-project performance are not
established by these software tests. Existing termination/eyedropper hotfixes remain.

## Termination verification and per-screen eyedropper hotfix (2026-10-27)

Owner reported Terminate all requiring a second click and screen eyedropper making
the right side of the application appear far outside the display. Changes are
local; this follow-up did not authorize another commit/push.

ConflictDialog previously rescanned immediately after asynchronous TerminateProcess.
It now polls original PID/creation-time/path identities every 100 ms for up to two
additional seconds, after the existing graceful-close/force stage. It does not
force-kill again or broaden targets to restarted replacements. Cancel still stops
pending stages; timeouts and failed scans remain visible. Tests inject asynchronous
exits, permanent pending exits and verification-scan failures, without terminating
any real RGB process. SystemConflictProcesses identity/service guards are unchanged.

Screen sampling no longer creates one virtual-desktop-sized tool window. It captures
all screens before showing independent fixed-size overlays on their own screens.
Local pointer coordinates map to each capture's pixel dimensions. Acceptance or
cancellation closes every overlay once and restores the picker. Main-window and
Inspector geometry are asserted unchanged. Capture failures name unavailable screens.
Qt reference: https://doc.qt.io/qt-6.8/highdpi.html and qscreen.html.

Final gates: Release CTest 17/17 (12.67 s), Debug CTest 17/17 with -j2 (38.17 s),
standard native Windows editor 122/122 (12.70 s). Explicit interactive-desktop
screenEyedropperGeometry/liveColorPicker runs passed 4/4 at both 125% and 200%,
with two captured monitors and exact #123456 test-field sampling. Enable
STUDIO_TEST_DESKTOP_CAPTURE=1 for that exact desktop-color assertion; restricted
harness desktops can return black screen pixels. Without that opt-in, geometry,
selection, sibling closure and cancellation still run; the log states the narrower
coverage. Offscreen capture-unavailable behavior is asserted separately.

Additional diagnostic results are not hidden: an interactive-desktop full-suite
run had four failures in unchanged edgeScrolling/workspaceNavigation cases. Those
cases pass in the standard Windows harness; no Canvas code was changed here.
A parallel Debug output_ui automaticOutput fixture also observed 3 writes instead
of 2 once; it passed standalone and in the final -j2 suite without source changes.
These environment/timing findings remain outside this narrowly scoped hotfix.

Portable: build/dist/LightingStudio-Hotfix/msi_mystic_light.exe.
Start-Lighting.cmd prefers Hotfix, retaining Editor/Audit/older fallbacks. The old
LightingStudio-Editor process was running and was neither stopped nor overwritten;
close it manually before starting the hotfix to avoid competing hardware writers.
ZIP: build/dist/LightingStudio-Hotfix-windows-x64.zip
ZIP SHA256: 019316664631182420deb41c6f8012c834f0f2a53ca8334b878ed0ed8d643509
EXE SHA256: ef89e580846c637b6b9932c675029629974b477d122a6424ec951c5503f2eccd
ZIP CRC, safe paths, personal-project exclusion and all extracted files checked;
EXE equals tested Release. --inspect returned 0 without SDK Qt paths.
default.lighting.json retains SHA256
9cc3be379525f95e26d75b9bc27977cac9b3bdb3a6c126ce55fe836473a812a5.
No hardware output or actual RGB process termination was performed by the tests.

## Editor interaction and channel preview follow-up (2026-10-27)

Owner authorized removing Focus devices, implementing the agreed editor/status
changes, then committing and pushing. This entry supersedes earlier Hardware-banner,
Add-layer-menu and focused-preview decisions below.

New layer creates an empty transparent container; its tree contains independently
editable source objects. Shape/gradient clicks select; drags use the platform
threshold. New drawn objects commit only on release, so Escape leaves no empty
object or undo entry. Filled rectangles/ellipses in containers are editable shapes.
Legacy raster painting remains raster painting. Shape gradients preserve geometry.
Layer routing/mask/opacity/blend apply to composed objects. Schema 13 preserves
legacy IDs/values and rejects malformed, orphaned or nested ownership atomically.
Legacy layer rows retain their original implicit source and may also own objects.
See docs/design/EDITOR_INTERACTION_IMPLEMENTATION_2026-10-27.md for exact limits.

A gesture previously reset the channel selector to All outputs, exposing unrelated
layers over the inspected composition. The selector now stays stable during edits
and appears only in Effects. Per-frame caches share equal channel compositions
across preview/output, and preview sampling visits actual channel placements.
Regression proof covers stacking, sampled LED values, cache identity/invalidation,
root opacity, click selection, cancellation, gradient fills and save/load.

Focus devices is removed. Zoom/Fit have icons. Compact transparent LS branding
belongs to the app toolbar; Hardware has no banner, About retains the original.
The screen eyedropper directly captures a temporary desktop sampling overlay;
click accepts, Escape/right-click cancels, capture failure is visible.
Output summaries count active/paused/failed/connecting/stopped channels separately;
clicking opens Hardware, where selected-device details name states and errors.

Validation on final source: Release CTest 17/17 (12.16 s), Debug CTest 17/17
(40.44 s), native Windows editor 121/121 (14.28 s). Earlier failed passes exposed
selection/gradient-handle regressions and obsolete synthetic schema fixtures;
these were corrected before final gates. Local shared-channel 30-frame benchmark
was 161 ms and warm Layout 30 frames 54 ms; neither is a universal FPS claim.
Five native documentation screenshots refreshed and inspected. Actual personal
project loaded read-only for additional 125% and 200% DPI captures in
build/research/editor-interaction; branding, compact layout, Hardware and picker
inspected. No physical output or screen-reader qualification was performed.
Graphify updated with 11 existing partial-parser limitations and nine unsupported
file types; its graph is supplementary, not a complete AST/security validation.

Portable app: build/dist/LightingStudio-Editor/msi_mystic_light.exe.
Start-Lighting.cmd prefers it and retains older fallback paths.
ZIP: build/dist/LightingStudio-Editor-windows-x64.zip
ZIP SHA256: c265cb26ffe0d0e0b8c0eaa3c74816d885e57f515aae505e5232b443dcc931c4
EXE SHA256: 9d9257f39852b7b7004487ecff722b0b8ff00c2f11fe906ad910dcd1ac988884
ZIP CRC, safe paths, project exclusion and every extracted file were checked.
Extracted EXE matches tested Release; --inspect returned 0 with Qt SDK paths removed.
Previous distributions retained; personal default.lighting.json remains untracked,
unchanged and excluded. No real-device write was issued by this work.

## GUI audit implementation and selected branding (2026-10-26)

User authorized implementation of docs/design/GUI_AUDIT_2026-10-26.md and chose
the final ice/cyan LS banner. The image is copied unchanged to assets/branding,
embedded in Qt resources and shown in Hardware and About. Previous vector studies
are superseded. Graphite/cyan styling replaces competing navy/violet states.

ParameterSpinBox separates whole-number display units from exact stored values.
Rates use cycles/min, trails and channel offsets use ms, channel speed uses percent,
crop uses source pixels. Legacy off-grid values show an approximation mark with
exact hover inspection; selection, focus and refresh never round the document.
Slider steps respect units and slow rate ranges. Source includes Seed/Randomize.
The offline Hardware card includes the selected saved header and configured counts.
Reset/default and Zero are available from number context menus; Alt+R resets.
Native speed/brightness preserve firmware level indices, with neutral labels.

Add layer groups source creation and shares actions with toolbar flyouts. Source
owns particle settings and colors; toolbar colors are drawing state. Motion groups
intensity, transforms and color animation. Pulse/Breathe/Combined preserves legacy
combinations. Bulk motion is an explicit undoable copy, not a persistent scope.
Opacity is displayed directly. Palette snap percent is independent of pixel grid.
Panels recovery is pinned in the menu bar, including minimum-width/offscreen views.

The shared nonmodal HSV picker previews live, with Hex, optional RGB, original/
project/recent colors and Qt screen eyedropper. Color previews use a transient
document snapshot; Apply adds one undo command, Cancel restores state and retains
the redo branch. Autosave records the committed pre-preview state. A regression
test covers all three. Eyedropper uses a sibling asynchronous dialog: a test found
and fixed popup lifetime corruption with the original nested stack dialog.
Optional 140 ms mouse popover/chevron feedback obeys Reduce UI motion; lighting
playback and direct manipulation stay independent and immediate.

Validation: final Release and Debug builds; CTest Release 17/17 and Debug
17/17 (both -j6); native Windows editor 119/119. A queued discovery refresh
could add a legitimate extra output frame under slow scheduling. The output_ui
fixture now drains discovery before counting explicit submissions; exact report
assertions remain unchanged. The Debug case passed 5/5 repeated runs. Editor
time budgets are now 60 s Release / 120 s Debug for the 119-case suite; final
full runs took 15.15 s / 46.25 s. Mouse disclosure motion and reduced motion are
explicitly tested. Native 30 warm Layout frames: 55 ms; particle/trail mean
23.375 ms. These local measurements are not a universal FPS claim.
Read-only real-project captures reviewed at DPR 1.25, 1.5 and 2 under
build/research/audit-implementation. At DPR 2 Windows constrained window height
to 708 logical px; Inspector remains scrollable and picker fits the screen.
Five native documentation screenshots regenerated and inspected. Graphify updated;
existing parser limitations remain. git diff --check clean.

Portable app: build/dist/LightingStudio-Audit/msi_mystic_light.exe. Root
Start-Lighting.cmd now prefers this folder, falling back to the previous build.
Package: build/dist/LightingStudio-Audit-windows-x64.zip
SHA256: f62ed338a999d52811a2656188bafa5b7bc0bda2087c6495775843477f120c42
ZIP CRC/path/project-exclusion checks passed. Extracted EXE matches tested Release;
--inspect passed with SDK Qt removed from PATH. Previous distributions retained.
No physical device writes, personal project saves, commit, push or publication.
Screen-reader behavior and physical native-speed ordering remain unverified.

## Fixed optional component captions (2026-10-26)

User requested smaller labels, fixed positions, an eye toggle and momentary reveal.
Component captions now use 75% of the canvas font (minimum 7 pt), fixed below the
body at its lower-left anchor. Long names stay elided even when selected. Removed
collision relocation and leader lines. Full names remain in tooltips/Inspector.
Controller group headers remain visible and retain their drag targets.

Layout toolbar/View share the Show labels eye action. Visibility defaults off and
persists in editor/showComponentLabels. Holding H reveals labels temporarily,
including fullscreen; release, window deactivation, workspace changes, hiding or
entering a text/numeric field clear the override. Moving focus within Layout
preserves it. L remains the existing line tool. Reserved navigation bounds prevent
camera jumps; hidden captions cannot be clicked. No document mutation or hardware
write is involved. The application event filter preserves existing Delete scope.

Validation: Release and Debug builds; CTest 17/17 for each; native Windows editor
116/116. Coverage includes fixed anchors, hidden hits, stable navigation, saved
visibility, auto-repeat, text entry, focus transfer, tab/window/hide reset and
fullscreen. Native documentation screenshots regenerated and reviewed; real saved
project screenshots under build/research/label-controls were captured read-only.
Graphify updated (existing parser limitations remain). git diff --check clean.

Portable archive: build/dist/MSI-Mystic-Light-GamingUI-Labels-windows-x64.zip
SHA256: 7b4bfb9253ac7fa20179d048b30bb60681a474b60924131e8441e7da3e622184
Extracted app: build/dist/LightingStudio-GamingUI-Labels/msi_mystic_light.exe
Archive CRC/path checks passed; extracted EXE matches tested Release; --inspect
passed with SDK Qt removed from PATH. Previous GamingUI app/package and root
launcher retained. No commit, push, device writes or personal project changes.

## Gaming lighting workspace implementation (2026-10-26)

Implemented all seven findings from the user's UI review in the existing Qt
Widgets app. Shared styling uses navy surfaces, cyan interactions, violet
workspace selection and higher-contrast control boundaries. No new dependencies,
web fonts or decorative animation. Native system fonts and existing SVGs remain.

Navigation/history and preview controls now use separate toolbar rows. Navigator
and Inspector are closable and accessible through Panels/View and Ctrl+Shift+L/P.
Entering <1180 logical pixels collapses Inspector once; explicit choices survive
tab switches and main-window hiding. Wide mode restores intended state; Reset
workspace restores both panels. Hardware inventory gives names more width and
shows an actionable offline state. Preview and Devices statuses remain distinct
and visible, including live device state in Layout fullscreen.

Unselected model names abbreviate, selected/hover names remain complete. Shared
label rectangles avoid housing/label collisions, contribute to navigation and
hit-testing, and use subtle leaders when displaced. Reserved protocol slots do
not enlarge component bounds/focused preview. Live Hardware gets more initial height
and a Focus devices camera toggle; canonical sampling, saved geometry, channel
clocks and output reports remain unchanged. Empty previews hide their whole pane.

Source/Motion/Mask tabs plus an Editing badge identify context. Redundant effect
subtitles are suppressed. Move up/down and Ctrl+PageUp/PageDown reorder layers
with one undo step and disabled boundary actions. Transport behavior is unchanged.

Validation: Release and Debug CTest each 17/17; native Windows editor 115/115.
New cases exercise compact/panel state, output/playback separation, fullscreen
status updates, rendered control contrast >=3:1, layer order/undo and context,
collision-free label clicking/tooltips, focused preview coordinates/colors and
reserved slots. Refreshed screenshot cases also pass after queued layouts settle.
Native documentation screenshots were visually reviewed and promoted using the
existing guarded script. Real 17-placement project was loaded read-only by a
no-device review helper; screenshots and helper source: build/research/gaming-ui/.
A 30-frame native Layout render/grab check measured 495 ms; this is CPU rendering
evidence, not display FPS or physical output proof.

Separate portable: build/dist/LightingStudio-GamingUI/msi_mystic_light.exe.
ZIP: build/dist/MSI-Mystic-Light-GamingUI-windows-x64.zip. CRC, private-project
exclusion, tested EXE hash identity and portable --inspect without Qt SDK paths
passed. Existing launcher/installation,
personal project and published release artifacts remain unchanged. No push,
publication, installed-app restart or physical hardware writes.

## Dense strip SVG visibility regression (2026-10-26)

The reported Strimer Plus failure was reproduced with its 27 addresses normalized
to a three-unit span: fixed six-world-unit LED circles overlap and obscure the
housing. The previous procedural strip path also bypassed SVG artwork. This
supersedes the earlier claim that the strip presentation fix was complete.

Ordinary strips now use an SVG housing and one embedded SVG LED module per real
position. Strimer/Strimmer products use a separate multi-wire cable SVG, remain
Strips in hardware selection, and preserve multi-row geometry on new imports.
Existing saved positions are never migrated. Shared Layout/Live Hardware marker
radii fit closest distinct physical LED spacing and housing dimensions; reserved
slots and duplicates do not shrink markers. A closest-pair sweep avoids quadratic
work at the 65,535-slot custom-definition limit. Keyboard cells and numbered LED
editor handles keep their existing contracts. LianLi display spelling normalizes
to Lian Li so imported names do not repeat the manufacturer.

Read-only audit rendered 225 installed, bundled and saved definitions across six
contact sheets, all visually inspected. Sources, TSV and contact sheets live in
build/research/artwork-audit/. Native category/Strimer screenshots live in
build/next/editor-artifacts/windows/. New regression covers SVG validity, visible
artwork after LED overlays, dense 27-LED imports, scaling, reserved/duplicate
points, 65,535 LEDs, cable row preservation and unchanged serialized geometry.

Validation: Release/Debug CTest each 17/17; native Windows editor 111/111.
The existing channel-color regression also caught subpixel outline blending;
small LED centers now retain the exact sampled color in the zoomed-out preview.
Separate portable: build/dist/LightingStudio-StripSvg/msi_mystic_light.exe.
ZIP: build/dist/MSI-Mystic-Light-StripSvg-windows-x64.zip. CRC/private-project
exclusion and EXE hash equality passed. Existing installed app/launcher and user
project remain unchanged. No app restart, push, publication or hardware writes.

## Layout presentation and playback fixes (2026-10-26)

Glacier One is presented as AiO even when legacy/imported metadata calls its lit
zone pump; the display suffix is corrected without rewriting saved definitions.
The shared painted/hit-tested rotation grip follows the component angle (first
member for groups). Dashed outlines exclude model labels; component/group labels
remain part of navigation bounds. Model descriptions sit below their outline.
Strip artwork/picker icons use actual LED geometry/count with package sizes bounded
by spacing. The fallback strip SVG now contains its complete rightmost segment.
No saved geometry, wire order, sampling, hardware reports or user projects changed.

Repeated model normalization/counting now runs once per document revision, with
undo/load invalidation. Static SVG housings use bounded resolution-specific Qt
pixmap caching. A read-only native benchmark of default.lighting.json (17
placements, 1600x900 widget, 30 animated render+grab frames) measured 2224 ms with
the previous name routine/vector rendering versus 551 ms after both caches.
This is a local CPU rendering comparison, not physical output or display FPS proof.
Diagnostic source/logs/screenshot: build/research/layout-performance/.

Validation: final Release/Debug CTest each 17/17; native Windows editor 110/110
plus the final strip/grip/artwork regressions 5/5. Screenshots visually inspected.
Updated build/dist/LightingStudio after confirming no app process was running;
installed EXE hash matches tested Release. Portable CRC, project exclusion and
installed --inspect passed. No app restart, push, publication or hardware writes.

# Implementation status

## Fixed devices cannot edit component chains (2026-10-25)

The Layout settings dialog still exposed append/remove/reorder despite the prior
standalone-device display change. A shared channelHasFixedComponent contract
now treats address.componentId as a built-in output. ChannelEditor hides and
disables chain controls, uses Device settings terminology, and preserves original
placements even if hidden widgets are modified. Assignment validation rejects
multiple components on such channels independently of LED capacity. Genuine
controller headers retain chain editing. Software output still requires one
component; native output may retain no geometry as before. Header settings button
renames itself to Device settings for fixed devices. No transport/output changes.

Validation: Release/Debug CTest each 17/17; Windows editor 109/109. Tests cover
keyboard, mouse, RAM, GPU and IOShield, hidden-control tampering, switching modes,
playback clock editing, the exact Layout -> settings modal path, ordinary header
append/remove/reorder, and rejecting two five-LED strips on a ten-slot keyboard.
The rejection is atomic and does not change undo history. Docs updated.

Portable ZIP rebuilt and inspected. After the user explicitly confirmed the app
was closed, verified no running process and updated build/dist/LightingStudio.
Installed EXE matches the tested Release build; extracted --inspect passed. The
temporary LightingStudio.next staging folder was removed after installation.
Published release ZIP and user project unchanged. No push or physical hardware
writes; app was not restarted automatically.


## Single model labels and fullsize keyboard parity (2026-10-25)

Every Layout component has one lower-left model label, including when unselected.
Dedicated peripherals have no duplicate chain caption and appear directly in the
device tree. Exact output assignments remain unchanged. Real controller chains
retain header captions/drag behavior. LightingDocument::componentName is shared
by canvas/list/properties; it adds instance numbers only for repeated normalized
models across placements, not electrical chain positions. AIMO casing normalized.
New placement rotation remains zero; Rotation deg label renamed Rotation.

Fullsize keyboards share one registered Vulcan-derived SVG, including the per-key
models. Per-key color markers use aligned key-cell dimensions; Apex keeps its ten
real zones. Regression compares the normalized rendered artwork pixel-for-pixel
(up to raster rounding), checks unique/repeated model numbering and undo, default
angles, and clicking a flattened dedicated device with preserved output mapping.
Scroll navigation regression now includes the full always-visible model label.

Explicit user authorization overrides the prior read-only project boundary for
the Basilisk correction only: default.lighting.json was backed up privately under
build/research/user-backups/default-before-basilisk-zero-20261025-200050.lighting.json,
then its -90-degree placement rotation was absorbed into its instance LED geometry.
Rotation is now zero with the wheel facing up. Measured world-coordinate error
was exactly zero. All other definitions, placements, mappings, layers and output
assignments were verified unchanged. No app process was running. This is a one-off
user-requested adjustment, not an automatic load migration. Never stage/package
the user project or its backup.

Release/Debug CTest each 17/17; Windows editor 103/103. Portable CRC and extracted
--inspect passed. Stable app path remains
build/dist/LightingStudio/msi_mystic_light.exe. No push or hardware writes.


## Aero visibility and build cleanup (2026-10-25)

The single-zone Aero now has a recognizable card-sized visual housing; its logo
LED/sample position is unchanged. A new regression caught and fixed a separate
one-point bounds issue: default QSizeF() is invalid (-1,-1), so explicitly use
QSizeF(0,0) when initializing an LED bounds rectangle. Single-zone housings now
remain centered on the actual LED. User project read-only diagnosis confirms
Vulcan placement rotation 0.8447652411840636 degrees: the whole keyboard follows
that saved angle. Use existing Rotation=0 for horizontal alignment; do not
silently round saved layout angles or alter the project on load.

Release/Debug CTest each 17/17; Windows editor 101/101. Current portable is now
build/dist/LightingStudio/msi_mystic_light.exe, also used by Start-Lighting.cmd.
Development ZIP: build/dist/MSI-Mystic-Light-current-windows-x64.zip. The original
v0.1.0 ZIP/hash are preserved unchanged. Portable CRC and --inspect passed.

Build cleanup reclaimed 751.4 MiB (1912.7 -> 1161.3 MiB): removed 42 obsolete
packages/build targets, archived and hash-verified 104 transient helper scripts,
moved root logs to build/logs, retained research/root artifacts and artwork
reviews under build/research, and removed generated review-browser profiles.
Kept current Release (next), Debug (multi), Qt plugin builds/source and their
verification commands. No running Lighting Studio process was found before
cleanup. All recursive removal paths were checked inside build; no user projects
or published artifacts removed. Records: build/logs/cleanup-{plan,result}.json.
The diagnostic scripts are preserved in build/research/legacy-build-helpers-*.zip;
historical CONTINUE entries below reference old paths deliberately. No push or
physical hardware test.


## Artwork geometry and frame regression fixes (2026-10-25)

Read the user project only to diagnose: an instance named D30 has 15 collinear
LEDs; the imported mouse has its quarter-turn already baked into coordinates;
Apex has ten zone points on one row and a large placement scale. No user project
was edited. Artwork now treats collinear fan instances as strips without changing
hardware identity (three distinct points required; small non-collinear offsets
remain fans). Mouse contour handles baked rotation. Per-key keyboards have no
second generic key grid; actual keys and housing share placement rotation. Apex
has full-height schematic key geometry plus separate zone markers.

Housing extents now participate in hit areas, resize handles, group outlines and
navigation. Dashed group padding is distinct from selection padding. Labels and
group frames paint after artwork; complete component labels sit below the body.
Repeated manufacturer prefixes are removed from display names. Nonempty canvases
hide the initial Add placeholder. No output, sampling, mapping or schema changes.

Validation: Release/Debug CTest each 17/17; Windows editor 100/100. New regression
reproduces imported mouse/Vulcan geometry, a linear D30, enlarged Apex, resize at
the housing corner, and unchanged sampling/serialization. Native screenshot
device-artwork-alignment.png inspected. Existing corner resize and protected-label
regressions pass. Local portable:
build/dist/LightingStudio-ArtworkAlignment/msi_mystic_light.exe.
No push, running-app restart or physical hardware test.


## Detailed technical hardware artwork (2026-10-25)

User approved the B-style rotor and authorized direct integration without another
preview. Added square fan housings plus original fine-line technical SVGs for RAM,
GPU, keyboard, mouse, strip, pump, AiO, motherboard and fallback. One cached Qt SVG
renderer path serves HardwarePicker, Layout/Live Hardware and ComponentEditor.
Layout artwork uses placement rotation/scale; nonphysical slots do not enlarge
housings. Draggable LED circles and centered wire indices are retained. No LED
coordinates, mapping, sampling or saved schemas changed. No user project edits.

Fan derivative attribution: Delapouite, CC BY 3.0, game-icons/icons revision
82d948812bfe3f269ef8f731dcdb07b08160edc4. Original, SHA256 and adaptation notes
in assets/hardware. Removed unused trial imports before integration. Other SVGs
are original GPL-2.0-or-later drawings. Help > Artwork credits and portable notices
include attribution. Qt SVG is dynamically linked; Qt6Svg.dll added to explicit
portable runtime and packaging regression. No online assets at runtime.

Validation: Release/Debug CTest each 17/17; native Windows editor 99/99. Focused regression
checks all ten rendered categories, square fan bounds excluding reserved slots,
unchanged LED coordinates/permutation/serialization and numbered editor circles.
Native rendered artwork and LED editor screenshot inspected locally. Portable
CRC, extracted --inspect, SVG runtime/license presence and private-project exclusion
passed. Published v0.1.0 archive hash unchanged.
Local package target: build/dist/LightingStudio-HardwareArt/msi_mystic_light.exe.
No push, running-app restart, output changes or hardware writes.


## Mask playback, stationary minimap and cosmetic variants (2026-10-25)

Mask tab now preserves playback and chooses Pencil; its composite uses live time.
Gestures still temporarily pause/resume. Minimap reserves both bar gutters to avoid
jumping when the second bar appears. Hardware names omit Black/White; picker merges
only equal normalized names plus identical geometry/mapping/metadata, keeping saved
IDs intact. Installed SignalRGB app-2.5.80 contributes 197 visible imported geometry
definitions after filtering/deduplication; these are not bundled hardware drivers.
Native tests cover brush write/undo/resumed clock, variant mapping differences, and
pixel-exact minimap anchor across one/two/one scrollbar transitions.
Release/Debug CTest each 17/17; native editor 98/98. Portable CRC and
extracted --inspect passed; published archive unchanged. Local app:
build/dist/LightingStudio-MaskMinimap/msi_mystic_light.exe. No push or device writes.


## Shared canvas extent follow-up (2026-10-25)

Hardware layout bounds now determine the aspect ratio and dimensions displayed
by Effects and Live Hardware. Effects retains its canonical raster for persisted
layers/masks; display, inverse pointer mapping, scrollbar extents and wheel
anchoring use the hardware world projection. Layout labels still reflect live
in-progress component drags. No component redesign, hardware writes or project
migration. Save As icon and collapsible preview follow-up are included.

Validation: Release and Debug CTest each 17/17; native Windows editor 96/96.
New regression checks expanded positive/negative extents, shrink-back, literal
pixel color agreement, pointer roundtrips and both overflow bars. Portable CRC
and extracted --inspect passed; published release ZIP unchanged.
Local app: build/dist/LightingStudio-SharedCanvas/msi_mystic_light.exe.
Not pushed. Component redesign still requires discussion.

## Scrollbar contrast and Effects preview divider (2026-10-25)

Styled both scrollbar orientations, handles (normal/hover/pressed), rails, page
regions, zero-sized line controls and scroll-area corners. This removes the old
vertical-only style/native horizontal mismatch and patterned page backgrounds.
Effects fullscreen is hidden across resize and has no workspace signal handler;
Layout fullscreen remains intact. A custom QSplitterHandle highlights the divider,
adds a right-side up/down arrow, vertical resize cursor, hover feedback and tooltip.
The handle is 18px. Follow-up: both panes can collapse on extreme drags and be
restored by dragging the visible divider. Save As now uses a distinct floppy/pencil
icon. Native regression covers both extremes, restoration and distinct icons.
Canvas extent clarification resolved: hardware layout bounds are authoritative
for both views. Effects display and pointer/scroll transforms now project its
canonical raster into that same aspect ratio. Component redesign still awaits
discussion; no component definitions or user projects changed.

Tests sample literal handle/track colors in both orientations (DPR-aware), drag
the arrow to change both pane sizes, check extreme limits, tool visibility,
fullscreen absence after resize and unchanged Layout fullscreen availability.
Release/Debug CTest each 17/17; native Windows editor 95/95. Package CRC and
extracted --inspect passed; published release archive hash unchanged. Evidence:
build/preview-divider-*.log and
build/next/preview-divider-native.log. Native screenshots visually inspected:
scrollbar-contrast.png and effects-preview-divider.png. No device writes, running
app restart or user-project changes. No push/publication authorized.
Local portable: build/dist/LightingStudio-PreviewDivider/msi_mystic_light.exe.

## Eraser playback and standard editing controls (2026-10-25)

User physically confirms the MSI ordering fix now addresses individual fan LEDs.
New issue: after erasing a procedural top layer, playback could not restart until
a new shape was created. Cause: Eraser switches to mask editing, where every
properties refresh disabled Canvas Start. Historical behavior (superseded below): mask entry paused once; Start stays
enabled. Every Effects gesture, including mask/no-op gestures, balances pause/end;
previously running playback resumes after release, initially stopped stays stopped.

Visibility uses painted eye/eye-off icons, retaining Qt check semantics and undo.
Space toggles the selected layer even with Source/Mask thumbnail current. Stroke
size reuses sliderField beside its numeric value, 1..320px, including the mask
brush. Initial brush radius now matches the displayed 6px diameter. Delete filters only
canvas/inventory/layer-list key events, preserving text/spin editors. Effects
removes the selected layer; Layout/Hardware remove components or the group with
assignment/effect-target cleanup in one undo step. Empty selections/autorepeat
are ignored. Fullscreen reuses the filtered canvas.

Regression cases cover Paint, Shape, Gradient and explicit masks, stopped/running
playback, actual drag/undo/redo/restart, eye click/Space, slider limits/sync,
Delete from both canvases/lists/Hardware/fullscreen, group cleanup, no selection
and text/numeric editing. No user project changes or physical writes by the agent.
Local portable: build/dist/LightingStudio-LayerControls/msi_mystic_light.exe.
Release/Debug CTest each 17/17; native Windows editor 93/93. Native eye/slider
screenshot visually checked. Validation logs: build/layer-controls-*.log and
build/next/layer-controls-native.log. No push or publication authorized.

## MSI spatial fan output report ordering (2026-10-25)

User confirms Layout fan LEDs show a spatial gradient, but physical ARGB fans
change together; keyboards/mouse work. Read-only current-project audit confirms
distinct RGB for individual fan addresses. Saved project was not modified.
Found a real ordering difference: restoring the IOShield Canvas 0x50 fallback in
1b3e779 caused whole-board configuration after JARGB 0x51 pixels in inventory
order; the original studio sorted IOShield before JARGB. OutputSession now stably
places MSI native/shared-zone control before per-address software channels.
Other modules, geometry, counts and mappings are unchanged. This removes the late
configuration write; actual firmware effects remain physically unconfirmed.

New output regression is red before the fix (0x51 where 0x50 must precede pixels),
then checks distinct six-address payloads on all three headers over repeated
shared-zone changes. Renderer-to-real-module test checks shifted spatial ramps
against literal RGB bytes. Scheduler tests retain exact latest-payload/cancellation
checks with configuration-first indices; Stop now prevents pixels after a blocked
configuration call. No physical writes, driver changes or running-app restart.
Local build: build/dist/LightingStudio-FanGradients/msi_mystic_light.exe.
Release/Debug CTest each 17/17; output-session cases 10/10. Package CRC and
extracted --inspect passed; published 0.1.0 archive hash unchanged. Evidence:
build/fan-order-*.log and build/fan-output-detail.log.
No push or publication authorized. Physical fan confirmation remains outstanding.

## Hardware identity and workspace navigation (2026-10-25)

Added optional manufacturer/category component metadata (validated, persisted and
preserved by LED edits), manufacturer filtering/Hardware inventory column, full
category filtering and branded presentation for legacy M25/Glacier entries.
Chain suffixes now follow current assignment order instead of embedded #1 names.
Reverse variants are hidden from new attachment; saved devices remain usable.
Imported strips are straight. D30 selection is circular, with only the exact old
narrow 15-address template migrated; custom edits and permutations are retained.
No address/count/wiring remap was performed. User observed independent LED gradients
within fans; that does not identify serial versus mirrored wiring across fans.
NV5 MKII built-in strip count remains unverified (18 is not a confirmed default).

Canvas has a protected 48px header and 16px gutters. Scrollbars and minimap only
appear on overflow; minimap stays 12px inside the usable lower-right corner.
Effects navigation stays fixed to the raster even for oversized animated sources.
Layout includes group labels in navigation bounds and preserves their painting
outside the raster while clipping them below the header. Fullscreen accounts for
Windows taskbar insets. Edge scrolling ramps over 48px, default 720 screen px/s;
setAutoScrollSpeed accepts 0..4000, rejects invalid/non-finite input. Opposite-corner
anchored uniform resize replaces symmetric scaling. Gestures remain single undo.

Save as and active filename are beside Open/Save. The new filename is adopted
only after successful atomic writing; cancellation/failure keeps the old target.
Preview now uses a precise 16ms timer. Hardware pacing remains 33ms plus driver
latency; smoother physical output has not been qualified or claimed.

Release/Debug CTest each 17/17; native Windows editor 85/85. Boundary tests cover
all four scroll directions, every resize corner for grow/shrink, exact/one-pixel
viewport overflow, negative world positions, invalid speeds/metadata, label/header
painting, save failure and original-file preservation. Native pixel tests account
for screen DPR without changing expected colors. Native fitted/fullscreen/overflow
screenshots inspected; no physical RGB writes or process termination performed.

Local portable: build/dist/LightingStudio-Workspace/msi_mystic_light.exe.
ZIP: build/dist/MSI-Mystic-Light-0.1.0-workspace-windows-x64.zip (+ SHA-256).
Package CRC/extracted --inspect passed; published 0.1.0 ZIP verified unchanged.
User default.lighting.json untouched. No push/tag/release authorized for this work.

## Snap-to-grid editing (2026-10-24, after published 0.1.0)

Added a shared, persistent UI-only grid preference, default enabled, spacing 1-100
Layout pixels. Toolbar toggle and LED-dialog controls share settings; Alt bypasses
snapping and Ctrl+Shift+G works across fullscreen. Layout drags snap one placement
anchor and translate the entire group rigidly. Effects move/resize/gradient/drawing
gestures, mask gradients, LED marker drags and palette stops participate. Freehand,
fill, camera controls, exact numeric input and non-spatial list ordering stay free.
Rotation snaps to 15-degree increments. World spacing is independent of zoom.

LED scene units are converted using 24 presentation units per component unit and
20 Layout pixels per component unit; selected LEDs keep offsets and wire mapping.
Palette spacing is percentage-based. Grid painting is viewport-bounded, thinned
at small zoom and capped for huge imported coordinates. No project schema changes.
The implementation reuses Pixaroma's common-anchor/common-delta principle without
importing its runtime or adding dependencies. Tests isolate preferences in temporary
INI stores so they never modify real application settings. Existing free-placement
checks remain; explicit grid tests use fixed expected positive/negative positions,
different zoom levels, invalid spacing, overflow, undo, Alt and Shift, resize,
gradient endpoints, LED mapping, palette positions and synchronized UI controls.
Output tests assert that the editing grid never enters the hardware color frame.

PawnIO follow-up: installed runtime accepts ABI major 2; no online updater exists.
The bundled module is pinned to 0.2.11 with size/hash verification. Upstream latest
module release still resolves to 0.2.11. Maintenance procedure documented in
third_party/pawnio/README.md; no driver installation or ACL changes performed.
Do not overwrite published 0.1.0 ZIP/tag with this development build.

Validation: Release/Debug CTest each 17/17; native Windows editor 69/69 and output
UI 14/14. Native grid screenshots visually inspected. Debug exposed an existing
output-test race: wait for the exact expected blue/red packet rather than any
write count; literal packet comparisons retained. An off-grid LED click is tested
to leave coordinates unchanged. Package CRC and extracted --inspect pass.
Local build: build/dist/LightingStudio-SnapGrid/msi_mystic_light.exe.
EXE SHA256: 1c65fb7584b8629f160fd5d8db1d7637a5b55554eb64527d84b9d253a1eac907.
ZIP: build/dist/MSI-Mystic-Light-0.1.0-snap-grid-windows-x64.zip (+ SHA-256).
Published 0.1.0 ZIP hash verified unchanged. Running instance and user project
were not modified. No physical hardware writes or new publication performed.

## Release 0.1.0 preparation (2026-10-24)

Release and Debug CTest pass 17/17, including Python artifact checks. Native Windows
editor tests previously passed 58/58. Added a tracked portable packaging script,
release notes and corrected outdated UI/RAM documentation. Package inputs explicitly
exclude user projects and stale catalogs; missing runtime, empty catalog and missing
PawnIO source cases are covered. The extracted ZIP passes --inspect with all three
modules, ten profiles and nine component definitions. No physical writes were made.

Portable: build/dist/LightingStudio/msi_mystic_light.exe
ZIP: build/dist/MSI-Mystic-Light-0.1.0-windows-x64.zip (adjacent SHA-256 file).
EXE SHA256: d3e5484478dabfbf586453b817578f8245d8bc5016901abfbbaae8120975eb12.
Unsigned; RAM still needs separately installed PawnIO and permitted elevated access.
The user-owned default.lighting.json remains untouched and excluded from Git/package.
Public push, tag and GitHub Release are not part of this local preparation.

## Native documentation screenshots and permission verification (2026-10-24)

Replaced five documentation screenshots with visually inspected native Windows
captures, fixing missing-font boxes. Editor artifacts now use platform-specific
directories and rendering metadata. The promotion script rejects offscreen,
missing-glyph and incomplete sets; three Python tests and all 58 native editor
tests pass. These changes have not been committed or pushed.

The packaged EXE manifest was extracted and confirms `asInvoker`. RAM access uses
the installed PawnIO kernel driver, unlike the HID-only public baseline. The
non-elevated probe logged access denied (0x80070005); signing the app does not grant
driver access. No manifest, driver installation, service or ACL changes were made.

## Independent test review and scrollbar endpoint fix (2026-10-24)

User requested boundary/invalid-value tests independent of implementation and
reports LightingStudio-Scrollbars navigation is not working. Review found a
self-derived layout-color oracle and replaced it with literal world/pixel/color
fixtures. Native mouse-drag, exact-fit/one-pixel/axis and camera-endpoint cases added.
Red tests exposed invalid zoom, overflowing transforms/non-finite draft positions
and a thumb-at-limit/camera-out-of-range mismatch. Production guards and idle
camera clamping fixed these without relaxing the tests. RAM literal vectors and
all-transfer failure injection added; two deliberate encoder mutations detected.
Details and evidence: docs/TEST_REVIEW_2026-10-24.md.
Final Release/Debug each 16/16; native Windows editor 58/58, output UI 14/14.
Debug also exposed/fixed a dangling temporary in a test; expectations retained.
Updated portable: build/dist/LightingStudio; ZIP CRC/content verified.
EXE SHA256: d3e5484478dabfbf586453b817578f8245d8bc5016901abfbbaae8120975eb12.
No hardware writes, external signing submission or Git publication performed.


## Canvas scrollbars and right-hand minimap (2026-10-24)

Release CTest 16/16 passed; native Windows scrollbar/fullscreen tests passed.
Updated portable: build/dist/LightingStudio-Scrollbars; ZIP rebuilt/CRC/content verified.
EXE SHA256: 87db32940830da9808ed8414252a2572c38abec9d0009e66ba279b6fa5818f9a.
No physical hardware writes performed; user default.lighting.json untouched.


Added as-needed horizontal/vertical Qt scrollbars bound to canvas world/pan state.
Bounds include draft placements, zoom and negative coordinates. Integer scrollbar
ranges are capped while retaining double world coordinates for large layouts.
Minimap moved to bottom-right, inset around visible bars; scrollbar/minimap/middle
mouse navigation share one pan state. Preview-only hardware canvas hides bars.


## Canvas Start/Stop, fullscreen and growing Layout (2026-10-24)

Release CTest 16/16 passed. Native editor/output UI: 39/39 and 14/14;
final targeted fullscreen/Escape/triangle checks passed after the icon/focus fix.
Native screenshots inspected; no physical RGB writes by the agent.
Latest portable: build/dist/LightingStudio (ZIP rebuilt/CRC/content verified).
EXE SHA256: 41f706265a1bdebdd1730b713cd000815f35e029f5bebbdc15807e3865584788.
Changes are uncommitted; user default.lighting.json was not edited.


User clarified auto-expansion ONLY for Layout. Single green/red Canvas Start/Stop
button replaces Play/Pause plus toolbar output status; no numeric timer.
Fullscreen reuses the same canvas through a wrapper and restores it on Escape.
Minimap supports click/drag navigation. Layout bounds derive from physical LEDs
including draft transforms and negative positions. Composite effect raster stays
640x400 and maps across these bounds consistently in preview and hardware output.
Expanding/shrinking bounds changes peer samples too; no new project schema.
Previous Other effects removal and shape interaction fixes are included.


## Shape interaction and toolbar cleanup (2026-10-24)

Removed numeric preview time from toolbar; its internal clock remains. Shape and
particle creation exits crop mode. Move hit-testing/resize use the visible animated
transform and preserve preview time; rotation pivots around the visible center.
Regression covers triangle move/resize after crop, undo and animated hit-testing.
Other effects button/gallery removal is included in this update.


## Remove Other effects (2026-10-24)

Removed the Other effects button and its unused gallery function/declaration.
Shapes/Particles toolbar menus and direct drawing/gradient tools remain available.
Existing procedural layers retain their renderer and project compatibility.
Tests now create non-toolbar fixtures through the document API and assert that
the removed button is absent.


## LED marker readability (2026-10-24)

Release 16/16 passed; native Windows keyboard/fan presentation test passed.
Keyboard screenshot inspected, no marker overlap at default size.
Latest portable: build/dist/LightingStudio-ReadableLEDs; ZIP verified.
EXE SHA256: 7b8d509ba07bddc4a9695cd31c94cbc0b8a15bde4765201825a235299bfa958c.


Markers increased from 20 to 32 logical screen pixels; labels from 8 to 12 pixels
semibold. White text on dark blue replaces low-contrast white on light blue.
Default dialog is 1180x760 with smaller scene padding so a full keyboard retains
separated circles at opening. Stored geometry and RAM transport are unchanged.


## Fan LED marker correction / RAM confirmation (2026-10-24)

Release 16/16 passed; native Windows keyboard/fan presentation test passed.
Both screenshots inspected. Latest portable: build/dist/LightingStudio-LedFix.
ZIP rebuilt and verified; includes the user-confirmed working RAM backend.
EXE SHA256: 2343ffb9f02f4c3d6f3de03b0167257aab5a519e3274d082498675b7a9e4c2eb.


User confirms RAM Canvas output works with LightingStudio-RamDiagnostics.
This is user-observed physical output, not an agent hardware test.
Fan screenshot exposed the shared minimum-axis-spacing calculation: nearly equal
axis values magnified scene extent and shrank markers to subpixels. Removed that
calculation. Qt ItemIgnoresTransformations keeps LED circles/labels at a readable
constant screen size; fit/zoom still positions the LEDs without changing geometry.
Keyboard/fan presentation test checks viewport circles and preserves coordinates.


## RAM still ignores Canvas: diagnostic readback (2026-10-24)

Release CTest 16/16 passed; readback tests cover mode rejection, color mismatch,
read errors and all eight distinct RGB values. Portable + ZIP verified.
Latest executable: build/dist/LightingStudio-RamDiagnostics/msi_mystic_light.exe
SHA256: ed9ea389ad56567bdb13314ec20382d1c04b799c1ca3d82d848b2ae869f3443e.


User confirms RAM status Running but native color cycling continues after Apply fix.
Root cause is NOT yet established. Added per-address identity and first-frame logs,
first-frame direct-register/color-buffer readback, exact register mismatch errors
and transport error propagation to OutputSession. Readback does not prove light
output. No agent hardware writes were performed. Await user logs from this build.


## RAM Canvas activation fix (2026-10-24)

Release CTest 16/16 passed, including mode/apply ordering and transfer failures.
Updated portable: build/dist/LightingStudio-CanvasFix; ZIP rebuilt/verified.
Includes earlier RAM geometry and keyboard presentation fixes.
EXE SHA256: 366808f4bf981e63d93f873fa52c706423fc3286c3beaf9c4319fae558f443cc.


User reports RAM ignores Canvas. Reference ENESMBusController::SetDirect writes
DIRECT then APPLY; our implementation omitted APPLY. pushColors now writes
0x8020=1 and 0x80A0=1 before live colors. This is volatile Apply, never flash save
(0xAA). Tests verify ordering and aborts at mode/apply/color transfer boundaries.
Physical RAM output remains to be confirmed by the user.


## Keyboard LED editor correction (2026-10-24)

Release CTest 16/16 passed; native Windows keyboard presentation test passed.
Rendered screenshot checked: 105 separate circles, centered contained labels.
Latest portable with BOTH RAM and keyboard fixes: build/dist/LightingStudio-Update.
ZIP refreshed and CRC/content verified. Original portable remains running/locked.
EXE SHA256: 0f4a5617fc4c77e110b229d751f1fe60832f3376750b1c8826c3d9f53bdb840c.


Shared LED view uses adaptive display spacing with centered numbers inside circles.
Explicit optional nonPhysicalSlots preserves all wire slots while hiding reserved
slots from the visual editor/Layout and excluding them from shape arrangement.
Vulcan has 105 visible keys/144 report slots; wire 0 (Esc) is retained.
Legacy embedded Vulcan definitions, including instance copies, acquire the explicit
reserved slot list during parsing. Coordinates/output ordering remain unchanged.


## RAM layout unit correction (2026-10-24)

Release CTest: 16/16 passed, including legacy repair/custom geometry preservation.
Updated portable: build/dist/LightingStudio-RamFix (original app is running and
locks its executable); ZIP refreshed from the corrected build.
Current corrected EXE SHA256: 85132738c07752bd8c97afc88487c0d15a33fb7d0c050d064b900b290eaf4c17.


The initial ram.xpg definition incorrectly used pixel distances as component units.
Use 0.9 units per LED (18 canvas pixels, 126 total at 100% scale).
Document load repairs only the exact shipped eight-point 18-unit line with identity
mapping; custom arrangements, placement transforms and wire order remain intact.
The user-owned default.lighting.json is not rewritten. Reload a saved project in
the updated application to repair its embedded definition.


## Direct PawnIO RAM integration (2026-10-24)

The user now explicitly permits PawnIO for a lightweight direct integration.
This supersedes the earlier exclusion below; OpenRGB/helper processes remain
excluded. The host loads the installed official PawnIOLib 2.x and the pinned
signed SmbusPIIX4 module 0.2.11. ENE detection/output is restricted to primary
AMD 1022:790b bus, addresses 0x71/0x73, AUDA0-E6K5-0101, eight LEDs.
GUI discovery reads identity registers; --scan remains passive. Module creation
and report proof are memory-only. Worker open rechecks identity; complete ENE
operations hold the shared SMBus mutex and restore the previous bus port.
RAM profiles/components participate in existing Hardware/Layout/output paths.
No driver is installed or started automatically. User default.lighting.json
remains untouched. Physical RAM output has not been tested by the agent.
The explicit --probe-ram diagnostic was attempted without color writes. The
execution environment could not open PawnIO (HRESULT 0x80070005), so physical
RAM identity/output remains unverified. The installed official driver ACL grants
SYSTEM/administrator access; run Lighting Studio as administrator for RAM.
Hardware rows expose this access error rather than silently claiming detection.

Validation: final Release and Debug each pass 16/16 CTest. Native Windows editor
and output UI passed 36/36 and 14/14. Protocol tests cover both addresses, exact
firmware/LED-count rejection, R-B-G blocks, failed transfers and invalid payloads.
The Debug editor timeout is now 60 seconds to accommodate unoptimized particle
rendering; unplug/restart setup drains queued discovery before exact report-count
assertions. Graph updated (12 existing partial C++ parser warnings).
Portable folder and ZIP refreshed and verified for required runtime/profile/module
files, CRC and EXE byte equality. No OpenRGB/QtNetwork/PawnIOLib is bundled.
EXE SHA256: 8ef53262d373a8c99655bbd7a152e621edf9184ca24be862c5bce7b3948b552b.
Changes remain uncommitted/unpushed.


## Superseded RAM dependency decision (2026-10-24, before PawnIO approval)

Earlier user constraint at that stage: no PawnIO and no OpenRGB dependency.
The unpublished RAM SDK/helper experiment was removed: no launch code, TCP RAM
adapter, RAM profiles, helper resource or QtNetwork dependency remains. The old
RAM implementation used that helper; its removal does NOT mean RAM is restored.
Do not reintroduce it or an SDK dependency without discussing the access design.

Current uncommitted changes retain the flat Endpoint / Profile / State table,
IOShield shared-color 0x50 Canvas fallback, and the qualified RTX 5080 AERO NVAPI
backend. The user-owned default.lighting.json must remain untouched/untracked.
No replacement RAM backend is implemented. No package/commit/push of these changes
has been made. The installed portable folder remains the prior 091de5b build.

Official research: SignalRGB documents SMBus plugins and its Internals Driver:
https://docs.signalrgb.com/developer/plugins/determining-device-type/
https://docs.signalrgb.com/troubleshooting/software-conflicts/blocked-drivers/
MSI's public SDK guide (version 1.0.0.08, Jan 2022, still linked by MSI) explicitly
requires MSI Mystic Light related applications to be installed before SDK use:
https://storage-asset.msi.com/files/pdf/Mystic_Light_Software_Development_Kit.pdf
These documents do not identify the exact current proprietary kernel interfaces.
No driver installation, RGB process termination or real color write was performed.

The read-only NVAPI PCI scan detected the RTX 5080 AERO; the separate RAM helper
was not listening on port 6745. That helper must remain unused per user direction.
Final verification/packaging of the retained changes is still outstanding; do not
claim the ongoing parity task complete or current portable hardware support fixed.

## Live layout sampling and output inventory (2026-10-24)

Layout member-body clicks now select the individual placement even from group
selection. Group labels remain dedicated whole-chain drag targets. Transient move,
scale and rotation placements drive both LED drawing and real output preparation
without changing document state/history. Escape restores document sampling;
release clears the preview delta before committing one undo step, avoiding double
application during document notifications.

Hardware's expanding Controller / output tree lists all supported channels of
discovered controllers plus saved assignments, including unconfigured channels.
JARGB1/2/3, EZConnector, IOShield and JRGB1 are channels of the exact MSI endpoint,
not separate USB devices. Inventory and output selection follow each other. The
status beside Redo is a plain label; the old action is disabled and removed from
the toolbar. Resume is neutral and disabled for running/unconfigured selections.
Stop/Resume targets the selected channel or all configured channels of a selected
controller. Pausing filters channel commands, preserves the shared transport and
holds the last state; already executing writes may finish and native firmware
animations may continue. Peers keep running, including after another endpoint
faults or fails to reconnect. Tray/global Stop still disables automatic retries.
Pauses last for the session, survive scope reconnects and reset at next launch.

The interrupted LED-presentation/tray experiment was removed from the working
changes: the Vulcan geometry/model decision remains under discussion. The user's
untracked default.lighting.json remains untouched and is not part of this commit.
It currently has 1/5/4 placements on JARGB1/2/3; do not silently replace them with
assumed hardware counts.

Validation: final Release and Debug each pass 15/15 CTest. Native Windows editor
36/36 and output UI 13/13 pass. Recording transports prove a changed physical wire
report before mouse release, Escape restoration, one undo commit, selection from a
group, independent header pause/resume, continued peer writes and fault isolation.
The animation regression waits for observed per-area color changes rather than a
fixed 200ms delay. Initial connection submits once; Resume only resubmits immediately
for an already active controller. Screenshots refreshed and native Hardware UI
inspected. No real hardware writes were performed by the agent.

Portable folder and ZIP refreshed; CRC, required DLL/module entries and exact
source/folder/archive EXE equality pass. Packaged SHA256:
ee7f9af6e671eda63b4f0fe28894a39239d4cf9654caad2717a40f2303409fac.

## Per-device LED arrangements restored (2026-10-24)

Select a placement in Layout or Hardware and choose Edit LED arrangement. The
existing visual geometry dialog now provides Line, Circle, Ellipse and Rectangle
presets matching the original Component::arrange shape set, individual dragging,
wheel zoom, Fit LEDs and Reset arrangement. Device mode hides the definition ID
and locks name, LED count and wire mapping. Shared definition authoring remains
available separately.

LightingDocument::setComponentGeometry copies geometry into an instance-specific
embedded definition, keeping sibling placements, wire order and project schema
unchanged. Validation, sampling, undo and portable save/load use existing paths.
Private definitions count toward the existing 256-definition limit; repeated
edits of the same placement reuse its definition. No hardware write was performed
by the agent; physical illumination remains unobserved.

Validation: Release and Debug each pass 15/15 CTest; native Windows editor passes
36/36. Regression exercises instance isolation, undo/redo, rejected count/unknown
placement, portable reload, shape actions, reset, original wire-order sampling and
the actual Layout action/dialog/commit path. Native dialog screenshot inspected.
Portable folder and ZIP refreshed; CRC, hidapi entry and EXE byte equality pass.
Packaged EXE SHA256: 6439eeb66c0feb06c90a29b8da5b3b8d46719c688242cb28c81243bf4bf57619.

## Draggable group labels and automatic canvas output (2026-10-24)

The user's current instruction explicitly removes the report-approval checkbox
and requires canvas output by default. This supersedes earlier handoff entries
requiring exact-report consent. Do not reintroduce that dialog or a hidden opt-in.

Layout group label painting and hit testing share the same bounds calculation.
Dragging a label selects its assignment and moves every member in one undo step,
even if an individual component was previously selected. Body, scale and rotation
behavior remains available.

Main enables automatic output after project restoration/import and window setup.
Configured, discovered endpoints connect without a second process-conflict/report
dialog. Startup conflict decisions remain user-controlled; no process is killed
automatically. Exact descriptors, paired Vulcan interfaces, serialized reports and
channel capacities are still validated before opening hardware. Offline saved
endpoints are skipped so present headers can run. Topology/native changes stop old
workers and rebuild automatically; project loading reconnects after replacement.

Stop output disables automatic retries and pending queued restarts for the session;
Resume or a new app launch enables them. Inactive automatic output retries every
two seconds. Existing live connections are not periodically reopened. Partial
online sessions currently discover additional saved endpoints when their configured
scope changes or output is resumed; they do not rescan offline peers while active.

Validation: final Release and Debug each pass 15/15 CTest. Native Windows editor
35/35 and output UI 10/10 pass. Added regression covers dragging a group label
from component selection, group translation/undo, automatic startup without a
button, configuration reconnection, an offline subset and Stop before queued
reconnection. The report-approval function/dialog and its checkbox were deleted;
serialization tests remain memory-only.

Portable folder and ZIP refreshed; CRC, runtime/module entries and source/folder/
archived EXE equality pass. Windows-only PATH --inspect and read-only --scan exit 0.
Packaged EXE SHA256: a1cf53ba20142da4fd8f1cf5caae6ac4dd44fe9ce4679ca2d4c49d2b9f54f3f6.
Actual physical illumination remains unobserved; no real HID write was performed
by these automated checks. Launching the updated app now automatically sends its
canvas to present configured hardware, as expressly requested by the user.

## Travelling-gradient seam and paused output correction (2026-10-24)

Supersedes the raster-wrapping behavior documented in the previous entry. The
reported diagonal gradient at 144.87 seconds exposed a rectangular discontinuity:
wrapping a completed clamped image joins unrelated endpoint colors. Travelling
linear/radial gradients now evaluate translated source coordinates, with a closing
palette segment from the last color to the first as in the original
CustomGradientEffect::paletteAt. Arrow orientation and global X/Y travel remain
independent. Stationary gradients keep their clamped endpoint appearance.

Starting an animated output now resumes paused preview playback after successful
review/connection submission. Previously it could send the reviewed frozen frame
indefinitely. Static output does not force playback. Hardware's status label now
reflects actual output state, and the global action tooltip includes the status /
stop reason. Review and start remains an explicit user operation; no HID writes
were performed by this agent.

Expanded blend tests cover asymmetric RGB colors for Normal, Multiply, Screen and
Overlay, partial transparency plus gray masks, and the actual inspector selector.
All match expected results; no compositor defect was reproduced. A single bottom
layer has no backdrop to blend with; the tooltip now explains that. Blend issues
in an unprovided multi-layer project remain unverified rather than presumed fixed.

Validation: Release and Debug 15/15 CTest each. Native Windows editor 35/35 and
output UI 9/9. Seam regression scans every row of a diagonal three-stop gradient
at 0, 0.25, 1 and 144.87 seconds, checking continuity, perpendicular-axis invariance
and travel direction. A new paused-start output UI test runs the real timer,
scheduler and MSI serializer and confirms changing 0x51 reports for JARGB1 and
JARGB2 using a recording transport. No physical illumination proof is claimed.

Portable folder and ZIP refreshed; CRC, required runtime/module entries and
source/folder/archived EXE equality pass. Windows-only PATH --inspect and read-only
--scan return 0; four profile/endpoint matches found.
Packaged EXE SHA256: 231c4a07224dc180ca80e99335aa074fb18131fd281fb3953819c104ab1f2e4f.

Pending user observation: after Review and start output, does state stay Running
with static hardware, stop/fault, or was output never started? The supplied
screenshot shows both Stopped output and paused Play. This distinguishes physical
transport/configuration failures from intentionally unsent/frozen preview frames.
Do not auto-start hardware, terminate RGB processes or infer physical success.

## Drawing, animation and hardware inventory corrections (2026-10-24)

Supersedes the source-gallery/fill details in the previous status below. Shapes
and Particles are toolbar flyouts; Other effects contains gradients and patterns.
Bucket Fill compares unpremultiplied colors and retains antialiased alpha. Solid
procedural fills edit source color directly. Gradient gestures recolor Paint
coverage with SourceIn; procedural shapes retain their kind and geometry with
independent normalized fill endpoints. Schema 12 persists those endpoints and
rejects them under older schema numbers; documents 1..11 still load.

Every advanced motion value has a slider. Enable color animation activates
palette cycling and playback, including Paint coverage; gradient cycling updates
all stops. Apply animation to all visible layers controls subsequent motion edits.
Static gradients clamp without an internal wrap seam; animated drift wraps the
completed raster. Live hardware preview includes the composition and clock,
with canonical 640x400 sampling. Undo/redo icons use continuous curved arrows.

Hardware has a header/component overview sharing Layout's exact model and
selection. Add/Remove actions use existing validated, undoable chain operations.
Custom component definitions are collapsed. Peripheral devices cannot receive
motherboard fan/strip attachments. Actual OutputPanel remains in Hardware.

Validation: final Release build/next and Debug build/multi both pass 15/15 CTest.
Native Windows editor 35/35 and output UI 8/8 pass. New regressions cover alpha
edges, gradient fill/save/load/old-schema rejection, palette animation, all-layer
motion, live sampled LED changes, slider bindings and shared inventory/removal.
A recording transport verifies animated red/blue Paint frames through document
composition, component sampling, OutputSession and the real MSI serializer for
both JARGB1 and JARGB2, including actual 0x51 report bytes. This is protocol proof,
not physical illumination proof: no real HID writes were sent this turn.

Refreshed native screenshots and portable folder/ZIP. Package CRC and source /
folder / archived executable equality pass; hidapi.dll, qwindows.dll and both
modules are present. Windows-only PATH --inspect and read-only --scan return 0;
scan identifies MSI, Apex 3, Vulcan and Basilisk (four exact candidates).
Packaged EXE SHA256: 0f5aa6bf8f16c26fb40dfa2e87e4f0d1e1a53e71af5be75cb2277555dfdb6b5a.

Remaining hardware check: review the concrete reports in the app, explicitly
start output, then observe JARGB1/JARGB2 colors and motion on the connected fans.
Stopped sends nothing; no automatic output, process termination or reconnect was
introduced. Existing running app and unsaved user project were left alone.

## Restore layout and peripheral regressions (2026-10-23)

Supersedes the neutral Layout and detached output window below. Layout renders the
live raster behind groups, with dashed group bounds, corner scale and rotation
handles, body dragging and explicit zoom controls. Component definitions and the
real OutputPanel live in Hardware. Empty hardware preview space is returned to
Effects. Standard tools remain; Shapes and effects is the source gallery.

Gradient arrows remain editable after release; radial/conical controls retain
their source type and center/radius semantics. Fill now samples the selected
procedural/image source as its region boundary, painting only that connected
region on a transparent layer above the source. Paint fills include a small edge
color tolerance and explicit visited storage. Tooltips explain results and units;
palette cycle rate is beside its colors rather than hidden in another section.

Restored original Kone, Basilisk, Apex 3 and Vulcan protocol encoders and shipped
exact selector profiles/default geometry. Dedicated devices are discovered into
Layout automatically, never represented as fan/strip attachments. Vulcan requires
one unique control/LED pair of the same PID; both exact paths are reviewed and
revalidated. API 2 adds HID output transfers, retaining API 1 feature-report slots;
old hosts reject the new module metadata. No automatic hardware writes/reconnect.

A new empty session can recover original hardware layout/native commands from
%APPDATA%/MsiMysticLightApp/settings.json. Hardware also offers explicit import.
Imports are bounded, atomic, undoable and preserve source files/wire permutations;
existing assignments take precedence. Actual previous settings were checked
read-only against the fresh scan: six supported chains (JARGB1 6 addresses,
JARGB2 24, JARGB3 30, JRGB1 native, Basilisk 11, Vulcan 144). Discovery additionally
finds Apex 3. The packaged scan matches four device endpoints.

Validation: Release build/next and Debug build/multi each passed 15/15 CTest.
Native Windows editor 34/34 and output UI 8/8 pass. New coverage exercises source-
bounded fill, radial/linear arrow editing, group transforms/undo, hardware import,
HID output short writes, peripheral reports and ambiguous Vulcan pairing.
Native screenshots updated, including docs/images/hardware-workspace.png.
Graph refreshed: 1771 nodes/3559 edges; nine parser limitations, including two
new Qt-related declaration files. Prepared folder/ZIP refreshed, required files
and ZIP CRC checked, source/folder/archive EXE bytes match. Packaged --inspect
and --scan pass with Windows-only PATH. EXE SHA256:
cca33c7186c7e052a00921282324b1cdcb5bf825c16944a2b90036a913a89c45.

Remaining limits: no real HID writes or physical color/fan verification, and no
clean-machine runtime qualification. Old custom-effect recipes are preserved but
not converted by hardware-only recovery. The old worktree's GPU/RAM integrations
and IOShield software-average fallback are outside the modular host; do not claim
complete parity with every feature of the old Lighting Studio application.


## Drawing workflow, product picker and live hardware preview (2026-10-23)

Supersedes the source-icon toolbox and generic fan/strip dialog below. New layer
creates a transparent Paint source without a dialog. Standard drawing tools edit
pixels/masks; Presets is the sole procedural-source chooser. Particles form one
gallery family with shared controls and a contextual motion pattern. Gradients
use draggable stops, RGB sliders and Add/Remove for the selected stop. Schema 11
stores paint pixels/gradientPositions and preserves old project rendering.

Restored the original searchable hardware picker, product cards, categories,
quantities, M25 models and data-only installed component metadata. Validate and
normalize imported geometry without changing wire order; dedicated RAM/GPU/board
models are excluded from ARGB attachment. One attachment is an atomic undo edit.

Effects has a persistent fitted live hardware pane with sampled channel scopes
and clocks. Preview starts playing; authoring temporarily resets/pauses base time
and resumes prior playback on commit/cancel. No automatic HID start/reconnect.
Session layout format 3 resets obsolete toolbars while keeping projects/geometry.

Validation: Release build/next and Debug build/multi pass 14/14 CTest each.
Native Windows editor passes 32/32, output UI 8/8. Checks cover paint drawing,
flood fill, pixel/mask erasing, gradient-stop dragging/undo, source-color isolation,
source/paint persistence and invalid input/layer-cap guards; the hardware adapter
rejects invalid mappings and dedicated products. Pixel assertions verify that
channel-scoped hardware previews show distinct per-channel colors at Windows DPI.

Native screenshots: docs/images/studio-workspace.png, hardware-picker.png,
header-layout.png and lighting-properties.png. Prepared app and ZIP were refreshed;
EXE bytes match the build/folder/archive, ZIP CRC validation passes, required HIDAPI,
Qt platform and driver files are included. Packaged --inspect and --scan exit 0
with only Windows on PATH. The scan identifies this B850 Edge TI and one endpoint.
No hardware writes, physical color calibration or clean-machine test performed.
The graph was refreshed (1659 nodes/3297 edges, 7 existing parser warnings).



## Header-based application workflow (2026-10-23)

Supersedes the Devices-page navigation below. Layout opens first, discovers
supported controllers read-only and displays every profile header in its sidebar.
Select a header to append fan/strip quantities and explicit LED counts in one
validated undo transaction. Chains appear as groups with individually selectable
components; group drag/nudge moves the whole chain. Saved chains remain offline
editable. Custom component tools remain in a collapsed library. Output controls
open on demand from the toolbar; no permanent bottom panel or Devices dashboard.
Session layout version 2 resets obsolete dock arrangements without losing projects.

Validation: Release build/next and Debug build/multi pass 14/14 CTest each.
Native Windows editor passes 30/30 and output UI 8/8. Regression coverage includes
automatic/deduplicated discovery, atomic capacity rejection, fan/strip batching,
component removal, group drag/nudge, undo/redo and saved offline chains. Native
capture: docs/images/header-layout.png. The read-only scan identifies this machine's
B850 Edge TI and one matching endpoint. No HID reports were written.
Updated build/dist/LightingStudio and its ZIP; source/folder/archive EXE bytes
match, ZIP CRC validation passes, packaged --inspect exits 0. Start-Lighting.cmd
selects this prepared folder. Physical colors/calibration remain unqualified.


## Unified application and complete color/property editing (2026-10-23)

- Single app navigation: Devices (startup), Layout and Effects. Output is embedded
  on Devices; no bottom dock. Layout renders physical geometry on a neutral field.
- All effect sources have one-click toolbar tools. Compact active property grids
  expose Motion parameters at desktop size; inputs have explanatory tooltips.
- Global picker + RGB hex ColorEditor/ColorListEditor; no CSV color entry.
- Schema 10 persists 2..16 gradient stops; old projects keep two-color defaults.
- Periodic source rendering, drift, linked masks and warp re-enter opposite edges.
- Session state format 1 discards only obsolete dock layouts.
- Start-Lighting prefers the prepared build/dist/LightingStudio app folder.
- Validation: Release build/next and Debug build/multi each passed 14/14 CTest;
  native Windows editor 30/30 and output UI 8/8. Release build/ninja was not replaced
  because an existing running instance holds its EXE; no user process was killed.
- New raster/format tests cover wrapping, linked-mask edge coverage, three-stop
  gradients and portable persistence; UI tests cover global RGB controls and all
  effect tools. Native clipping checks verify the complete active Motion page.
- No physical-device writes, calibration or clean-machine runtime test performed.


## Visual workspace redesign (2026-10-23)

- Replaced the long shared form with left layers/components, fitted center canvas
  and contextual Effect/Motion/Mask inspector with live overview. Effects opens first.
- Searchable gallery uses the real renderer; layer subtitles identify effects.
- Precision controls start collapsed; brush/motion sliders and canvas tools are primary.
- Component geometry is draggable; channel colors use swatches and mode-specific UI.
- Output toolbar action/status connects to the actual reviewed output dock/pipeline.
- Effect edits and undo update approved software RGB payloads live. Endpoint/profile,
  ordered channels, modes, LED counts and native commands are checked before ANY
  replacement frame; structural changes stop all endpoints. Opening projects stops
  before loading. Edits during initial modal review invalidate that review.
- Validation: Release and Debug 14/14 CTest entries each; native Windows editor
  28/28 and output UI 7/7. Fresh extracted package --inspect exits 0 without SDK PATH.
- Native screenshots are in docs/images; the complete ZIP is under build/dist.
- See docs/EDITOR_DESIGN_REVIEW.md for references, motion decisions and qualification.


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

Color animation stop-position correction (2026-10-28): cycle palettes now use
editable positions rather than color reordering. Schema 15 stores optional cycle
positions; older documents keep legacy equal timing. Rendering reserves the
closing transition, and motion copy includes positions. Regression coverage checks
existing/new points, document refresh, undo, save/load, timing and invalid stops.

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
