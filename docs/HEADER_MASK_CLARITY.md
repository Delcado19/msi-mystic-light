# GS-06-R2 editor, background and mask validation

Date: 2026-11-13. Current local runtime:
`build/aurora/Release/msi_mystic_light.exe`. `Start-Lighting.cmd` selects it before
installed/older builds. A batch-selection probe preserves the real branches and
arguments but replaces only process launch with echo; the selected executable's
read-only `--inspect` check passes using its shipped Windows platform plugin.
The installed application/service has not been replaced.

## Resulting interface

- Unchanged owner-supplied turbine logo, right aligned at 360x120 DIP; 4K aurora
  photo fills the window with proportional scenery cropping.
- Navigator and Inspector use the same translucent surface. Empty viewport pixels
  match a single alpha-180 blend; document/mask raster pixels retain their own backgrounds.
- One grouped command row below the menu bar. Filename is in the window title;
  Fit remains direct, Zoom groups its steps. At 1440 DIP Composite remains visible;
  Panels is directly reachable at the 1060 DIP minimum.
- Centered chevrons and rounded dropdown frames. SVG decoding is included in app,
  tests, package inputs and installer checks; QtSvg linking alone was insufficient.
- Stable tool-option slots, clear hand/brush/resize cursors, a searchable flat
  effect library, and contextual canvas/Navigator edit actions.
- New layer is a paint source; New group is a container. All effect families use
  the selected-group/sibling/root creation rule. Existing source parents and files
  preserve hierarchy and rendering. No schema migration.
- Crop is enabled only for an editable image. Drag/move/resize a pending frame;
  Enter or double-click applies one undoable edit; Escape cancels. Source pixels
  remain embedded. Selecting a source shows its extent.
- Preview scope changes displayed composition/dims other outputs without changing
  canonical hardware samples or undo history. Global effects can look identical.
- Color Picker has a pipette icon and explicit desktop-sampling scope. Mask UI
  names the target, shared scope, output bypass, grayscale preview and brush coverage.

## Behavioral evidence

| Check | Result |
| --- | --- |
| Full Release build | Pass |
| Single complete Release CTest run | 22/22 groups pass; 160.14 seconds |
| Native Windows targeted run | 18 QtTest results pass, zero failures/skips; 14.67 seconds |
| Creation matrix | Every available effect-family action, group/sibling/root ownership, locked insertion, undo/redo and legacy round-trip pass |
| Crop | Pending vs committed, Enter/double-click/Escape, zero-area/reversed frame, repeated crop, move/corners, rotated geometry, original pixels and undo pass |
| Preview | Selected output pixels, other-output dimming, restoring All, canonical samples/history unchanged pass |
| Layout | Option positions/heights across tools, dropdown resource decoding, 1440 row visibility, minimum-width Panels and multiple background sizes pass |
| Teardown | Focus/hide callbacks run before document destruction with Move/Pan/Crop and an open fullscreen canvas; formerly crashing scope case passes |
| Desktop pipette | Real controlled RGB patch sampled correctly with two monitor overlays; editor geometry unchanged |
| Runtime | Launcher branch/argument selection and target `--inspect` pass |
| Portable artifact | ZIP CRC, current runtime equality, excluded user projects and extracted `--inspect` pass |

Local evidence: `build/aurora/release-reviewed-gates.txt`,
`release-reviewed-native.txt`, `launcher-target-inspect.txt` and Qt-generated captures.
Native screenshots: [editor](images/aurora-background.png),
[mask inspector](images/mask-inspector-clear.png),
[panel transparency](images/panel-transparency.png).

## Investigation and limits

The old viewport palette-alpha assertion described an unused palette role, not
stylesheet painting. Pixel comparisons now use matched backing-store DPR; this
removes a one-unit interpolation difference at Windows 125% without tolerance
relaxation. Offscreen tests load installed Windows fonts instead of oversized
placeholder glyphs. The native monitor clamps the largest requested window from
3200x1800 to 3200x1415 physical pixels; offscreen covers the requested larger sizes.

The native abort was a real late cursor read after the value-member document was
destroyed. Explicit early hiding plus document-independent cursor reset resolves
the reproduced path and passes the focused, native and complete suites. This is
regression evidence, not an exhaustive memory-sanitizer audit.

No real RGB writes, SignalRGB stop/UAC or service installation was performed.
Historical full-width banners were rejected; earlier passing captures do not
qualify newer code. See LIGHTING_EDITOR.md for the complete editing contract.

Local portable archive: `build/dist/LightingStudio-Editor-R2-windows-x64.zip`.
The knowledge graph was updated with AST only; 14 pre-existing partial-parser
warnings and stale community labels remain. No semantic/LLM relabeling was run.

Portable ZIP SHA256: `0b2a74180fb7eb358748088b6cec2f4604ebbe713cf438d009fa6796315407aa`.
