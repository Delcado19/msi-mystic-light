# Lighting editor

## Visual workspace

The app opens on Layout. Read-only discovery lists all supported headers in the
left tree. Hardware is a separate workspace containing attachment, component definitions and
the actual output controls. Its product picker has categories, quantities and header selection. It restores the original studio's product-oriented workflow,
including the M25 models, generic fans/strips and validated installed component
metadata. Browsing works offline; adding requires an available or saved header.
Hardware also shows the same header/component tree and selection as Layout.
Select a header or component, then Add or Remove; both views update together.
Configured, detected hardware receives the canvas automatically.

Layout edits physical component placement. Effects is a separate authoring canvas
with a live hardware preview below it using the same 640 by 400 coordinate space. The splitter
resizes these two views. The hardware pane shows sampled component colors, including
layer channel targets and channel clocks; it is a software preview, not device readback.
Native firmware preview uses the configured first color, not a firmware simulation.

New layer immediately creates a transparent Paint layer with a white mask. There is
no effect-selection dialog. Shapes and Particles have direct toolbar flyout menus;
The redundant Other effects button/gallery is removed. Gradients are drawn with
the gradient tool; Shapes and Particles retain their toolbar menus. Import image
handles image files. Existing procedural effect layers still load and render.
Particle motion patterns remain editable in the contextual inspector. Layers show source and mask thumbnails, effect
type, visibility and name. Drag to reorder; double-click to rename.

The left toolbar contains Move (V), Pencil (B), Line (L), Rectangle (U), Ellipse (O),
Gradient (G), Fill (F), Eraser (E), and Crop (C). Shapes support filled/outlined drawing.
Tool options show only applicable color, stroke and shape controls. A stroke or shape
commits one pixel edit on release; Escape discards the draft. Fill floods connected
pixels of the same color while preserving antialiased coverage, and erasing removes source alpha. On a procedural/image source, Eraser selects
the existing mask and hides coverage instead of flattening the source. Drawing on a procedural
source creates a separate paint layer instead of flattening the source.

Gradient draws a linear gradient along a drag. On an empty paint layer it turns that
layer into an editable gradient. On painted coverage it recolors existing pixels
while preserving alpha. On a procedural shape it changes the fill without changing
the shape geometry; its arrow endpoints remain independently editable. Its inspector offers draggable color points, Add/Remove for the selected stop,
RGB sliders, a color picker and an optional exact hex value. Stop drags are one undo
step, and Add samples the existing gradient at the insertion position.

Click a layer's mask thumbnail (or the Mask inspector tab), then use the same drawing
tools on coverage. Mask Pencil honors hardness and opacity; Eraser hides coverage.
The Gradient tool draws black-to-white mask coverage. White reveals, black hides,
and gray supports smooth partial transparency. Pixel brushes and mask strokes remain
independent of procedural source parameters.

Preview starts playing automatically and can be paused. Source gestures temporarily
pause/reset to base time, then resume previous playback after commit/cancel; mask
editing stays at base time. Preview time never changes the saved document. It does
not change hardware assignments. Configured hardware receives the current composition automatically. The effect, motion and mask inspectors expose the
current task; numeric transforms and additional motion parameters are disclosures.
Particle motion patterns share Count, Speed and Radius sliders; seed and spatial
motion remain in More animation settings, with sliders beside every number.
Enable color animation starts palette cycling and playback, including painted sources.
Apply animation to all visible layers applies subsequent motion edits to the full visible
stack; otherwise only the selected layer changes. No decorative animation delays tools or shortcuts.

Output: Stopped/Connecting/Running/Faulted is a read-only status beside Undo/Redo. Use Hardware to configure outputs. Its tooltip
includes the current output status/reason; the Hardware status follows actual state.
Changing workspace does not stop output; use Stop output. Output starts automatically for configured, detected hardware; there is no report
approval checkbox. Tray Stop disables retries until Resume or the next app launch. Starting an animated output resumes
a paused preview so devices receive moving frames. Static output does not force
playback. See [output](OUTPUT_SESSION.md).

Launch `build/ninja/Release/msi_mystic_light.exe --editor` to skip the startup process-conflict dialog.
Normal startup runs the existing process-conflict dialog first. The application connects configured hardware automatically after startup. Layout and Effects are mutually
exclusive tabs in one Lighting category, with independent selection/zoom/pan.

## Layout

Select a header and open Hardware. Search/filter product cards, choose quantity,
and add them to that header's physical chain. Generic products allow the actual
addressable LED count to be set; named product mappings retain their count and
wire permutation. The original M25 pairing uses six addresses for twelve physical
LEDs. Imported dedicated-controller products (RAM/GPU/motherboard) are excluded
from the ARGB attachment catalog. Custom document components remain available.

Installed metadata is read from the newest installed VortxEngine component folder,
with bounded reads and topology validation. No upstream scripts, effects, drivers,
artwork or network resources are loaded. Imported geometry is normalized to layout
units without changing LED order. Only a selected product is embedded in the project.

Each accepted quantity is one validated undo transaction. Header capacity and fixed
counts are checked in the picker and by the document. Header settings reorders a
mixed physical chain and edits native mode/channel clocks. Cancel changes nothing.

Header groups and their individual components appear in the layout tree. Select a
header to drag or nudge its complete chain; select a child to move one component.
Dashed group boundaries remain visible. Drag the visible header label to move the
whole group, even if an individual component was selected. Drag the selected frame corners to resize
and its round handle to rotate the complete selection, with one undo step.
The live effect raster stays visible behind the geometry; only placements are edited.
Arrow keys move one unit, Shift+arrows ten. Removing a component removes it from
its chain too; removing the last member removes an empty software assignment and
its explicit layer targets atomically. Wire indices stay in the component geometry editor to keep the normal layout uncluttered. Wheel
zooms; explicit Zoom in/out and Fit controls also exist; middle drag pans. Effects and masks are authored exclusively in Effects.

Discovery reads descriptors and OS board identity without opening HID. Refresh
headers repeats discovery and deduplicates profile/endpoint/header matches. Saved
chains stay visible as offline groups when their controller is absent. Merely
selecting or attaching components never starts output. Hotplug discovery is manual;
a running output session still monitors endpoint presence independently.

## Layers and masks

New layer creates a transparent paint source with an independent white grayscale mask.
The top panel row is the top composited layer. Rename by editing the name,
toggle visibility, drag to reorder, duplicate or delete. Effect/mask thumbnails
select the target. Drawing tools operate on the selected source or mask.
Changing effect types preserves ID, mask, geometry, settings and embedded pixels.

Transparency has its own slider: 0% fully visible, 100% fully transparent.
Blend modes are Normal, Multiply, Screen and Overlay against the layers below.
RGBA is preserved, including a lone Multiply layer over transparent pixels.
Mask coverage multiplies source alpha and inverse transparency before blending.
Qt's encoded-color raster composition is used; Photoshop pixel parity and
calibrated LED appearance are not claimed.

Mask white reveals, black hides and gray creates soft partial transparency.
Choose brush or gradient. Brush diameter, hardness, opacity and gray level are
editable. A gradient runs black-to-white along the drag. Fill white/black,
invert, bypass and grayscale preview are available. Escape or switching tabs
cancels an unfinished gesture. Each completed gesture is one undo step.

Masks default to canvas coordinates. Link mask to effect makes geometry edits
move/rotate/scale the mask, and animation follows drift/spin/zoom/warp. Geometry
edits resample the linked mask; undo restores the prior pixels. Spatial motion
is evaluated from original stored pixels during preview. For predictable brush
coordinates, Mask editing pauses/resets time to zero and edits the base mask with
the selected layer's warp/link projection suspended in that view. Leaving mask
editing restores the normal linked projection. No mask data is discarded.

## Effects and direct tools

Available sources: Static Color; Linear, Radial and Conical Gradient; Stripes;
Waves; Circle, Ring, Star, Heart, Triangle, Hexagon, Diamond, Cross, Moon;
Image; Particle Rain, Particle Orbit, Fireflies and Particle Burst.

Gradients use 2..16 movable color stops, interpolated in premultiplied space
to avoid transparent-stop halos. Stripes alternate colors and Waves interpolate
sinusoidally; repetitions are editable. Conical gradients start along positive
X and progress clockwise, with a seam. Stationary linear/radial gradients clamp; travelling gradients close their palette
back to the first color and advance in gradient-local coordinates. They never tile
a finite canvas raster, which would introduce a hard rectangular seam. Patterns
repeat. Shapes use antialiased fixed templates (five-point star, fixed ring
thickness and fixed moon cutout).

Select Move, then drag inside the selected source's yellow frame to
move it. Four corner grips resize about its center; the external circular grip
rotates. Direct gestures pause/reset the preview clock before editing. Numeric
center, width, height and rotation provide keyboard-accessible alternatives.
Positive rotation is clockwise. Static Color covers the canvas and has no
spatial handles. Effect geometry does not move Layout components. Escape/tab
switch/history changes cancel unfinished transforms; undo restores committed
transforms. Gradient geometry controls its interpolation domain, while shapes
and images are transparent outside their geometry.

## Images and crop

Import image creates a new Image layer from PNG, JPEG, WebP, GIF or BMP. JPEG
orientation is applied. Animated GIF/WebP inputs import their first frame only;
image animation/video decoding is not part of the authoring feature set.
Decoded pixels are embedded losslessly, so source removal does not affect a
project. An empty Image layer is transparent.

Fit whole image preserves aspect ratio with transparent margins. Fill and crop
preserves aspect ratio and clips opposite edges equally. Enable Crop image in
the left toolbar and drag a rectangle over the visible image to select a new
source region; drag again to refine it. The normalized left/top/width/height
fields provide precise crop coordinates. Reset image crop restores the whole
source. All crop operations retain original pixels, and undo restores previous
selections. Select Move to leave the crop tool and restore transform grips.

## Motion and color

Every layer exposes applicable animation parameters:

- Spin: -360..360 degrees/second; procedural hue rotation uses the same range.
- Pulse: 0..10 Hz, cosine alpha modulation, initially fully visible.
- Breathe: 0..10 Hz, smooth exponential alpha modulation, initially fully visible.
- Drift X/Y: -500..500 pixels/second; content can move off the canvas.
- Zoom: 0..90 percent sinusoidal size amplitude at 1 Hz around the source center.
- Warp: 0..100 pixels of horizontal sine displacement, 100-pixel wavelength,
  travelling at one cycle/second; pixels outside the canvas are clipped.
- Trails: 0..2 seconds, six decreasing-opacity historical samples plus the
  current source. Samples before time zero are skipped. Fixed temporal samples
  make seeking reproducible and bound rendering cost.
- Color cycles: 2..16 comma-separated QColor colors (`#RRGGBB` or `#AARRGGBB`),
  with 0..10 full cycles/second; the last color interpolates back to the first.
  They animate the primary procedural color, then hue rotation is applied.

Zero disables each modifier. Image hue rotation is supported per pixel;
achromatic pixels remain achromatic. Procedural color cycles are disabled for
images. Spin is disabled for full-canvas Static Color. Pulse and breathe
multiply each other when both enabled. All modulation happens before masks,
transparency and layer blending. Source trails use the current mask projection.

The Preview toolbar starts paused. Enter a time or enable Play preview. Pause
retains time; enter zero to reset. The 0..86400-second range stops at its end.
Elapsed wall time controls playback, so dropped frames do not accumulate drift.
The timer targets about 30 fps and shares each composed frame between canvases;
actual speed depends on source count, image grading, warp and trail samples.
Time and playback do not dirty the document or add undo entries. Motion is
explicitly started, with no automatic UI decoration or motion on shortcuts.

## Seeded particles and performance

Four patterns share count (1..400), seed (0..1000000), speed (0..500) and radius
(1..40 pixels), plus primary/secondary colors, geometry and all motion/mask/layer
controls. Rain wraps downward; Orbit uses seeded radii/angular velocities;
Fireflies use smooth intersecting sine paths; Burst repeats radial expansion.
The speed value advances normalized path phase by speed/200 cycles per second.
The seed and absolute time determine every frame, independently of playback
history. Changing a seed changes the pattern. Particles have no hardware I/O.

The test suite logs the mean composed frame time for a 400-particle layer with
seven trail samples. This is a local performance measurement, not a 30 fps
promise for 64 simultaneously animated layers. Playback skips stale time steps
rather than queueing old frames. See CONTINUE.md for the latest measured result.

## Project files and boundaries

Save/Open uses `.lighting.json` schema 12, including placements, ordered layer
IDs, source/animation parameters, embedded PNG masks/images, normalized crop,
linked masks and color cycles. Stable parameter IDs drive UI ranges, validation
and advanced persistence. Schemas 1..11 remain readable with their original
semantics; missing newer settings get disabled/default values. Saves always use
schema 12; older versions reject it instead of silently losing effects.

QSaveFile replaces files atomically. Load parses/validates the entire candidate
before replacing the document/history. Unknown component IDs, unsupported effect
types, corrupt rasters, invalid crop/colors or out-of-range values preserve the
live document. Invalid typed UI values are reported in the status bar and
restored, without exceptions escaping Qt event handlers.

Limits: fixed 640x400 canvas, 64 layers, 256 placements, 40 undo steps, 32 MiB
project read/write, 16 MiB image input, 4096 pixels per image side, and total
16 * 1024 * 1024 decoded image pixels (including images retained after effect
type conversion). Dimensions and accumulated pixels are checked before decode.
An oversized save is rejected before replacing its destination. Undo snapshots
share raster storage until edited. Save/Discard/Cancel protects unsaved changes.
Schema 7 embeds document-owned component definitions, so changing the installed
catalog cannot change a saved layout. Older schemas still resolve geometry from
the catalog loaded at startup. Viewports/time/selection are session context.

The accepted image/shape/gradient/motion/color/particle authoring increment is
implemented. Component geometry creation/edit/import, placement transforms and
component sampling, staged channel assignments and native-mode settings are also
implemented. Separate future work includes per-layer channel targets, reconnect
handling, scheduling, hardware output, autosave and automatic session restoration. Keyframes, multistop gradient
editing and animated image playback are not claimed by this increment.

## Verification

Qt tests exercise composition, soft masks, independent copies, layer operations,
separate view contexts, persistence/migrations, malformed input rejection,
image codecs, deterministic animation/particles, linked masks, canvas move/
resize/rotate/crop, gesture cancellation, UI validation and undo. Native Windows
checks complement offscreen tests, which may lack sandbox fonts. Screenshots
are written to the build's editor-artifacts directory.

## Component sampling

`LightingDocument::sampleComponents(frame)` accepts one canvas-sized composed frame
and returns RGB values keyed by placement UUID, in each component's local wire
order. Repeated instances remain independent. Layout drawing and sampling share
`componentPoints`: catalog coordinates use 20 canvas pixels per unit plus the
placement origin. Sampling uses the containing pixel (floor for nonnegative
coordinates); positions outside the canvas produce black. Premultiplied channels
composite transparent effects onto black before discarding alpha.

This is an offline backend contract, not channel assignment or device output.
Callers can reuse a single rendered frame for all components. Channel offsets,
calibration, scheduling and hardware transmission remain separate work.

## Layout transforms

Select a component in Layout to edit its X/Y position, independent X/Y scale
(1..10000 percent) and rotation (-360..360 degrees). Scaling happens in catalog
coordinates, followed by rotation around the placement origin and translation.
Drawing, hit testing and LED sampling use this same transform. Each numeric edit
is undoable; dragging and arrow keys retain the current scale/rotation.
Schema 6 stores these transforms. Schemas 1..5 load with 100 percent scale and
zero rotation; transform fields under older versions are rejected rather than
silently lost. Layout transforms never modify effect-layer geometry or masks.

## Component authoring and import

In Layout, **Create component** opens a geometry table with a stable ID, name,
LED count, X/Y coordinates and wire indices. **Edit component geometry** edits
the definition selected in the catalog picker, affecting every instance with
that ID in this document. Existing IDs stay fixed so placements remain valid.
New rows default to a straight strip with sequential wiring. Each wire index
from 0 to LED count - 1 must appear exactly once; coordinates must be finite.
Invalid cells keep the dialog open and show the validation error. Cancel leaves
the document untouched. Accepted changes are one undo step.

**Import component JSON** uses the same schema-1 parser as startup catalogs,
with a 1 MiB file limit. Duplicate IDs are rejected rather than replaced.
Create/import adds a definition to the picker; **Add component** places an
instance in Layout. Definitions and their edits participate in project undo,
dirty state and saves, including definitions not currently placed. Limits are
256 definitions per document and 65535 LEDs per definition, within the existing
32 MiB project limit. Schema 7 embeds definitions and rejects duplicate or
malformed embedded IDs before replacing live state/history. Schemas 1..6 remain
readable using the original startup catalog. Embedded geometry under an older
schema is rejected rather than silently discarded. This editor changes local
data only; it does not download a repository or configure attached hardware.

## Staged channel configuration

In Layout, scan supported devices, explicitly choose a profile/endpoint and one
of its channels, then **Configure channel**. The dialog selects **Native firmware**
or **Software canvas**, plus an ordered component chain. Append existing Layout
instances and reorder them by dragging or the Up/Down buttons. Each instance may
belong to only one channel; repeated products require separate Layout instances.
A software channel needs Direct Mode support and a nonempty chain. All chains
respect maximum capacity and any fixed LED count. Native channels may omit
geometry. Unassign a component before deleting it; invalid geometry changes that
would exceed an assigned channel's capacity are rejected without altering undo.

Native settings include a profile-supported mode, 1..4 opaque RGB colors,
brightness 0..5, speed 0..2 and direction, matching the current driver contract.
Native settings are disabled in Software mode and retained when switching back.
Selecting a candidate alone changes nothing. Accepted assignments and loaded
projects automatically update connected hardware, including configured firmware modes.
The assignments list can edit saved entries even when the endpoint is absent;
remove an assignment to release its component instances. Accepted changes and
removal participate in undo/redo and project dirty state.

Schema 8 stores assignment UUIDs, exact endpoint paths, profile/channel IDs,
explicit modes, native parameters and placement UUIDs in physical chain order.
Device profiles must still be installed and valid; unavailable profiles reject
load atomically. Schemas 1..7 migrate with no assignments. Assignment fields under
older schema numbers are rejected. At most 256 assignments are allowed, one per
endpoint/channel; one endpoint cannot use conflicting profiles. Stored endpoint
paths identify intended targets, not verified connections or permission to write.

`sampleChannels(frame, seconds)` returns software channel samples in physical wire
order. Default channels share the supplied composition; scoped layers or custom
clocks render a channel-specific composition before sampling.

## Channel targets and clocks (schema 9)

Each layer targets all software channels, or an explicit list of assignment UUIDs.
An empty explicit list targets none. Removing a channel or converting it to Native
removes its UUID without broadening a layer to all channels. Unknown/duplicate
UUIDs are rejected. Each software channel has rate 0..10 and offset 0..86400 seconds;
local time is clamped to 0..86400. Rate zero holds the offset frame. The preview
selector shows all layers or the selected channel's composition and local clock.
Geometry/mask authoring returns to the global base frame.
Schema 9 stores layer targets and channel clocks; schemas 1..8 migrate to all
channels and rate 1/offset 0. New fields under older schema numbers are rejected.

## Session and output

Dirty projects are atomically autosaved after a two-second debounce without
marking undo history clean. Startup offers recovery and restores the last project,
window and docks. Recovered documents remain unsaved. A single-instance lock
protects the session. The tray offers Show editor, Stop output and Quit; hiding
is explicit, and normal closing retains the unsaved-project prompt.

Configured hardware receives the canvas automatically. Effect/mask edits and undo
update software channels live; topology changes reconnect using validated endpoint
identities. Hardware provides Stop output and Resume output. See
[output session](OUTPUT_SESSION.md). Physical calibration remains unqualified.

## Colors, paint sources and gradient stops (schema 11)

Every color editor provides a picker and optional #RRGGBB value; invalid or
incomplete hex input leaves the color unchanged. Palettes use a color strip with
selection, RGB sliders and Add/Remove instead of per-color numeric rows. Gradient
strips also support dragging stop positions. Palette limits remain 2..16 colors for
gradients/cycles and 1..4 opaque colors for firmware.

Schema 11 adds gradientPositions and the Paint source type. Paint pixels are embedded
losslessly at canvas resolution, under the shared image budget. A null Paint image is
transparent. Schema 10 gradients keep evenly spaced colors when positions are absent;
schemas 1..9 retain their original two-color gradients. New fields under older schema
numbers are rejected rather than silently discarded. Schema 11 is not writable by
older application versions; keep a backup when editing shared projects.

Effect geometry and absolute-time drift wrap horizontally and vertically across
the 640x400 canvas. Intersecting neighboring source copies are drawn before
clipping, so partial shapes/images/particles reappear at the opposite edge.
Linked masks use periodic inverse sampling with bilinear coverage; fixed masks
stay in canvas coordinates. Wave distortion wraps its rows too. This is source
behavior shared by preview and software output, not a canvas-only visual trick.

## Restored hardware and drawing workflow (2026-10-23)

The hardware preview hides when no components are configured, returning that space
to the effect canvas.

The Hardware workspace owns the component library and the actual OutputPanel;
there is no permanent bottom output area in Layout/Effects and no detached output
window. The current effect remains the Layout background so sampling is visible.
The Gradient tool leaves an arrow with two draggable endpoints. Radial/conical
sources retain their type and use the center as the starting handle. Fill targets
connected paint pixels, matching transparent coverage and tolerating small edge
color differences; visited pixels keep similar replacement colors from looping.
On solid procedural shapes, Fill changes the source color directly. Other procedural
patterns and images use their source raster to find a connected region and place
the result on a new transparent paint layer; the original source stays editable. It no longer fills a blank new layer indiscriminately.
The foreground picker remains available while drawing over a gradient source.

The peripheral module restores the original pure protocol encoders for Kone Aimo,
Basilisk V3, Apex 3 and Vulcan 100/120. Discovery adds dedicated keyboard/mouse
geometry automatically, never fan/strip models. It does not open HID. Apex uses
output reports with brightness 100; Kone and Basilisk use feature reports. Vulcan
uses a uniquely matched control/LED pair of the same PID, requiring review of both
exact paths before opening either. Multiple identical Vulcans fail with an explicit
pairing error rather than choosing the first interface. API 2 metadata prevents
old hosts from loading the new output-report module; API 1 transport slots remain
unchanged for old feature-only modules. No reconnect or automatic output is added.

On a new session without a loaded project/assignments, the original settings file
at `%APPDATA%/MsiMysticLightApp/settings.json` can restore the active profile's
supported component chains and native commands. The import binds to fresh exact
endpoint discovery, preserves wire permutations, rescales the original 1100x700
sampling footprint to 640x400, validates atomically, and never modifies the source.
Existing assignments are kept; ambiguous/unavailable endpoints are reported.
The Hardware import action also supports explicit recovery into an existing setup.
**This is hardware/layout recovery, not an old custom-effect recipe converter.**
The import status explicitly asks for an effect before starting software output.
Old recipes remain available in their original file.

Reference: [Paint.NET gradient control nubs](https://mail.getpaint.net/doc/latest/GradientTool.html)
and [move/resize controls](https://mail.getpaint.net/doc/latest/MoveTools.html).

## Shape gradient fills (schema 12)

Schema 12 stores optional normalized shape-local gradient endpoints in gradientFill.
The shape kind, transform and mask remain independent. Older schema numbers reject
this field; schemas 1 through 11 without it still load. Solid Fill clears the gradient
fill on that shape. Moving an arrow endpoint never resizes the underlying shape.

The hardware preview shows the live composition beneath sampled LEDs and its current
clock. It is software feedback, not device readback. The same timed composition and
component sampling feed OutputSession. A recording-transport regression verifies
animated painted red/blue frames through the real MSI serializer for both JARGB1 and
JARGB2. Physical illumination must be observed on real hardware; a Stopped status sends nothing.

## Gradient travel and blend verification (2026-10-24)

Travelling linear/radial gradients evaluate the moving coordinates directly. Palette
stops reserve one final segment returning from the last color to the first, like the
original CustomGradientEffect paletteAt. A diagonal arrow retains its direction
throughout travel. Stationary gradients retain their endpoint-clamped appearance.
Tests cover horizontal pixel continuity across the complete diagonal canvas at
0, 0.25, 1 and 144.87 seconds, perpendicular-axis invariance and travel direction.

Blend verification now uses asymmetric RGB foreground/background values, 50%
transparency, a half-gray mask and the actual inspector selector. Normal, Multiply,
Screen and Overlay match their expected formulas. An isolated bottom layer has no
backdrop to blend with; the tooltip explains this. No compositor replacement was
needed. Live animated output is also tested through the UI timer, scheduler and
real MSI serializer for both JARGB1/JARGB2 from a paused start. A recording transport
confirms changing reports; actual device behavior remains a separate observation.

### Per-device LED arrangement

Select an individual component in Layout or Hardware and choose **Edit LED arrangement**.
The **Arrange LEDs** menu provides Line, Circle, Ellipse and Rectangle templates;
drag individual numbered LEDs to refine their positions, use the wheel to zoom,
and **Fit LEDs** to frame the result. **Reset arrangement** restores the geometry
from when the dialog opened. Cancel discards changes.

This action changes only the selected device. LED count and physical wire order
stay locked; sampling and hardware output use the new positions with the original
wire mapping. Apply is one undo step and the arrangement is embedded in saved
projects. Shared definition authoring remains under Hardware's custom component
definitions. Device geometry uses a private embedded definition and counts toward
the existing 256-definition project limit.

### Live layout movement and output selection

Layout retains the animated effect canvas behind the hardware. Dragging a component
samples its temporary position immediately, including during resize/rotation;
actual output receives those samples before mouse release. Escape restores original
sampling positions; release commits one undo step. Clicking a member of a selected
group selects that member. Drag the group label to move the complete chain.

Hardware keeps its component/header inventory on the left. The right side is a
flat **Endpoint / Profile / State** table, one row per endpoint/profile pair,
including saved offline endpoints and explicit NVAPI profile targets not yet
detected. HID paths and PCI endpoints remain technical identities;
there is no duplicate header tree. Full endpoint paths are available in tooltips.
Selecting a component/header on the left scopes Stop/Resume to its channel and
names it in the button. Clicking an endpoint row scopes the actions to that
endpoint's configured channels.

Select a channel to stop/resume only its submissions; select the controller to act
on its configured channels. Running channels disable Resume. Unconfigured channels
have both actions disabled. Paused channels hold their last applied device state;
firmware-native animations can continue independently. A currently executing report
may finish when Stop is pressed. Other channels keep their controller connection.
The endpoint table expands to use available space rather than a fixed 100-pixel box.

### Restored integrated lighting

IOShield uses the original Canvas-to-`0x50` shared-color fallback, without claiming
per-LED control. The qualified RTX 5080 AERO OC uses NVAPI. Detected devices are
added to Hardware/Layout even for older embedded catalogs. Qualified ENE RAM
uses the installed official PawnIO 2.x driver directly, approved by the user on
2026-10-24. Run the app as administrator and use Hardware > Refresh. Discovery
verifies firmware and eight LEDs at 0x71/0x73; detected modules receive their own
Hardware/Layout placements. No OpenRGB process or network helper is used.

### RAM component geometry correction

Component coordinates use 20 canvas pixels per unit. The XPG eight-LED line uses
0.9 units per LED (126 pixels total at 100% scale). Loading a project repairs the
exact initially shipped 18-unit spacing; custom arrangements and placement
transforms are preserved. This changes spatial sampling only, never wire order.

### Physical LEDs and reserved protocol slots

Optional component `nonPhysicalSlots` lists reserved wire indices. They remain in
fixed-length reports and the coordinate table, but are excluded from visual LEDs
and shape arrangement. Vulcan has 105 physical keys and 39 reserved slots in its
144-slot buffer; Esc (wire 0 at 0,0) remains physical. Old embedded Vulcan definitions
receive the same explicit slot list on load, including instance geometry.
The geometry editor uses a constant screen size for LED circles and centered wire
numbers. Fit/zoom changes their positions on screen without shrinking the markers
or changing sampling coordinates. This also handles near-equal axis coordinates
in circular fan arrangements.

LED arrangement opens in a larger window with 32-pixel markers and 12-pixel
semibold white wire labels on dark blue. Markers retain their screen size when
fitting the device; reduced outer padding gives the geometry more room.

The toolbar hides the numeric preview-time field; Play/Pause still drives the
shared animation/output clock. Move/resize/rotation freezes the current frame and
uses that frame's animated transform for hit testing, rather than jumping to time
zero. Creating a shape or particle clears crop mode so the new layer is immediately
movable, even after cropping an image.

### Canvas playback, fullscreen and growing Layout

The main toolbar has one Canvas Start/Stop button: green when stopped, red while
running. Stop freezes the shared preview/output time; it does not turn off LEDs.
The numeric seconds field and duplicate toolbar output status are hidden.

The top-right canvas button opens that same live canvas in a fullscreen window.
The fullscreen view has the same Start/Stop control. Escape or the canvas button
returns it to its original workspace without losing document edits.

Only Layout expands: its bounds include the initial 640x400 area and all physical
LED positions plus their marker margin, including negative coordinates. Bounds are
derived from placements on every draft/committed move, scale or rotation; undo and
project reload therefore restore them without a new file schema. Effects retain
the separate 640x400 raster. The composite effect image stretches across the Layout
world bounds; preview, component/channel sampling and real output share exactly
the same world-to-raster mapping. Expanding the bounds consequently changes other
components' sampled locations too. This does not allocate a larger effect image.

The bottom-right minimap shows the effect image, hardware locations in Layout and
the visible viewport. Click or drag inside it to navigate; wheel zoom and middle
mouse panning remain available. Fit canvas includes the expanded Layout bounds.

Horizontal and vertical canvas scrollbars appear as needed when the world exceeds
the visible area at the current zoom. They share the same pan position with the
minimap and middle-mouse navigation, including negative Layout coordinates and
transient hardware moves. The minimap sits above/left of any visible scrollbars
in the lower-right corner. Fit canvas hides scrollbars when everything fits.

Boundary validation: zoom ignores non-finite/non-positive factors; transformed
component coordinates and draft placements are validated before sampling. Finite
coordinates must leave arithmetic headroom for world spans and navigation zoom.
Scrollbar camera and thumb positions are synchronized at both endpoints; camera
clamping waits for the current drag/pan/minimap gesture to finish.
## Snap to grid

The Preview toolbar provides a persistent Snap to grid toggle (default enabled)
and 1–100 px world spacing. Ctrl+Shift+G works across fullscreen and LED dialogs;
Alt temporarily bypasses snapping during a gesture. The LED dialog shares the
same toggle/spacing; component units are converted through the Layout scale
(20 px per component unit), independently of the dialog's presentation scale.

Layout component/group drags use one placement anchor and one translation for
the entire selection. Effects snap their base centers; drawing, gradient and crop
endpoints and resize pointers use world grid points. Uniform Layout resizing
preserves proportions, and rotation uses 15-degree increments. Selected LED
markers retain their offsets while their shared anchor snaps. Palette stops use
percentage increments and keep existing stop-order constraints. Freehand pencil,
eraser, fill, camera panning/scrolling and numeric fields retain continuous input.

The grid is an editing overlay only: preview frames and hardware output do not
contain it. Grid changes cancel an in-progress canvas gesture. Project files and
saved geometry are never resnapped merely by opening them or enabling the grid.
Each completed canvas gesture remains one undo step. Layer-list ordering, window
splitters and other non-spatial controls do not use a coordinate grid.
