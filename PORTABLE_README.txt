MSI Mystic Light - Windows x64

Run msi_mystic_light.exe from this complete folder. Keep hidapi.dll, Qt DLLs,
platforms, imageformats, modules, profiles, components and third_party together.
Do not copy only the EXE. Microsoft Visual C++ x64 runtime must be installed.

Canvas output starts automatically for configured, detected hardware. Layout opens with automatic header discovery. Attach fans/strips to a
header using Hardware, then arrange its group or individual components.
New layer creates an empty transparent layer for editable objects. Standard drawing tools are on the left;
Shapes and Particles have toolbar menus; gradients use the gradient tool. Gradients use draggable stops and RGB sliders.
Gradient arrows have draggable endpoints. Layout has a live effect background,
group frames and resize/rotation handles. Effects includes a live hardware preview.
Roccat Vulcan/Kone, Razer Basilisk V3 and Apex 3 are detected by the peripheral
module. Previous hardware layouts can be imported; old custom-effect recipes
are preserved in their original settings file and are not converted. Layout and Effects are
separate workspaces in the same window. Stop leaves the last hardware state. Device behavior/calibration has
not been physically qualified. Automatic output/reconnect is enabled until Stop output is used.

Source and build instructions: https://github.com/Delcado19/msi-mystic-light
See LICENSE and THIRD_PARTY_NOTICES.md for licenses and replacement instructions.

Drawing and hardware update:
Shapes and Particles are toolbar menus. Gradient fills preserve painted coverage
and procedural shape geometry. Enable color animation starts playback; the motion
scope can apply edits to all visible layers. Hardware shares Layout's selectable
header/component overview with Add/Remove. The live preview shows software sampling;
Configured, present headers receive those colors automatically.

Travelling gradients now close the last color back to the first in gradient-local
coordinates; there is no rectangular wrap seam. Starting an animated output
resumes paused playback. The output status tooltip includes the stop/fault reason.
Blend modes affect the selected source against visible layers below it; a lone
bottom layer has no backdrop to blend with.

Group labels in Layout are draggable: drag a header label to move its complete
chain. Hardware has Stop output and Resume output; no report checkbox is required.
Tray Stop cancels queued restarts and automatic connection retries. Hardware Stop
pauses the selected output; other headers keep running. Select a controller to
pause/resume its configured outputs together.

Select a component in Layout or Hardware, then Edit LED arrangement. Arrange LEDs
offers Line/Circle/Ellipse/Rectangle; drag individual LEDs to refine. Only that
device changes; physical LED count and wire order remain intact. Undo is supported.

Layout sends live colors while components are dragged. Click a component to select
it individually, or drag a group label to move its chain. Hardware has a flat
Endpoint / Profile / State list, alongside the header/component inventory; Play preview / Pause preview controls animation playback.

Restored output paths: IOShield Canvas as a shared color zone and RTX 5080 AERO OC
through NVAPI. RAM uses the separately installed official PawnIO 2.x runtime
(https://pawnio.eu). The signed AMD module is included under third_party/pawnio.
After installation, run Lighting Studio as administrator and use Hardware > Refresh. Supported: AMD 1022:790b primary bus,
ENE AUDA0-E6K5-0101, eight LEDs at 0x71/0x73. No OpenRGB/helper process is needed.
--scan skips active RAM probing; GUI discovery reads ENE identity registers.
The user confirmed physical RAM Canvas output with the diagnostic build.

Canvas controls: Play preview / Pause preview controls animation time, separate from Devices output status.
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

Gaming workspace: graphite surfaces and consistent cyan interaction accents.
Use Panels or View to show/hide Navigator and Inspector. Ctrl+Shift+L / Ctrl+Shift+P
are the panel shortcuts. The Inspector collapses once below 1180 logical pixels;
you can reopen it manually. Reset workspace restores both panels.
Preview and Devices chips stay visible in every workspace.
Live Hardware shows the complete layout. The redundant Focus devices control is removed.
Source / Motion / Mask and the Editing badge identify what you are editing.
Move up/down or Ctrl+PageUp/PageDown reorders layers with undo support.


Show labels (eye icon in Layout, also under View) defaults off and remembers your
choice. Hold H to reveal component labels temporarily, including Layout fullscreen.
Releasing H or switching windows/workspaces or entering a text field clears the temporary reveal. Text/numeric
input keeps H, and L remains the line tool. Labels never relocate or grow on
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
Palette Stop snap uses percent independently from the spatial pixel grid.
View > Reduce UI motion controls optional popover/chevron feedback independently
from lighting playback. The toolbar uses the compact LS identity; About retains the conceptual banner.
