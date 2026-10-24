MSI Mystic Light - Windows x64

Run msi_mystic_light.exe from this complete folder. Keep hidapi.dll, Qt DLLs,
platforms, imageformats, modules, profiles, components and third_party together.
Do not copy only the EXE. Microsoft Visual C++ x64 runtime must be installed.

Canvas output starts automatically for configured, detected hardware. Layout opens with automatic header discovery. Attach fans/strips to a
header using Hardware, then arrange its group or individual components.
New layer is a blank paint layer. Standard drawing tools are on the left;
Shapes and Particles have toolbar menus; Other effects contains gradients and patterns. Gradients use draggable stops and RGB sliders.
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
it individually, or drag a group label to move its chain. Hardware lists all
supported outputs under each controller; the toolbar output text is status only.
