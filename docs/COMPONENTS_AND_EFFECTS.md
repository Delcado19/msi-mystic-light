# Component chains, device regions, and effects

## Configure a motherboard header

Open **Hardware**, then select JARGB1, JARGB2, JARGB3, or EZ Connector. Use
**+ Add component** to search products and select a quantity. Components consume wire
addresses consecutively in list order. The header limit remains 120 slots;
an over-capacity or malformed replacement is rejected before changing output.
Each component card has its own **Remove** button. Removing the last entry explicitly
clears the Canvas output; it no longer restores a fictitious 120-LED chain.
Older profiles without explicit configuration retain their previous default.

The initial explicit templates are:

| Template | Addressable slots | Interpretation |
| --- | --- | --- |
| Fan / 12 LEDs, 6 paired regions | 6 | Two physical LEDs share each region; use only when this matches wiring |
| Fan / 12 independent LEDs | 12 | Twelve independent regions |
| Fan / 16 independent LEDs | 16 | Sixteen independent regions |
| Strip / 30 LEDs | 30 | Thirty independent regions |
| Strip / 60 LEDs | 60 | Sixty independent regions |

These five generic templates are always available. The hardware picker additionally
reads component JSON metadata from the newest installed SignalRGB version, if
present. This imports declared geometry and wire mappings, not detection results:
choose the actual product manually. Definitions with invalid address mappings or
more than 120 slots are skipped. No SignalRGB runtime, scripts or images execute.
**Manage definitions > New definition** saves a named Fan/Strip with 1-120 addressable
slots/groups and 1-16 physical LEDs per address. A 12-diode fan with six hardware
addresses uses 6 slots and 2 physical LEDs per address. Software grouping is
different: retain every wire address and group its sample positions in the editor.
Custom definitions can be removed from the library without removing configured
copies. **Import JSON** also accepts valid external definitions.
Exact fan models and physical wiring still need to be established.
Three paired-region fans consume 18 serial addresses, not 36 independent ones.
A parallel hub can mirror a single six-region signal across all fans; adding
more software components cannot make those shared data lines independent.
In that case configure one shared group. True serial chains consume consecutive
addresses and can be configured as multiple components.

The distinction is also documented by [SignalRGB's RGB versus ARGB guide](https://docs.signalrgb.com/troubleshooting/rgb-vs-argb/).
Whether this particular installation is serial, mirrored, or Gen2-managed still
requires a physical identification test; a connector shape is not sufficient.

## Spatial component and LED layout

Select an output in **Layout**. Drag it to move, drag its mint bottom-right
handle to resize, and drag its pink handle to rotate. Ctrl+wheel scales it about
its center. Width, height and rotation also have exact inspector fields. This
changes the effect footprint sampled by real LEDs. Existing 70x40 placements
are migrated to readable sizes for keyboards/mice only; custom sizes are retained.
Explicitly adding real components starts with a larger usable
footprint. Camera/window resizing does not alter the stored physical layout.

**Arrange components / LED groups** opens an editor with movable component frames,
resize/rotation handles, exact scale/angle controls, and individually draggable
fan/strip LED points. The wheel zooms the editor view; **Fit view** restores it.
Select a component and apply Line, Circle, Ellipse or Rectangle. **Reset layout**
restores its saved original LED geometry. Shapes cannot rearrange keyboard keys.
Ctrl-select/marquee several LEDs and **Group selected LEDs** to give those wire
addresses one sample position/color; moving coincident LEDs changes the group.
Reset separates them again. Save applies coordinates; Cancel leaves output intact.
Wire addresses/counts stay unchanged by geometric edits. Hover shows addresses.

The renderer and both preview views share `LayoutGeometry.h`: bounds, translation,
footprint scaling, center rotation and integer rounding use the same function.
Detected default registration also preserves saved geometry on non-selected
outputs during startup. `LedMapping` is now honored as logical region -> wire address, including imported
non-identity mappings. Independent effects keep their local coordinate convention
and scale/rotate with the footprint; GlobalFollow uses shared scene coordinates.
Approximate body silhouettes do not establish physical address correctness.

## Unit-specific keyboard calibration

OpenRGB is an initial reference, not authority over the actual keyboard. The
repeated Left Windows / Right Ctrl discrepancy remains physically unverified.
Normal rendering now keeps unused packet slots black instead of sampling (0,0).
That corrects a reference mismatch; it is not claimed as the proven cause.

Select Vulcan and **Calibrate keyboard wire addresses**. Choose Left Windows or
Right Ctrl, enable **Identify slot**, and step through addresses 0-143. Only the
selected slot receives white; all others receive black, including normally unused
slots. Find the slot that lights the chosen physical key, then Save. The mapping
swaps addresses to preserve a bijection, with fixed colors following their logical
keys. Close/Cancel restores lighting; no probe is saved as an effect. Calibrated
geometry/mapping persists per profile. Confirm on real hardware before claiming a
key is fixed; a unit test can only establish the outgoing address and byte colors.

## Keys and LED regions

Select a device in **Layout**, then open **Edit LED colors...**. The Vulcan starts with an ISO
reference matrix and wire IDs; unused packet slots are not drawn as keys. Key legends
follow German ISO positions. The Basilisk V3 exposes logo, wheel, and nine
strip regions; the Kone Aimo exposes wheel, four left-strip, four right-strip,
and two lower regions. Their zone identities follow the upstream OpenRGB
controller/protocol definitions already used by the drivers. Mouse silhouettes
and fan body dimensions are illustrative, not measured product drawings.
The Apex 3 exposes ten lighting zones, not individually addressable keys.

Hover shows the key/zone name and zero-based wire address. Click in Canvas mode
to assign a fixed color to that address. Fixed colors override the effect and
are saved per profile. **Clear fixed key / region colors** returns to the effect.
Changing a component chain clears old overrides because their address ownership
may have changed. Firmware-native effects, including I/O shield native mode,
do not support per-region editing through this interface.

**Identify clicked region** temporarily sends white at the clicked address and
black at other addresses in that channel. It does not change saved overrides.
Uncheck it, switch device/profile, hide the window, or quit to restore the saved
assignment. Other channels continue their existing output. A doubled physical
LED pair should illuminate together; multiple complete fans responding together
indicate shared/mirrored addresses. This is observation guidance, not a claim
that such a wiring arrangement has already been verified.

Unconfigured headers and long strips use a wrapped address grid with separate
hit targets; the display wrapping never changes wire order or effect sampling.

The desktop canvas no longer includes decorative case or desk outlines. Its
zone markers show the registered LED geometry and per-region sampled colors;
the detailed view provides readable full-size keys and regions.

## Smooth and custom effects

RainbowWave spans 720 scene pixels per cycle. Fractional HSV and continuous
radial distance remove the old hue/ring steps. Default spiral and ring gradients
span a 720-pixel radial cycle instead of the old 90-pixel cycle. Custom palettes interpolate
smoothly across their stops, including the cyclic seam.

The desktop preview uses `QOpenGLWidget` and Qt's OpenGL paint engine for texture
filtering, scaling and composition. The color field remains CPU sampled, bounded
to 220 x 140 samples (30,800 rather than 192,500 at the default scene size).
Bilinear filtering gives continuous gradients when enlarged. This is a preview
approximation: real LEDs sample the effect at exact coordinates, independently
of the preview texture. The field is not a full GPU effect shader.

Rendering and all runtime HID operations, mode changes, reconnects and channel
updates run on one serialized hardware thread. GUI commands copy Settings at
submission; worker operations do not read live GUI-owned settings. The worker
owns its 16 ms timer, so missed USB deadlines coalesce rather than queue stale
frames. Shutdown drains restoration commands and joins the worker before HID
handles are destroyed. Hidden or paused previews stop repainting; hardware
continues. All views and the worker use a shared monotonic effect clock. USB/device throughput and physical region counts still limit output.

**Open visual effect editor** replaces the comma-separated palette form:

1. Drag a pattern from the library onto the running preview (clicking also works).
   Wave, Vertical, Diagonal, Radial, Spiral, Plasma, Breathing, Sweep and Static
   gradient provide different spatial/temporal behavior.
2. Drag gradient stops to reposition them. Double-click empty gradient space to
   add a color; double-click a stop to edit it. Add/Edit/Remove buttons provide
   an alternative. Left/right selects stops; Enter edits and Delete removes.
3. Pick colors with the hue/saturation wheel, brightness, exact RGB fields
   (0-255), or hex. The same picker serves global, channel, native and region
   colors. Adjust cycle duration and pattern size with sliders, name and Save.

Recipes contain 2-16 colors, a 500-120000 ms cycle, and a 40-4000 scene-pixel
wavelength. Up to 64 named recipes are persisted. Older color-only recipes load
as evenly spaced Wave palettes. Invalid stop positions fall back to even spacing;
unknown patterns fall back to Wave. Saved recipes are available in both shared
and individual selectors; editing an existing recipe updates both consumers.
The standard selectors also retain eight original effects and six palette
presets, and expose seven additional spatial patterns (21 choices in total).

Implementation follows [Qt's QOpenGLWidget painting guidance](https://doc.qt.io/qt-6/qopenglwidget.html).
The Qt test exports the renderer name and a 30-frame rendering/readback timing;
that synthetic measurement is not a claim about sustained hardware FPS.

The same effect factory serves preview and hardware rendering. Palettes are
interpolated continuously, including last-to-first closure. Profiles, component
chains, region overrides and custom recipes share atomic settings persistence.
Up to 64 custom recipes are loaded, with bounded parameters and coordinates.
Old settings without these fields retain their previous component output.

This is a native declarative effect library. SignalRGB HTML/JavaScript effects
are not imported or executed. Their runtime uses a different API, described in
[SignalRGB's effect development guide](https://docs.signalrgb.com/lightscripts).
No compatibility with that library or its distribution rights is claimed.

## Evidence and remaining checks

Tests cover atomic chain replacement, capacity rejection, malformed geometry,
paired-region address counts, profile/recipe round trips, region overrides in
the render dispatcher, temporary-identification signaling without profile
mutation, custom recipe validation, and continuous palette interpolation.
Hardware-free Qt screenshots exercise the actual Vulcan and Kone descriptors
and a three-fan generic chain. They are visual checks, not hardware proof.

In the current conversation the user confirmed JARGB1-3 following the canvas
and I/O shield color/native-effect control. Fan models, pairing, chaining order,
per-region colors and perceived smoothness of the new version remain subject
to the next physical check. GPU lighting remains outside the current drivers.

Tests also drive the visual editor's Save button, pattern drag/drop, stop dragging,
RGB/hex synchronization, settings migration and GUI heartbeat during a deliberately
slow worker operation. The editor is themed consistently with the workspace.

Validation on 2026-10-21: all 21 Debug CTest suites passed. The optimized Release
app was built with deployed Qt/OpenGL DLLs and launched visibly with the existing
Hardware Check profile. The actual desktop and editor were visually inspected.
The GPU test reported NVIDIA GeForce RTX 5080/PCIe/SSE2 and approximately 12 ms
per synthetic Plasma preview frame including framebuffer readback (Debug).
This does not establish sustained USB frame rate or perceived physical LED smoothness.

Spatial-layout validation on 2026-10-21: all 21 CTest suites passed, including
34 Studio UI checks. Tests cover component add/remove and custom creation,
205 locally available valid definitions, grouped LED dragging, shape/reset,
resize/rotation handles, profile/catalog persistence, transformed dispatch,
sparse-slot zeroing and raw address calibration/restoration. The rebuilt Release
GUI starts and responds. Left Windows / Right Ctrl physical colors remain
unconfirmed; SignalRgb.Service was observed running and service control was
refused by Windows. This is not proof that the service causes the discrepancy.


## Effect persistence and chain workflow correction (2026-10-21)

Select a device in the sidebar, then **Hardware**. ARGB headers show
physical component cards in wire order, their address intervals and physical
LED counts. **Add hardware** searches the installed/catalog definitions;
**Remove** deletes the selected component. **New definition** creates a fan or
strip with explicit address/group counts. **Arrange components / LED groups**
opens the spatial editor. Configured strips retain their declared geometry.
Unconfigured headers show an instruction instead of 120 invented LED positions;
legacy hardware output remains until the user explicitly configures the chain.
The first configured component enables Canvas / GlobalFollow. Small windows
scroll the hardware page rather than overlapping its controls.

**Effects** separates **New effect** from **Edit effect**.
New names are unique; editing loads the selected palette instead of a generic
replacement. Stop selection exposes exact RGB values alongside the color wheel.
Saving emits an immediate atomic settings write; failures remain visible in the
editor and footer. Palette edits also rebuild independent assignments that still
have the same effect ID. Named palette overrides are shared by preview/hardware.

The recipe `path` distinguishes `Component LEDs` (one cyclic gradient per
component), `Entire chain` (one gradient over all addresses) and `Canvas positions`
(the spatial pattern). Missing/invalid paths migrate to Component LEDs. This
corrects the former uniform Breathing palette rotation: LED paths now phase
colors along addresses rather than assigning every address the same hue.
Coincident software-group points use the same effect address in all renderers.
RGB changes, path selection and edited preset palettes are tested through Save,
JSON reload and reopening the editor. These checks do not prove physical fan
wiring, per-key Vulcan colors or sustained USB frame rate.

The independent [roccat-vulcan HID implementation](https://github.com/duncanthrax/roccat-vulcan/blob/master/src/hid.c)
uses the same bank-of-12 planar RGB layout and seven 65-byte reports as this app.
Agreement is protocol evidence, not proof that the fallback key map is correct
for this unit. No speculative Win/Right-Ctrl swap or key-assignment initialization
has been applied based solely on an upstream reference.


## Spatial canvas correction and channel picker

This section supersedes the previous LED-path workflow for shared canvas effects.
The Desktop layout renders the animated effect across its entire surface, with
neutral fan/keyboard/mouse/strip silhouettes above it. The surface is not dimmed
to a decorative background. LED dots and address colors remain in Keys & LED
regions and the hardware detail view. The shared effect selector is available
above the layout without switching away from the placement surface.

GlobalFollow (labelled **Canvas**) always calls the same spatial colorAt(x,y,t)
as the field renderer. Moving/resizing/rotating hardware therefore changes the
sampled colors, including recipes previously saved with a component/chain path.
Independent (**Own effect**) keeps LED-path semantics. Shared-effect editing
opens spatial recipes; it does not offer a misleading positional bypass.
Breathing retains its temporal brightness pulse while distributing palette
colors spatially. Preview pause and the Windows motion preference do not pause
hardware output. Dragging remains immediate and does not add a transition.

Each header now has **+ Add component to JARGBn**. A channel-scoped picker shows
product icons, text search, address/physical LED counts and quantity. It disables
products that cannot fit; adding multiple products is atomic. Each connected
component has its own Remove button. Advanced definition import/create/delete
is under **Manage definitions**, and the picker also offers custom creation.
No settings migration destroys existing component definitions or overrides.

The interaction reference is SignalRGB's documented
[controller configuration flow](https://docs.signalrgb.com/guides/device-configuration/configure-rgb-controllers/).
Product icons are generated locally; vendor artwork is not copied.

Validation: 21 Debug suites passed (the registry-based autostart test required
execution outside the filesystem sandbox). Studio has 37 passing checks; canvas
tests assert background pixel colors and placement-dependent output. Both window
sizes and the picker were visually reviewed. The rebuilt Release process responds,
and its own-window capture shows the restored field with the existing Hardware
Check profile. Physical LED colors and the outstanding Vulcan discrepancy remain
unverified by these software checks.


## Focused workspace and artwork revision

The primary sections are **Layout**, **Effects**, and **Hardware**. Effect selection,
New effect and Edit effect exist only in Effects. The Layout header shows the
current effect name without duplicating the editing controls. Hardware and Effects
hide the placement inspector. LED colors/identification open in a dedicated device
dialog; closing that dialog restores any temporary identification output.
Profile creation/rename/deletion live in Profile options.

Artwork is an original, locally bundled SVG set for fan, mouse, keyboard, zoned
keyboard, strip and controller. It is generic illustration, not vendor photography
or a promise of exact model dimensions. Vulcan keycaps follow the ISO descriptor;
colored markers use actual logical-to-wire mappings and the shared coordinate
transform. Five generic presets and imported installed SignalRGB JSONs populate
the product picker. OpenRGB supplied the fallback Vulcan ISO key table and the
protocol references; workspace profiles are this app's own settings data.
No SignalRGB/OpenRGB product artwork is copied. Legacy 70x40 keyboard/mouse sizes
are migrated to readable dimensions while custom sizes and position stay intact.
Unconfigured or explicitly empty ARGB channels stay in Hardware but no longer
appear as invented devices in Layout. Their existing hardware output policy is
unchanged. The Effects preview now uses the same effect instance as Layout.


The control selector now says Device effects / Software lighting. Color source
says Own effect / Layout effect; the two different concepts no longer both use
Canvas as their visible label. The stored Native/Canvas and source enum values
are unchanged.


Cycle duration uses a logarithmic 0.5-120 second slider: one quarter is about
1.97 seconds, not the former 30 seconds. The adjacent seconds field supports
exact millisecond values and stays synchronized with slider edits. Existing
recipe durations are preserved on opening and still persist as integer milliseconds.

### Vulcan modifier color investigation (2026-10-22)

The working application source is in `.claude/worktrees/lighting-studio/src`
relative to the main checkout. A targeted green/blue swap for Left Windows
(0x0A) and Right Ctrl (0x59) was rejected by the physical retest and removed.
The encoder preserves RGB channels for all 144 slots, covered with purple,
blue, cyan and orange regression inputs. The reported intermittent white flashes
and dark ISO keys remain unresolved. SignalRGB GUI and service were also running
during diagnosis; process presence alone does not prove competing HID writes.
Do not change key mapping or channel order without a controlled single-writer test.
