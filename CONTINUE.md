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
