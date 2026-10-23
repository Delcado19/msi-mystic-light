# Offline lighting editor

Launch `build/ninja/Release/msi_mystic_light.exe --editor` for offline editing.
Normal startup runs the existing process-conflict dialog first. Neither editor
view opens devices or sends HID reports. Layout and Effects are mutually
exclusive tabs in one Lighting category, with independent selection/zoom/pan.

## Layout

Choose a component and Add component. Drag a LED point to move its instance;
arrow keys move one unit, Shift+arrows ten. Remove selected component removes
only that instance. Wire indices appear beside LEDs. Layout previews the
composition but cannot author effects/masks. Wheel zooms; middle drag pans.

Scan supported devices reads descriptors and OS board identity. All matching
profile/endpoint pairs remain selectable without auto-selection. Selection is
staged/session-only, without connection or channel assignment. Rescan clears
stale selections. Automatic unplug/reconnect monitoring is not implemented.

## Layers and masks

New layer creates the selected effect with an independent white grayscale mask.
The top panel row is the top composited layer. Rename by editing the name,
toggle visibility, drag to reorder, duplicate or delete. Effect/mask thumbnails
select the target; Edit effect and Edit mask are also in the left toolbar.
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
coordinates, Edit mask pauses/resets time to zero and edits the base mask with
the selected layer's warp/link projection suspended in that view. Leaving mask
editing restores the normal linked projection. No mask data is discarded.

## Effects and direct tools

Available sources: Static Color; Linear, Radial and Conical Gradient; Stripes;
Waves; Circle, Ring, Star, Heart, Triangle, Hexagon, Diamond, Cross, Moon;
Image; Particle Rain, Particle Orbit, Fireflies and Particle Burst.

Gradients use two colors including alpha, interpolated in premultiplied space
to avoid transparent-stop halos. Stripes alternate colors and Waves interpolate
sinusoidally; repetitions are editable. Conical gradients start along positive
X and progress clockwise, with a seam. Linear/radial gradients clamp; patterns
repeat. Shapes use antialiased fixed templates (five-point star, fixed ring
thickness and fixed moon cutout).

Select Edit effect, then drag inside the selected source's yellow frame to
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
selections. Select Edit effect to leave the crop tool and restore transform grips.

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

Save/Open uses `.lighting.json` schema 8, including placements, ordered layer
IDs, source/animation parameters, embedded PNG masks/images, normalized crop,
linked masks and color cycles. Stable parameter IDs drive UI ranges, validation
and advanced persistence. Schemas 1..7 remain readable with their original
semantics; missing newer settings get disabled/default values. Saves always use
schema 8; older versions reject it instead of silently losing effects.

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
Nothing is sent to hardware. Selecting a candidate, applying a dialog, loading a
project and playing the preview do not open a transport or change firmware modes.
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

`sampleChannels(frame)` returns only software assignments, keyed by assignment
UUID, using one supplied composed canvas. It concatenates each instance's local
wire-order RGB values in chain order; it does not apply the wire permutation a
second time. Native assignments are absent. This increment uses the shared full
composition for every software channel. Per-layer channel targeting, independent
channel timelines, transport connections, scheduling/calibration and output are
separate work; saving staged native settings does not run an effect on a device.
