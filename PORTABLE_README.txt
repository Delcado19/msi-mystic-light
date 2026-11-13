LIGHTING STUDIO GS-06-R2 - 2026-11-13

The current source build is build/aurora/Release/msi_mystic_light.exe.
Start-Lighting.cmd in the repository prefers that build, then the installed
editor and earlier portable builds. Help > About shows the executable path.
The supplied turbine logo remains complete at the right; the approved 3840x2160
aurora photo fills the window behind translucent Navigator and Inspector panels.
File/workspace/history, Edit Mode, Panels, Preview and view controls share one row.
The filename is in the window title; Fit is direct and Zoom has a compact menu.
New layer creates a paint source. New group collects independently editable
children with a shared mask, opacity and blend mode. The plus button searches
all effect families. Tool options keep fixed slots; irrelevant fields disable.
Right-click offers contextual editing actions. Color Picker samples the desktop.
Crop requires an editable image: drag/move/resize a pending frame, Enter or
double-click applies, Escape cancels. Source pixels and undo remain available.
Preview scope dims other channels without changing device output.
SVG image-format support is required for dropdown arrows and included in runtime
packaging. See docs/HEADER_MASK_CLARITY.md and CONTINUE.md for qualification.

Earlier updates (historical):

RELIABILITY UPDATE (GS-06-R1, 2026-11-11)
Start-Lighting.cmd prefers build/aurora/Release/msi_mystic_light.exe, then an
installed service editor and build/dist/LightingStudio-ReliabilityFix.
Help > About displays the running executable path.
The complete header artwork is fitted without cropping or fading the logo.
Recovery writes use a two-second edit window instead of waiting indefinitely
for an idle gap. Failed saves remain visible and retry on the next edit.
Offline service endpoints no longer restart healthy controller connections.
Portable startup has an explicit Stop SignalRGB service (administrator) action.
It reads the installation path from SCM, requests UAC, and checks the stop result.
Close the SignalRGB application first. Start mode and scheduled tasks are unchanged.
The layer redesign is a proposal in the source repository's docs/LAYER_MODEL_PLAN.md,
not new behavior. The source repository's docs/COLLABORATION_FIXES.md records actual
tests and unverified physical/UAC behavior.

Earlier updates (historical):

STARTUP HOTFIX (GS-06, 2026-10-30)
Pre-artwork Qt toolbar states can crash before the window appears. Window-state
version 5 rebuilds those old dock/toolbar arrangements once. Window geometry,
explicit panel visibility, projects and recovery remain independently restored.
Current version-5 arrangements persist normally. The portable package also
includes the ASRock and Lenovo modules required by its current profile catalog.

ICE REFINEMENT (GS-06, 2026-10-29)
Larger owner-artwork strip with a soft native fade; older window layouts restore
it onto a separate full-width row. Effect/mask gestures keep
preview playback running; explicit Stop stays stopped. Layers have one row
selection marker. Plasma/Kaleidoscope gain spatial frequency, contrast and
pattern distortion; DNA gains Wave width. Visor Band width reaches 50 percent.
Lightning has growing branched channels/return strokes; Fireworks has spherical
spread, curved tapered trails and hot spark heads. New Fireworks defaults to 320
particles, with a supported maximum of 400. Existing saved particle counts stay.
Schema 22 persists the new spatial controls; older files retain neutral defaults.
Update editor and host together; older binaries cannot read new schema-22 files.

Earlier updates (historical):

ICE WORKSPACE (GS-05, 2026-10-29)
Owner-supplied ice panorama embedded in the header and shown fully in Help / About.
Deep navy surfaces, ice-white text and cyan focus/selection replace Stealth red.
All native patterns, including Ripples, support -600..600 cycles/min in the UI;
Repetitions supports 1..64. Renderer limits and file schema remain unchanged.
The RAM startup and continuous-header hotfixes are retained.

Earlier updates (historical):

STEALTH HEADER AND RAM STARTUP HOTFIX (2026-10-29)
The red header rule continues across inline preview controls in restored layouts.
Delayed RAM discovery starts saved RAM outputs even when HID peers already run.
Global Stop and individual channel pauses remain respected. No driver reinstall
or project migration is required. Physical RAM output still needs owner validation.

STEALTH REDESIGN (GS-04, 2026-10-29)
Graphite workspace with red interaction accents, panoramic branding, aligned
Motion/Color controls and responsive panels. Help / About shows the executable
path; the window title identifies GS-04. Preview and device output stay separate.

Editor UX and stability update (2026-10-28)
Pan is now an icon in the exclusive tool rail (H). Fixed an out-of-range tool
hint read after Pan plus property refresh. Space-pan clears on focus loss and
the minimap stays usable. Snapping moved entirely to Align / Snap. The inspector
shows source descriptions and key controls first; opacity is directly editable.
Panel visibility now survives restart and mode changes.
New native sources: Aurora, Lava Lamp, Checkerboard and Kaleidoscope. Schema 19;
update editor and host together. See docs/EDITOR_UX_REVIEW.md in the source tree.

Authoring update (2026-10-28)
Hex Return commits survive live preview refreshes. Hand (H) enables left-drag
canvas pan. Source menus separate organic/noise and scenes. Plasma and Ripples
are new native sources. The inspector replacement selector is collapsed/labeled.
Radar speed describes clockwise/counterclockwise revolutions; Voronoi speed is
cell movement (-30..30). Fireplace has soft footprint edges, detail 1..16 and
height 10..200. Files use schema 19; update editor and host together.

COLOR PICKER AND EXTENDED EFFECTS (2026-10-28)
Fixed the Hue track becoming cyan toward its upper end. The track remains a
rainbow and the selected hue approaches red. Picker numeric values omit units.
Added Voronoi (site count/distance/seed), Fireplace (detail/height/seed), Radar,
and Fireworks. Multizone creates four named, separately editable masked regions.
Choose a region in Navigator to edit its source/colors; the initial split is fixed.
Files use schema 19. Update editor and service host together; older files remain
readable. These are native core effects, not full upstream option replicas.

PATTERN CONTROL FOLLOW-UP (2026-10-28)
Effect controls use plain numbers and omit range rows. Pattern rate: Pinwheel
-60..60; Spiral/Visor/Sine Wave/DNA -30..30; Pixel Fill -20..20. Repetitions now
has a slider. Existing out-of-range project values remain intact until edited.
Sine Wave and DNA palettes close smoothly. New native patterns start as separate
layers; existing hierarchy is preserved. Rotation visibility has native regression
coverage, but the reported project-specific disappearance is not reproduced.

NATIVE PATTERNS (2026-10-28)
Added Gradient Pinwheel, Spiral, Visor, Sine Wave, DNA and Pixel Fill under
Gradients and patterns. Source contains pattern rate and effect-specific controls.
Negative rates reverse motion; zero freezes it. Pixel Fill offers Rows, Snake and
Spiral order. Projects now save as schema 19; schemas 1..18 remain readable.
Older editor/service builds cannot open newly saved files; update both together.

EDITOR SELECTION AND TRANSPORT (2026-10-28)
Layers canvas selection respects the active Navigator layer, including overlapping
objects and marquee selection. Labels sits beside fullscreen in the Components
canvas header and in the Layers component composite; both use the same preference.
Hold L for temporary captions. Preview uses one green Play / red Stop icon button;
Stop freezes the animation time without turning hardware off.

EDITOR FULLSCREEN FIX (2026-10-28)
Components fullscreen keeps a compact playback/status header and fills the rest of
the monitor with the canvas. The fullscreen button stays anchored when scrollbars
appear. EDIT MODE highlights the Components / Layers selection with labeled icons.

WINDOWS SERVICE OPTION (2026-10-28)
Run installer/Install.cmd from the complete extracted package for Windows startup
playback. Installation requests elevation once; the service owns hardware and the
editor/tray run with ordinary rights. Scans need no new UAC. Closing the installed
editor keeps the last confirmed project running. Stop all output is explicit.
The installer reuses PawnIO or downloads/verifies its official installer if absent.
See docs/WINDOWS_SERVICE.md in the source repository for updates/removal, security
boundaries and remaining physical/clean-install validation. Real SYSTEM output and
a clean-install UAC count are not established by software tests. Without service
installation, this archive retains the direct portable behavior below.

MSI Mystic Light - Windows x64

Run msi_mystic_light.exe from this complete folder. Keep hidapi.dll, Qt DLLs,
platforms, imageformats, modules, profiles, components and third_party together.
Do not copy only the EXE. Microsoft Visual C++ x64 runtime must be installed.

Canvas output starts automatically for configured, detected hardware. Editor opens with Components selected and automatic header discovery. Attach fans/strips to a
header using Hardware, then arrange its group or individual components.
New layer creates an empty transparent layer for editable objects. Standard drawing tools are on the left;
Shapes and Particles have toolbar menus; gradients use the gradient tool. Gradients use draggable stops and RGB sliders.
Gradient arrows have draggable endpoints. Layout has a live effect background,
group frames and resize/rotation handles. View offers an optional split hardware preview.
Roccat Vulcan/Kone, Razer Basilisk V3 and Apex 3 are detected by the peripheral
module. Previous hardware layouts can be imported; old custom-effect recipes
are preserved in their original settings file and are not converted. Components and Layers are
selection targets in one Editor workspace. Stop leaves the last hardware state. Device behavior/calibration has
not been physically qualified. Automatic output/reconnect is enabled until Stop output is used.

Source and build instructions: https://github.com/Delcado19/msi-mystic-light
See LICENSE and THIRD_PARTY_NOTICES.md for licenses and replacement instructions.

Drawing and hardware update:
Shapes and Particles are toolbar menus. Gradient shape fills preserve geometry; G creates new gradient objects.
Enable color animation starts playback; Apply motion to all visible layers is an explicit action. Hardware shares Layout's selectable
header/component overview with Add/Remove. The live preview shows software sampling;
Configured, present headers receive those colors automatically.

Travelling gradients now close the last color back to the first in gradient-local
coordinates; there is no rectangular wrap seam. Starting an animated output
resumes paused playback. The output status tooltip includes the stop/fault reason.
Blend modes affect the selected source against visible layers below it; a lone
bottom layer has no backdrop to blend with.

Group labels in Layout are draggable: drag a header label to move its complete
chain. Hardware has Stop output and Resume output; no report checkbox is required.
In portable mode, Tray Stop cancels queued restarts and automatic connection retries. Hardware Stop
pauses the selected output; other headers keep running. Select a controller to
pause/resume its configured outputs together.

Select a component in Layout or Hardware, then Edit LED arrangement. Arrange LEDs
offers Line/Circle/Ellipse/Rectangle; drag individual LEDs to refine. Only that
device changes; physical LED count and wire order remain intact. Undo is supported.

Layout sends live colors while components are dragged. Click a component to select
it individually, or drag a group label to move its chain. Hardware has a flat
Endpoint / Profile / State list, alongside the header/component inventory; Green Play / red Stop preview controls animation playback.

Restored output paths: IOShield Canvas as a shared color zone and RTX 5080 AERO OC
through NVAPI. RAM uses the separately installed official PawnIO 2.x runtime
(https://pawnio.eu). The signed AMD module is included under third_party/pawnio.
For portable direct output, run Lighting Studio as administrator and use Hardware > Scan hardware. Installed service mode keeps the editor unelevated. Supported: AMD 1022:790b primary bus,
ENE AUDA0-E6K5-0101, eight LEDs at 0x71/0x73. No OpenRGB/helper process is needed.
--scan skips active RAM probing; GUI discovery reads ENE identity registers.
The user confirmed physical RAM Canvas output with the diagnostic build.

Canvas controls: Green Play / red Stop preview controls animation time, separate from Devices output status.
Use the Layout canvas top-right fullscreen button; Escape returns to the workspace.
Effects stays in its workspace with its tools and layers.
Layout expands with its components. Use scrollbars or the bottom-right minimap to navigate.
The Effects workspace keeps its separate 640x400 effect raster.

Snap to grid is enabled by default. Toggle it in the toolbar and choose 1-100 px
spacing. Hold Alt for free placement, or use Ctrl+Shift+G (also in fullscreen).
Components/groups, effect layers, drawing/gradient endpoints, resize handles and
individual LEDs snap in world coordinates; rotation uses 15-degree increments.
Gradient palette stops use percentage increments (10 = 10%). Freehand brushes,
camera navigation and exact numeric entries stay continuous. Existing projects
are never resnapped simply by opening them. The grid is not sent to hardware.

Workspace navigation update:
Canvas headers and fullscreen playback controls have a protected margin. Scrollbars
and the lower-right minimap appear when content overflows, including after zoom.
Layout drag scrolling accelerates near all four edges. Corner resize anchors the
opposite corner. Hardware has manufacturer/category filters and manufacturer names;
strips are straight and D30 fans have a circular visual layout. Save as and the
current filename are visible beside Open/Save. Save adopts a new path only after
successful writing. The NV5 MKII built-in strip count is not yet verified.

MSI fan gradient correction:
Shared IOShield/native board configuration is sent before addressable fan colors.
Tests preserve distinct LED RGB values through the actual module serializer.
User confirmed individual fan LED control with the configuration-first fix.

Layer editing update:
Eraser/mask edits keep Play preview available; playback resumes after a gesture
if it was running. Eyes toggle layer visibility (Space in the layer list).
Stroke size has a slider and numeric field. Delete removes selected layers or
Layout/Hardware components/groups, with Undo. Text fields keep normal Delete.

Preview navigation update:
Scrollbars use matching light handles and dark tracks in both orientations.
Drag the highlighted Effects divider or its right-hand up/down arrow to resize
the editor and live preview, or drag to either edge to hide one preview.
Drag the divider back to restore it. Save As uses a distinct floppy/pencil icon.
Layout fullscreen remains available.

Effects and Live Hardware use the shared complete-layout projection;
turn it off to display the complete layout extent and aspect ratio, with
independent fit zoom. Existing layer/mask data and hardware definitions are preserved.

Mask inspection keeps animation running and selects the painting tool. Black hides
and white reveals; Stroke adjusts brush size. The minimap stays anchored when
scrollbars appear. Cosmetic Black/White product variants are merged only when
LED geometry and wiring match; existing project IDs are retained.

Detailed technical hardware drawings appear in the picker, Layout, Live Hardware
and LED editor. Fans have square housings; circles and wire numbers remain
editable. Artwork credits: Help > Artwork credits and THIRD_PARTY_NOTICES.md.
Qt6Svg.dll is required and included in the portable package.

Artwork follows custom geometry: linear fan instances display as strips, imported
mouse orientation is respected, and per-key keyboards use actual key positions.
Selection/group frames include the housing; optional small component labels appear below it.

Components have an optional small model label at a fixed lower-left anchor; instance numbers appear only
for repeated models. Fullsize keyboards share the Vulcan-derived drawing.
Dedicated peripherals appear as devices rather than single-component chains.
New placements start at Rotation=0; saved rotations remain editable.

Device settings for keyboards, mice and other built-in lighting outputs do not
offer component-chain editing. Append/Remove/reordering remain available only
for attachable controller headers. Device output/clock settings remain editable.

Gaming workspace: graphite surfaces and consistent red interaction accents.
Use Panels or View to show/hide Navigator and Inspector. Ctrl+Shift+L / Ctrl+Shift+P
are the panel shortcuts. The Inspector collapses once below 1180 logical pixels;
you can reopen it manually. Reset workspace restores both panels.
Preview and Devices chips stay visible in every workspace.
Live Hardware shows the complete layout. The redundant Focus devices control is removed.
Source / Motion / Mask and the Editing badge identify what you are editing.
Move up/down or Ctrl+PageUp/PageDown reorders layers with undo support.


Labels (eye icon in Components and Layers canvas headers, also under View) defaults off and remembers your
choice. Hold L to reveal component labels temporarily, including Layout fullscreen.
Releasing L or switching windows/workspaces or entering a text field clears the temporary reveal. Text/numeric
input keeps L, and I selects the line tool. Labels never relocate or grow on
selection. Hover tooltips and the inspector retain full model names. Group captions
remain visible for chain dragging. Label visibility does not change your project.

Lighting Studio UI audit implementation:
New layer creates an empty layer; toolbar flyouts add objects; Source owns particles and source colors. Motion
changes one layer; Apply motion to all visible layers is an explicit undoable action.
Displayed values use integers (ms, cycles/min, px, percent), preserving exact saved
values until edited. An approximation mark identifies an off-grid saved value; hover
shows its exact value. Alt+R resets a numeric slider field to its default.
The shared HSV color popover previews live; Apply commits and Cancel/Escape restores.
One color session is one undo action. RGB details and screen eyedropper remain available.
Stop snapping sets the palette snap spacing; 1 divides the bar into 100 steps. It applies while Snap to grid is enabled; Alt bypasses it.
View > Reduce UI motion controls optional popover/chevron feedback independently
from lighting playback. The toolbar uses the compact LS identity; About retains the conceptual banner.

Startup termination waits for asynchronous process exit after force-stop; the first
click covers both stopping and exit verification. Restarted processes require an
explicit new action. Timeouts and scan failures stay visible. The screen eyedropper
uses one capture window per monitor to preserve DPI and editor/Inspector geometry.

Editor combines component arrangement and layered effects. Choose Components or Layers as the editing target; Composite / Effect only / Components only control visibility. G creates a new gradient; grips edit existing gradients. Shift-click/marquee select, Align arranges, Ctrl+D duplicates. Hold L reveals labels; Line uses I. View offers Lock/Unlock and optional split preview. Editing locks remain supported. Schema 19 adds extended effects; older application versions cannot read newly saved projects. The Windows service option is described above.
