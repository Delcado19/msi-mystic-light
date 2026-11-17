## Branching and releases - 2026-11-15

Single maintainer, so no `dev` branch. Every feature gets its own branch and reaches `master`
through a PR once CI and review pass. A hardware-tested `master` state gets a tag and GitHub
release (`vX.Y.Z`, last: `v0.1.0`); "stable" means the latest release, not `master`.

## Installed editor could not reach the service - 2026-11-17

Owner report: empty conflict dialog ("Lighting service identity rejected"), SignalRGB effects stayed.
Reproduced unelevated: `OpenProcess` on the LocalSystem service fails with ERROR_ACCESS_DENIED, so
`trustedServer` rejected every request, including project sync (`C:\ProgramData\LightingStudio` had
no playback.json). It now trusts the SCM PID plus the SCM-registered binary path. The tray icon is
created right after the instance lock, before the blocking conflict scan, so "Open it from the tray"
is true. Uninstaller question about user data names what it deletes. Release build and
startup/service tests pass; untested against a real installed service. Open: slider drag jumps,
split hardware preview shows the same view twice, uninstaller elevated window did not close on Enter
(exit 0xC000013A after closing it with X, cause unknown).

## Responsive service and installed conflict dialog - 2026-11-16

Branch `fix/service-responsive-conflict-stop` (off `feat/jargb2-shared-routing`). Owner report after
the first real installation: editor showed "Connection error" / "No compatible controller" and no
conflict dialog. Cause 1: discovery ran on the pipe thread (~6.5 s per scan, every 5 s; editor
connect timeout 1.5 s); it now runs on a worker (`ServiceRuntime::setBackgroundScan`). Which part of
the scan is slow is unmeasured; HID enumeration is not (0.1-0.2 s). Cause 2: the dialog was skipped
when the service is installed. It now appears whenever the installed editor window is opened and the
service performs the stops (`conflicts` / `stopConflict`), temporary only (owner decision). Unit
tests pass; nothing here has run against the installed service or real conflicting software yet.

Owner correction the same day: conflicts must be found per device, not by name list. Rebuilt in
`src/startup/DeviceConflicts.*`: exclusive-open probe per configured HID endpoint the service does
not hold, one handle lookup per busy period, block only the busy endpoints. The name list only labels
hidden-holder candidates. Built, untested on hardware: probe inside the service, lookup as
LocalSystem, hidden-holder candidates, generic service stop, and the program family of each holder
(`processFamily`: same install root, parent older than child, never Windows or our executables,
topmost first). Only `\\?\HID#` paths are probed (security review: project paths reach a SYSTEM open).
Later the same day the handle lookup and its "hides its handles" candidates were replaced by a USB
trace (writers only), after read-only ETW experiments (scripts in build/research/etw, not in git):
UCX event 53 carries the sending process for HID SET_REPORT; completion events do not. Verified on
real hardware with a throwaway harness (build/research/usbprobe): SignalRgb.exe found as writer of
MSI controller, Apex 3 and Basilisk, plus its family; without SignalRGB nothing blocked although the
MSI controller (MSI Center reads it) and Basilisk (hidserv) are busy. Gaps: interrupt-transfer writes
(Vulcan), occasional writers, RAM/GPU. MSI Center: with Mystic Light switched off LEDKeeper2 sent no
writes in 90 s; the dialog should suggest that switch instead of stopping MSI Center (not built).
Open: the conflict scan runs synchronously in the editor's GUI thread (CodeRabbit 2026-11-16), so the
dialog and "Resolve conflicts..." freeze the window for one USB trace (5 s) while a device is busy.

Findings for the next step (detection beyond the name list, read-only experiments 2026-11-16):
an exclusive-open probe (share mode 0) reports a held HID interface even when the holder protects
its handles (SignalRGB does; handle enumeration cannot see it); handle enumeration names unprotected
holders such as MSI Center's `CC_Engine_x64.exe`; Restart Manager and Kernel-File ETW do not cover
devices. A held interface is not proof of colour writes. Vendor suites the owner needs for macros
and key mapping (MSI Center, Razer, SteelSeries, Roccat, G-Menu) must not be stopped; their handling
is open. `SignalRgbLauncher.exe` stays off the name list (test `exactNamesAndGuards`); whether it
restarts a terminated SignalRgb.exe is unknown.

## Uninstaller and CI package - 2026-11-15

`installer/Uninstall-LightingStudio.ps1` removes everything the installer creates and is registered
in Windows Settings > Apps; PawnIO and editor settings only on request (docs/WINDOWS_SERVICE.md
"Removal"). Parse/WhatIf test only, never run against a real installation. Every CI run attaches
the installable ZIP; `release.yml` drafts a release on a `v*` tag. The editor executable has an
icon (`src/app.rc`). Open: PawnIO/app update check (not started), DisplayVersion in the Apps entry.

## GS-06-R9 - 2026-11-15

Hardware view has an LEDs column: header rows show used / max output positions (shared outputs once),
component rows their output range. Build ID GS-06-R9. Column not checked visually yet.

## Header LED count - 2026-11-15

Header summary now counts output positions instead of summing every component's LEDs, so shared
(splitter) outputs count once. Document test outputLedCount passes; not checked against the real
header yet. Card view with per-component photos and the component library are still open.

## Identify flash - 2026-11-15

Layout context menu "Flash component (identify)" lights one software component white for 2 s
(see docs/LIGHTING_EDITOR.md). Document-level test passes (22/22 ctest, build/r4). Confirmed by the owner on real hardware (2026-11-15: "flash funktioniert").
Details of the observed lighting were not recorded.

## LEDKeeper2 stop - 2026-11-14

Conflict dialog now stops `Mystic_Light_Service` (session only, start type unchanged) and then
terminates LEDKeeper2; before, LEDKeeper2 was manual-stop only. Owner test as admin confirmed the
service is what restarts LEDKeeper2. Needs the editor running as administrator. Dialog flow untested
on hardware.

## Service pipe access list - 2026-11-14 (Windows only)

`ServiceServer` creates its own named pipe instead of QLocalServer: DACL grants only SYSTEM,
the server account and the configured owner (no Everyone), remote clients rejected, first-instance
flag makes a pre-created pipe name fail the bind. Impersonation check stays as second layer.
Tested in simulation (DACL read back, squatting host fails); other Windows accounts untested.
The installed service keeps the old pipe until it is reinstalled. Linux: QLocalServer unchanged,
SO_PEERCRED owner check deferred until Linux is targeted (owner decision).

## Editor fixes and small audit fixes - 2026-11-14 (GS-06-R7, build/r4)

Branch `feat/jargb2-shared-routing`, launcher prefers build/r4. Editor: Header settings
rows aligned, Space panning, drawing options only in Layers, logo right and trimmed
(window state version 6 discards older toolbar layouts), auto-fit follows Layout edits,
effect area covers housings and keeps the 640:400 aspect (changes LED sampling for
layouts outside 640x400), all LEDs/keys numbered, lock moved to Edit with padlock,
one wheel notch = one unit on sliders, Help > Keyboard shortcuts.
Audit fixes: PawnIO DLL path from the Windows known folder; padded hid_write accepted;
Apex 3 brightness once per session; SMBus port restore retried once; service skips
rendering without connections and lists status from a black frame; portable editor
reconnects 3 s after Windows resume; installer quotes root paths and runs the PawnIO
setup from an administrators-only copy. Not done (needs a decision): lower-privilege
service account or separate project processing, pipe ownership check before accept.
Only software tests (22/22) and offscreen screenshots; no hardware, resume or real
installer run was performed.

## JARGB2 shared routing - 2026-11-14

Branch `feat/jargb2-shared-routing`: chain items accept an explicit one-based
start position (`ChannelAssignment::starts`, Hardware > Header settings: wiring
choice One after another / Splitter cable plus a Starts at LED field per row);
`sampleChannels` resolves overlaps (earlier chain item wins). Layout uses
`LightingDocument::previewComponentColors`, shows LED numbers on shared headers
and Color source / Same as #n badges. Window build ID is GS-06-R3. Projects using it save as schema 23, others
stay at 22. See docs/LIGHTING_EDITOR.md "Shared output positions" and the status
section of docs/JARGB2_TOPOLOGY_TEST.md for what is not done (16-LED strip
definition, personal layout, Hardware LED summary, hardware confirmation).
The installed service host must be updated together with the editor before a
schema-23 project is sent to it.

## JARGB2 topology follow-up - updated 2026-11-14

See docs/JARGB2_TOPOLOGY_TEST.md for packets, observations and limitations.
Owner observed synchronized positions1..6 across all four fans, each zone
comprising two vertically mirrored LEDs; the strip shares that initial range.
The explicit 16-yellow/17-magenta blink confirmed yellow as the last visible
strip position. Working mapping: four overlapping fan ranges1..6 and strip1..16.
Not all intermediate strip positions were individually qualified.
Production sampling still concatenates component colors; shared routing and
its source/conflict policy are not implemented. Personal layout is unchanged.
SignalRGB stopping was explicitly authorized for follow-up tests; startup
configuration was not changed. Sleep/resume cause remains unresolved.
The continuous 16-yellow/17-magenta blink is still running at this handoff
(session99522, apply-blink16-17.py). Stop it before any other hardware writer;
do not automatically send black merely because the owner provides feedback.
Earlier R2 no-write statements refer to software qualification before testing.

## GS-06-R2 UI correction - 2026-11-13

Full Release build passes. One complete CTest run passes 22/22 groups in 160.14s
(build/aurora/release-reviewed-gates.txt). Native Windows targeted tests pass
18 results with zero failures/skips, including exact two-monitor desktop color
sampling and active Move/Pan/Crop/fullscreen teardown (release-reviewed-native.txt).

The current executable is build/aurora/Release/msi_mystic_light.exe. The actual
Start-Lighting.cmd branches/arguments resolve to this file; its read-only
--inspect check passes using the shipped Windows platform plugin. A mistaken
offscreen-only app probe was cleaned up and rerun with the correct platform.
No installed service/editor was replaced, no physical RGB writes or real
SignalRGB stop was performed. Commit/push requested by the owner; Git history and the remote identify publication.
Local R2 ZIP CRC, runtime equality, exclusion of personal projects and extracted
--inspect pass. Graphify AST refreshed; 14 existing
partial-parser warnings and stale community labels remain; no LLM relabeling.

Implemented: owner-supplied turbine logo/4K background; shared translucent panels;
grouped one-row commands with centered rounded dropdowns and shipped SVG plugin;
fixed tool-option slots; explicit paint/group creation and consistent selected
ownership; searchable effect library; contextual actions/cursors/selection;
image-only pending crop; effective Preview scope; mask-target/coverage clarity.
Existing hierarchy/rendering and image pixels are preserved, no schema migration.
The teardown use-after-destruction and active-group empty-click regression have
behavioral tests. Native monitor clamps the largest window; offscreen uses real
Windows fonts and matched-DPR comparisons.

Current contract and evidence: docs/LIGHTING_EDITOR.md,
docs/HEADER_MASK_CLARITY.md and docs/UI_UX_AUDIT_2026-11-13.md. Screenshots updated.
Hardware migration work below remains separate. Personal .taste-skill and
default.lighting.json are outside the change. Prior R1/rejected-banner readiness
statements below are historical, not a qualification of other installed binaries.

---

Entries from 2026-11-11 and earlier are archived in [docs/history/CONTINUE_ARCHIVE.md](docs/history/CONTINUE_ARCHIVE.md).
