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
