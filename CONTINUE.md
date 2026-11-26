# Continue here - state 2026-11-19

Only current state and open points. Finished work lives in the git history (this file was cut down
on 2026-11-19; earlier entries are in `git log -p -- CONTINUE.md`).

## Build GS-06-R39 - 2026-11-26 morning (local, not pushed)

- USB captures (Wireshark/USBPcap on `\\.\USBPcap3`, Basilisk = device 3, 40 s each; files in the
  session scratchpad, not kept): with Synapse 4 lighting "Static", Synapse sends one plain 0x0F/0x03
  colour frame every 5.0 s and never reads the device -> the 5 s hiccup. With Synapse brightness OFF,
  Synapse sent nothing in 40 s; the mouse stayed dark because the device brightness is 0.
- Fix: Basilisk driver sends brightness 255 (0x0F/0x04, NO_SAVE, LED_ID_ZERO, as OpenRGB) on the
  first frame and once a second after that. Intended setup: Synapse keeps running with brightness OFF
  (macros/remaps stay), Lighting Studio drives the colours. Test in `test_peripherals` (reports).
- Delivered as `Downloads/LightingStudio-local-r39.zip` + `.zip.sha256`
  (SHA-256 `33edf5dad2ba1a3c039f32e173794c3a5c7a8a5c3f5a45e3b505fc7753c7421c`), 58 files, includes the
  status-row fix 0131fd0. 23/23 CTest. Not run: installer, real mouse with the new command.
- Unmeasured: whether Synapse sets brightness 0 again on its start, profile switch or resume (the
  1 s refresh would mask it). Our driver still sends the custom-mode report with every frame (26/s);
  unnecessary but harmless, left as is.
- Next: owner installs R39, Synapse brightness OFF + Ignore in the conflict dialog; check the mouse
  shows our effect without hiccup and buttons/macros work; then rethink the conflict dialog for
  suites (suggest "brightness off" instead of terminate).

## After R38 - 2026-11-25 night (local, not built into a ZIP, not pushed)

- Owner ran R38. Component Editor status row jumped on mouse move: the "grid x, y" label grew,
  squeezed the word-wrapped hint onto two lines and the canvas moved. Fixed: grid/zoom labels reserve
  their widest text (`ComponentEditorPage.cpp`, test in `componentEditorInstanceMode`, fails without
  the fix). 23/23 CTest. Not checked by the owner in the real app.
- Razer (owner observation, R38): Synapse 4 (`RazerAppEngine.exe` app-4.0.896, ~10 processes +
  `RzEngineMon.exe`) writes lighting to Basilisk V3 `VID_1532&PID_0099&MI_03`; mouse flickers while
  both run, also after closing the Synapse window (processes stay). "Terminate all" restores our
  output but kills the mouse button functions. Synapse offers no separate lighting module (Chroma app
  not installed). Unmeasured: whether Synapse writes continuously or only on events.
- MSI: owner can uninstall the Mystic Light module in MSI Center separately (keeps the rest).
- Options discussed for Razer, nothing decided or built: (a) save the button mapping to the
  Basilisk's onboard memory in Synapse, then run without Synapse (whether onboard keeps all of the
  owner's functions, e.g. macros, is unverified); (b) "leave device to the suite" from
  `docs/VENDOR_SUITE_COEXISTENCE.md`; (c) Chroma SDK path (unresearched). Open owner questions are in
  that doc.

## Build GS-06-R38 - 2026-11-25 late evening (local, not pushed)

- Cause of the fan noise, measured (boot CPU trace + cdb stack samples): the logon-started tray editor
  (`--background`) rendered its preview every 16 ms on the GUI thread, one core at 100% for the whole
  session. The service itself used ~0.8% of one core for the same lights.
- Fix `db48ec5`: no tray icon, no `--background`, no `Lighting Studio Editor` logon task (the installer
  removes an existing one); closing the window ends the editor. Beside the service a minimized window
  skips the preview render (`setRenderWhenMinimized(false)`, test `minimizedPreviewIdle`).
- Delivered as `Downloads/LightingStudio-local-r38.zip` + `.zip.sha256`
  (SHA-256 `dfed3fa6726996c4ac59842c84fb0342a778178494f1f636a2f5ebed27fc15e8`), 58 files (R37 + the AJAZZ
  component and experimental profile from phase A). 23/23 CTest. Not run: installer, real window, LED output.
- Owner state at that time: service `playback.json` had `enabled=false` since 19:38 (cause unknown), so
  Lighting Studio did not drive the lights; SignalRGB did. Owner wants Lighting Studio to own the lights;
  SignalRGB stays installed only as a code reference and must not autostart.
- Still unmeasured: CPU of the editor with its window visible (one frame seems to take >= 16 ms).
- Boot trace script `scripts/boot_cpu_trace.ps1` (untracked) and its CSVs in
  `%ProgramData%\LightingStudioBootTrace` are kept for a re-measurement; its scheduled task is gone.

## After R37 - 2026-11-25 evening (local, NOT built into a ZIP, not pushed)

State: branch `feat/lighting-studio-ux-redesign`, all local commits unpushed (PR #10 is the draft).
23/23 CTest green in `build/r36` (build: `build\r36-build.cmd`, tests: `build\r36-ctest.cmd`,
single test slots: `build\r37-slot.cmd <exe> -o <file> <slot>`).

- Fixed after the owner's R37 screenshots, offscreen-tested only:
  - Component Editor: the wire path is computed over all layers; edges are drawn only between
    visible LEDs (R37 showed a false dashed 12 -> 25 jump on SWAFAN layer 1).
  - Devices preview: the product photo keeps its aspect ratio (frame grows to the photo ratio instead of
    stretching the photo into the LED bounds). Old behavior since `ae0a68d`, not an R36 regression.
    No automated test; check in the real window.
- Not changed, explained to the owner: ThermalTake Riing Quad v2 has two close rings in the SignalRGB
  data (one grid unit apart, not identical), so 24 px markers overlap until zoomed in. Possible
  follow-up: smaller markers or labels at low zoom; owner has not decided.
- AJAZZ AK35I V4 phase A (`docs/superpowers/plans/2026-11-25-ajazz-ak35i-v4-phase-a.md`):
  Tasks 1-4 done offline (`78bb4a0` encoder vs captures, `bc2e34f` scripted fake HID, `f768d32`
  driver branch + profile `profiles/experimental/ajazz-ak35i-v4.json` + 105-key component,
  `acf2a46` dry-run ACKs for the output review gate). Peripherals module is now API 3.
  The experimental profile is NOT loaded by editor or service (both read `profiles/*.json`
  non-recursively; checked by reading code, not by a test). No hardware I/O has happened.

Next steps, in order:
1. Owner checks in a new build: Devices preview proportions, SWAFAN layer 1 without the false jump,
   layer box / path colors. Then bump to R38 and package (`scripts/package_release.py --runtime
   build/r36/Release`).
2. AJAZZ Task 5 needs explicit owner confirmation of D1 (Settings checkbox "AJAZZ AK35I V4
   (experimental, I own this exact model)", default off, loads the experimental profile). Open
   question before coding: how the service reads editor settings; stop and report if it does not.
3. AJAZZ Task 6: docs (`docs/AJAZZ_AK35I_V4_PROTOCOL.md` status, `docs/HARDWARE_MODULES.md` API 3).
4. Phase B (hardware, with the owner, step by step): START only, then Esc red / F1 green / F2 blue at
   low brightness; verify the 65-byte GET length on Windows hidapi (driver requires exactly 65).
5. Push to PR #10 only after owner approval.

## Build GS-06-R37 - 2026-11-25

- Delivered as `Downloads/LightingStudio-local-r37.zip` with its `.zip.sha256` sidecar
  (SHA-256 `14c44547918478e98dfa76866abf6fe8077f88e72ac43dead1bc5a4b7f3ad421`). R36 plus display-only
  LED layers in the Component Editor (plan `docs/superpowers/plans/2026-11-25-component-led-layers.md`):
  layer box below the tool rail, LAYERS section, PgUp/PgDn and Ctrl+PgUp/PgDn, colored wiring runs
  with dashed jump arrows, SignalRGB import puts LEDs on identical coordinates on separate layers
  (SWAFAN, Freezer 36, CoreLiquid block, Vertagear SL5000). Positions and effects are unchanged. Wiring
  across layers: switch layers with PgUp/PgDn while the Wire tool keeps the pending order (status hint).
- Checks: 22/22 CTest with the R37 ID, same 56-file list as R36, `GS-06-R37` marker inside the ZIP.
  Not run: real window (colors on the dark canvas, layer box placement), installer, physical output.
- Unverified: whether projects that already embed a SWAFAN definition pick up the import layers or keep
  their embedded copy without layers until the component is placed again or edited.
- Next: AJAZZ AK35I V4 phase A (`docs/superpowers/plans/2026-11-25-ajazz-ak35i-v4-phase-a.md`),
  D1 = Settings checkbox: assumed; the owner said "go" after this assumption was stated, confirm before task 5.

## Build GS-06-R36 - 2026-11-25

- Delivered as `Downloads/LightingStudio-local-r36.zip` with its `.zip.sha256` sidecar
  (SHA-256 `b2515c30adb04f1fa536732c8f963896f43ec7fff4fac7627f54b966d555e871`). R35 plus: Devices
  preview renders at the panel size, preview and output endpoint list share the spare height (1:2)
  instead of a trailing spacer, Component Editor shape buttons have icons.
- Built in a fresh `build/r36` tree: the old trees still point at the pre-move paths
  (`Software_Projects/msi-mystic-light`, `Software_Projects/OpenRGB`). `build/r36-build.cmd` uses
  `Lighting-Projects/OpenRGB/OpenRGB/dependencies/hidapi-win`.
- Checks: 22/22 CTest with the R36 ID, same 56-file list as R35, `GS-06-R36` marker in the editor.
  Not run: the new layout in a real window, packaged editor, installer, physical output.

## Build GS-06-R35 - 2026-11-25

- Delivered as `Downloads/LightingStudio-local-r35.zip` with its `.zip.sha256` sidecar
  (SHA-256 `1d905f73c768904a532b9e023658e98f0646244baf1c2d5081c021a068e79083`). Contains R34 plus the
  full-window Component Editor, the Phanteks NV5 MKII internal strip (16 addresses) and GUI tests
  that start without the ctest environment.
- Checks: 22/22 CTest before the build-ID bump, ZIP CRC, same 56-file list as the R34 delivery
  (three JSON files differ only in line endings), `GS-06-R35` marker in the editor. Not run: packaged
  editor, installer, physical output; the Component Editor was only exercised offscreen.

## Build GS-06-R34 - 2026-11-24

- Delivered as `Downloads/LightingStudio-local-r34.zip` with its `.zip.sha256` sidecar
  (SHA-256 `afc145f4c3761a80c4ee849412f10eb74d6c27a9da039946f4236314280ce3b4`). Contains R33 plus the
  photo decode fix, last-workspace-tab restore and anchored effect corner resize (Shift/Alt).
- Checks: 21/21 CTest (installer-plan test excluded), ZIP CRC, same 56-file manifest as R33,
  `GS-06-R34` marker in the editor, SHA-256 of the Downloads copy. Not run: packaged editor,
  installer (not even `-WhatIf`), physical output.

## After R34 - 2026-11-24 (local, not packaged)

- Panels: minimizing no longer records both docks as hidden (`7f6860a`).
- *Edit > Settings...* with "Disconnected devices: Hide (default) / Dim". Components whose
  controller the last scan did not find are hidden on Layout/Effects and not pickable; Devices
  still lists them. Offscreen test only. Open: no automatic rescan on hotplug (manual "Scan
  hardware" or restart); a peripheral on another USB port gets a new placement and the old
  one stays hidden in the project.

## Redesign follow-up - 2026-11-22

- The approved redesign plan (Codex, 2026-11-21) and its implementation status as of 2026-11-24
  are in `docs/UX_REDESIGN_PLAN.md`. Visual rules are in `DESIGN.md` on `docs/impeccable-product-design`.

- Local Windows build GS-06-R33 was delivered as
  `Downloads/LightingStudio-local-r33.zip` with its `.zip.sha256` sidecar.
  Extract it and run `LightingStudio/installer/Install.cmd` to install.
  The package includes the editor, service, installer, uninstaller, runtime
  dependencies, profiles, and component definitions.
- Package checks passed for ZIP integrity, the R32 file manifest, the R33 build
  marker, the SHA-256 of the copy in Downloads, packaged editor `--inspect`,
  and installer `-WhatIf`. The R33 installer was only run in dry-run mode.
- All 21 regular CTest suites passed. The installer-plan test hit a restricted
  HKCU test key and was excluded from the final run.
- Devices actions keep their position when switching from a connector to a component.
  Layout and Devices open the same LED editor for attachable components. Wiring
  shows an address path without the component artwork. The Effects mask inspector
  shows the active brush preset, and source frame clipping lives under Source.
- Native Windows tests exercised these paths on test fixtures. The reported
  fullscreen edge offset was not reproduced in the 2560 x 1440 test fixture.
  Physical LED output and broader Component Lab feature parity remain open.
- 2026-11-25: Custom components dialog and Component Lab replaced by the full-window Component
  Editor (Devices > Component Editor; plan docs/superpowers/plans/2026-11-25-component-editor.md).
  Offscreen tests only; not yet checked by the owner in the real app.
- Review 2026-11-22 (Claude): Codex's tree committed as-is (ae0a68d, local, unpushed). Fix: the
  Devices thumbnail decoded a full-size hardware photo on every preview frame; photos are now
  decoded once, scaled to 512 px and kept in `QPixmapCache`. 22/22 CTest green; not run on the rig.
- The hardware photos add ~19 MB to the repo and the exe (1254-2368 px PNGs, shown at most 176 px).
  Downscaling the source files is recommended before pushing (public repo); not done, owner decision.
- Vendor suites (macros/key remapping) vs. conflict stop: concept in
  `docs/VENDOR_SUITE_COEXISTENCE.md`, waiting for owner answers; nothing built.

## Branching and releases

Single maintainer, so no `dev` branch. Every feature gets its own branch and reaches `master`
through a PR once CI and review pass. A hardware-tested `master` state gets a tag and GitHub
release (`vX.Y.Z`, last: `v0.1.0`); "stable" means the latest release, not `master`.

## Current state

- `master`: PRs #1-#4 and #6 merged.
- PR #5 (`chore/build-r26`, package GS-06-R29, `Downloads/LightingStudio-local-r29.zip`) is open:
  file dialogs start in the home folder; saved projects always end in `.lighting.json` (a
  replace prompt guards a suffixed name that exists); the uninstaller asks only about PawnIO,
  always removes the editor registry settings and never touches `%LOCALAPPDATA%\Lighting Studio`,
  however it is started; a highlighted box names that folder at the end (owner decision
  2026-11-19, replacing the backup copy and the scheduled-task backup as the signed-in user).
- Owner-confirmed on hardware: smooth color playback (R26), live drag, keyboard replug, lighting
  after wake, `.lighting.json` suffix on a typed `test.json` (R28). The uninstall tests of
  2026-11-19 (elevated console, unelevated, Settings > Apps with the scheduled-task backup) covered
  the replaced backup behavior; the new behavior is untested.
- Kept branches: `feat/hardware-photos` (10 transparent device photos for placement, keep),
  `docs/impeccable-product-design`. Worktrees `.claude/worktrees/lighting-studio` and
  `v1-skeleton` still exist; whether they are needed is unknown.

## Open tests

- R28 editor: replace prompt when the suffixed name already exists.
- R29 uninstall via Settings > Apps and via `Uninstall.cmd`: same box naming the kept folder, editor
  registry key `HKCU\Software\Lighting Studio` gone.
- Event-driven presence check: service log at rest.
- LEDKeeper2 stop dialog flow; service pipe access list under other Windows accounts.
- JARGB2: not every intermediate strip position was qualified individually.

## Open work

- Docs, stage 2 (after the usage reset): verify the living references against the code
  (`LIGHTING_EDITOR.md`, `HARDWARE_MODULES.md`, `WINDOWS_SERVICE.md`, `ARCHITECTURE.md`,
  `OUTPUT_SESSION.md`, `NETWORK_PROTOCOL.md`, rest of `MB_800_HID_PROTOCOL.md`) and decide on the
  research files, `hardware-inventory/`, `HP_OMEN_MIGRATION.md`, `HEADER_MASK_CLARITY.md`.
- README screenshot: placeholder until a current screenshot exists.
- Redesign branch: Devices starts with connector setup and offline planning. Layout and Effects
  are direct workspaces. Custom components and Custom effects are stored per user.
  Projects reference components, and service playback carries the required definitions.
  Identify and the Phanteks M25 address mapping still need physical qualification for this build.
- Conflict detection gaps: interrupt-transfer writes (Vulcan), occasional writers, RAM/GPU; the
  periodic guard skips driven endpoints; hung discovery scan; portable mode keeps its name list;
  OutputPanel joins workers like the old service did. Vendor suites needed for macros (MSI
  Center, Razer, SteelSeries, Roccat, G-Menu) must not be stopped; for MSI Center the dialog
  should suggest switching Mystic Light off (not built). Whether `SignalRgbLauncher.exe`
  restarts a terminated `SignalRgb.exe` is unknown.
- Installer: PawnIO/app update check (not started); DisplayVersion in the Apps entry.
- Projects with the previous six-point M25 geometry keep their existing definition on migration.
  New attachments use the twelve-point model. Use the Component Editor to prepare a replacement
  variant for an existing layout if desired.
- Editor: Move tool does not grab a particle effect (falls back to marquee selection). Effect corner
  resize (2026-11-24) pins the opposite corner and keeps the aspect ratio; Shift frees the aspect,
  Alt scales about the center (offscreen test only, not tried by the owner). Frame clipping is opt-in
  (`clipToFrame`, branch `fix/effect-bounds-resize`, not HW/UI-tested).
- Performance with several simultaneous effects is suspected to drop sharply; not measured.
  Every gradient/pattern source evaluates all canvas pixels per frame.
- IO Shield LED count on MS-7E62 unverified (see `docs/MB_800_HID_PROTOCOL.md`).
