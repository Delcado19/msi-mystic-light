MSI Mystic Light - Windows x64

Run msi_mystic_light.exe from this complete folder. Keep hidapi.dll, Qt DLLs,
platforms, imageformats, modules, profiles, components and third_party together.
Do not copy only the EXE. Microsoft Visual C++ x64 runtime must be installed.

Canvas output starts automatically for configured, detected hardware. Layout opens with automatic header discovery. Attach fans/strips to a
header using Hardware, then arrange its group or individual components.
New layer is a blank paint layer. Standard drawing tools are on the left;
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
Endpoint / Profile / State list, alongside the header/component inventory; Canvas Start/Stop controls animation playback.

Restored output paths: IOShield Canvas as a shared color zone and RTX 5080 AERO OC
through NVAPI. RAM uses the separately installed official PawnIO 2.x runtime
(https://pawnio.eu). The signed AMD module is included under third_party/pawnio.
After installation, run Lighting Studio as administrator and use Hardware > Refresh. Supported: AMD 1022:790b primary bus,
ENE AUDA0-E6K5-0101, eight LEDs at 0x71/0x73. No OpenRGB/helper process is needed.
--scan skips active RAM probing; GUI discovery reads ENE identity registers.
The user confirmed physical RAM Canvas output with the diagnostic build.

Canvas controls: Canvas Start/Stop is green when stopped and red while running.
Use the canvas top-right fullscreen button; Escape returns to the workspace.
Layout expands with its components. Use scrollbars or the bottom-right minimap to navigate.
The Effects workspace keeps its separate 640x400 effect raster.

Snap to grid is enabled by default. Toggle it in the toolbar and choose 1-100 px
spacing. Hold Alt for free placement, or use Ctrl+Shift+G (also in fullscreen).
Components/groups, effect layers, drawing/gradient endpoints, resize handles and
individual LEDs snap in world coordinates; rotation uses 15-degree increments.
Gradient palette stops use percentage increments (10 = 10%). Freehand brushes,
camera navigation and exact numeric entries stay continuous. Existing projects
are never resnapped simply by opening them. The grid is not sent to hardware.
