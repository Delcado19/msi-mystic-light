# Continue here - state 2026-11-19

Only current state and open points. Finished work lives in the git history (this file was cut down
on 2026-11-19; earlier entries are in `git log -p -- CONTINUE.md`).

## Branching and releases

Single maintainer, so no `dev` branch. Every feature gets its own branch and reaches `master`
through a PR once CI and review pass. A hardware-tested `master` state gets a tag and GitHub
release (`vX.Y.Z`, last: `v0.1.0`); "stable" means the latest release, not `master`.

## Current state

- `master`: PRs #1-#4 and #6 merged.
- PR #5 (`chore/build-r26`, package GS-06-R28, `Downloads/LightingStudio-local-r28.zip`) is open:
  file dialogs start in the home folder; saved projects always end in `.lighting.json` (a
  replace prompt guards a suffixed name that exists); the uninstaller asks only about PawnIO,
  copies every `*.json` below `%LOCALAPPDATA%\Lighting Studio` to a dated backup, deletes the
  folder and shows the result in a highlighted box; an already elevated console keeps the files.
- Owner-confirmed on hardware: smooth color playback (R26), live drag, keyboard replug, lighting
  after wake, uninstall from an elevated console and unelevated (R28).
- Kept branches: `feat/hardware-photos` (10 transparent device photos for placement, keep),
  `docs/impeccable-product-design`. Worktrees `.claude/worktrees/lighting-studio` and
  `v1-skeleton` still exist; whether they are needed is unknown.

## Open tests

- R28 editor: `.lighting.json` suffix on save and the replace prompt.
- Event-driven presence check: service log at rest.
- LEDKeeper2 stop dialog flow; service pipe access list under other Windows accounts.
- JARGB2: not every intermediate strip position was qualified individually.

## Open work

- Docs, stage 2 (after the usage reset): verify the living references against the code
  (`LIGHTING_EDITOR.md`, `HARDWARE_MODULES.md`, `WINDOWS_SERVICE.md`, `ARCHITECTURE.md`,
  `OUTPUT_SESSION.md`, `NETWORK_PROTOCOL.md`, rest of `MB_800_HID_PROTOCOL.md`) and decide on the
  research files, `hardware-inventory/`, `HP_OMEN_MIGRATION.md`, `HEADER_MASK_CLARITY.md`.
- README screenshot: placeholder until a current screenshot exists.
- Operating-concept redesign (hardware cards per header like SignalRGB, duplicate/remove/pulse
  per component, custom strip by LED count, component lab, card view with device photos);
  needs a spec via brainstorming first.
- Custom hardware lives only inside the project: keep a per-user hardware library outside
  projects (cause: `HardwarePicker` gets `document_.catalog()` plus built-ins and SignalRGB files).
- Shared LED colors are hard to find: the option exists only in Layout > header > Header
  settings (`ChannelEditor`); fans on different headers cannot be linked at all.
- Effect library: previews missing; owner verdict on the R17 category submenus is open.
- Conflict detection gaps: interrupt-transfer writes (Vulcan), occasional writers, RAM/GPU; the
  periodic guard skips driven endpoints; hung discovery scan; portable mode keeps its name list;
  OutputPanel joins workers like the old service did. Vendor suites needed for macros (MSI
  Center, Razer, SteelSeries, Roccat, G-Menu) must not be stopped; for MSI Center the dialog
  should suggest switching Mystic Light off (not built). Whether `SignalRgbLauncher.exe`
  restarts a terminated `SignalRgb.exe` is unknown.
- Installer: PawnIO/app update check (not started); DisplayVersion in the Apps entry.
- IO Shield LED count on MS-7E62 unverified (see `docs/MB_800_HID_PROTOCOL.md`).
