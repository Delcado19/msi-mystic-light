# Lighting Studio UI redesign

Date: 2026-10-21. Platform: Windows, Qt 6 Widgets. Scope: desktop interface,
preview rendering and interactions; no new hardware driver or protocol changes.

## Design thesis

A lighting workspace should make the relationship between a physical zone,
its effect and its color immediately visible. The spatial preview is the
signature element; navigation and editing stay quiet and predictable.

## Audit and resulting changes

| Before | After | Reason |
| --- | --- | --- |
| Ten dense rows expose all channel controls at once | Device sidebar and one selected inspector | Progressive disclosure and clear ownership of an edit |
| 22 px color-only swatches | Large color buttons with readable hex values | Black is visible as an explicit choice; color is not the only information |
| Fixed-size canvas clips saved placements | Fitted scene with inverse pointer transform | Resizing changes presentation, not persisted coordinates or hardware sampling |
| Tiny scaled device labels | Screen-space labels and full names in the sidebar | Retain readable text in compact windows |
| Global effect behind white labels | Subdued effect preview with opaque label surfaces | Keep controls and content legible over arbitrary RGB output |
| Native zones have empty preview outlines | Representative color chips | Do not imply that a zone is unlit merely because it uses firmware effects |
| Continuous per-block painter calls at 60 Hz | OpenGL-composited filtered field at up to 60 Hz, suspended while hidden or paused | Lower preview overhead without changing the hardware rendering cadence |
| Profile actions mixed into the channel grid | Profile workspace above device navigation | Stable location for switching, adding, renaming and deleting profiles |

## References and interpretation

- [SignalRGB layouts](https://docs.signalrgb.com/guides/device-configuration/about-layouts):
  spatial relationships and selecting a device in its layout.
- [OpenRGB](https://openrgb.org/): lightweight local control and explicit device settings.
- [MSI Mystic Light](https://us.msi.com/support/technical_details/DT_SW_Mystic_Light):
  device-specific supported effects and profile workflows.
- Emil design engineering: immediate response, restrained transitions, deliberate
  spacing and interruption-safe feedback. Reviewed with the emil-design-eng skill.
- Apple design and HIG skills: grouping, hierarchy, native pointer/keyboard
  controls and progressive disclosure, translated to Windows rather than
  copying macOS window chrome.

HIG references read from the apple-hig-review skill: accessibility.md / Vision;
layout.md / Best practices and Visual hierarchy; typography.md; color.md;
designing-for-macos.md; sidebars.md; split-views.md; dark-mode.md; motion.md;
focus-and-selection.md / Best practices. These are design principles here,
not a claim of Apple-platform compliance.

## Motion audit, opportunities and implementation

The improve-animations and find-animation-opportunities passes were read-only;
implementation then used animate and was checked with review-animations.
Animation-vocabulary supplied terminology, not additional animation scope.

| Candidate | Frequency and purpose | Decision |
| --- | --- | --- |
| Pointer selection highlight | Frequent, direct editing | Immediate; unused fade state removed |
| Channel dragging | Continuous direct manipulation | Track pointer 1:1; never ease or spring the hardware placement |
| Keyboard device navigation | Frequent, functional selection | Immediate; no animation |
| Inspector content changes | Frequent, functional editing | Immediate; avoid moving controls under the pointer |
| Profile menus and color dialogs | Native focused interactions | Keep Qt controls and their focus behavior; no custom animated dropdown |
| Ambient panel shimmer and decorative entrance sequence | Repeated decoration without functional purpose | Rejected |

Windows SPI_GETCLIENTAREAANIMATION starts the
on-screen preview paused when client animations are disabled. A visible pause
control affects only preview repainting, never hardware output. A hidden
window does not keep its preview repaint timer running. This Qt painter
implementation makes no browser-compositor or GPU-only animation claim.

## Contracts preserved

- Existing profile names, channel identifiers, settings schema, effect IDs and
  change signals remain stable; selecting a device does not write settings.
- Mode choices still come from IDriver capabilities, including native-only
  IOShield and JRGB1. Irrelevant form rows are hidden.
- Custom saved sizes and positions remain unchanged; only exact legacy 70x40
  peripheral sizes are upgraded for readability. Newly created channel entries
  receive separated default positions instead of all overlapping at the origin.
- The canvas is a representative preview, not device discovery, exact firmware
  animation emulation or a statement that physical LEDs changed.
- GPU lighting remains unsupported and is labeled accordingly.

## Verification and limitations

Tests cover device-list/inspector selection, canvas selection synchronization,
scaled drag behavior through existing integration tests, preview pause without
settings changes, and all existing color/profile/capability interactions.
The optional LIGHTING_STUDIO_SCREENSHOT environment variable exports a
hardware-free demo at 1440x880 and 1080x700 logical pixels, using the real Qt
widgets and draw code. Demo screenshots never load or modify the user's profile.

Visual review checks spacing, labels, focus states, dropdown affordances and
both window sizes. Dark surfaces intentionally provide a neutral surround for
RGB inspection. A separate light/high-contrast theme, full screen-reader audit,
and real reduced-motion toggle testing remain follow-up work. Native firmware
animations are represented by their configured color, not simulated exactly.

Final validation: all 21 CTest targets passed in Debug on 2026-10-21,
including the Windows autostart test. Both exported window sizes were visually
inspected. UI screenshots are synthetic fixtures, not hardware verification.


## Component/region follow-up

The later [component and effect implementation](COMPONENTS_AND_EFFECTS.md)
supersedes the original single-color chips and decorative case/desk backdrop.
The desktop now shows registered region geometry; a separate detail tab shows
readable keys or regions. A bounded, OpenGL-filtered color field replaces the
earlier CPU-heavy preview. USB rendering has its own serialized thread.
Explicit fan/strip chains and custom recipes are configurable and persisted.
The visual editor replaces text-based palettes with draggable patterns/stops,
a live preview, and a shared color-wheel/RGB/hex picker. See the linked document
for exact rendering limits and migration behavior.

## Spatial layout and physical calibration follow-up

The fixed 70x40 footprint is now a legacy default, not the only sampling size.
Desktop manipulation includes resize/rotation handles and exact dimensions.
A transactional graphics editor adds movable component/LED geometry, software
LED grouping, four layout presets and per-component reset. Installed component
metadata and custom definitions populate the hardware list. Explicit empty chains
stay empty. All views and hardware share the geometry transform and wire mapping.
Vulcan wire calibration is based on physical single-address probes; reference
OpenRGB tables are a starting point whose correctness must still be tested.


## Workflow review: effects and physical chains

| Before | After | Why |
| --- | --- | --- |
| Hardware actions below the inspector fold | Dedicated Hardware chains tab and visible shortcut | Adding/removing physical products is a primary task. |
| One ambiguous new/edit button | Separate New canvas effect and Edit selected effect | Preserve the selected palette and avoid accidental default-name overwrite. |
| Temporal palette preview only | Explicit per-component, whole-chain and canvas paths | Preview the LED behavior the user intends to configure. |
| Generic LED slot cloud | Component cards, address intervals and spatial geometry | Make cable order and physical placement distinguishable. |
| Claimed autosave without failure feedback | Immediate recipe save with editor/footer feedback | Save must have an observable outcome. |

The native Qt interaction model remains. No decorative transitions were added;
direct manipulation and continuously rendered previews provide the feedback.


## Corrected layout intent

| Before | After | Why |
| --- | --- | --- |
| LED dots replace the spatial field for LED-path recipes | Continuous full-color field underneath neutral hardware silhouettes | Users place hardware by the visible flow of color. |
| GlobalFollow can ignore hardware position | Preview and shared output sample the same scene coordinates | Moving a device must change the colors it samples. |
| Catalog combo plus generic add/remove actions | Channel-scoped searchable product picker and per-component Remove | Keep the target channel visible throughout the action. |
| Effect selector lives only on another tab | Shared selector stays above the layout | Tune an effect while arranging hardware. |

Emil/Apple/animate remain the active design constraints: direct pointer tracking,
clear action targets, native focus and keyboard handling, no cosmetic movement
on repeated actions. The continuous effect serves as placement information;
existing pause/reduced-motion support remains independent of hardware timing.


## Screenshot-driven structure and artwork review

| Before | After | Why |
| --- | --- | --- |
| Hardware shortcut duplicates Hardware tab | Three primary sections: Layout / Effects / Hardware | One navigation target per task. |
| New effect and effect picker compete with placement | Effect creation/selection/editing only in Effects | Keep the placement task focused. |
| Placement inspector remains beside unrelated tasks | Inspector only in Layout; LED details in a device dialog | Show controls in context. |
| Fan outline, oval mouse, rectangular key grid | Original cached SVG bodies with descriptor-based keys/light markers | Make hardware types recognizable. |
| Tiny 70x40 keyboard/mouse footprints | Readable legacy-size migration; custom sizes retained | Preserve deliberate placements while fixing unreadable defaults. |
| Unconfigured connectors appear as fake layout devices | Configure them in Hardware before they appear | Do not imply a known physical product. |
| Effect preview reconstructs a similar pattern | Preview shares the selected effect instance | Prevent visible preview divergence. |

Apple HIG references applied as Windows design principles: layout.md / Best
practices and Visual hierarchy (group related controls, separate controls/content),
accessibility.md / Vision (readability), typography.md / Ensuring legibility,
color.md / Best practices, sidebars.md, toolbars.md, icons.md and split-views.md.
Emil review: direct response, fewer competing actions, one consistent illustration
system. Motion review removed unused selection-fade state; dragging and selection
remain immediate. The functional color field retains pause/Windows-motion support.
A complete screen-reader and alternative-theme audit remains outside this change.

## Layered effects and readable hardware (2026-10-22)

The component and live LED views use compact, clipped labels with full product
names retained in tooltips and settings. The component editor fits after show,
when its viewport has the final size. Hardware controls use Add hardware; the
picker offers category filtering and original category-specific SVG illustrations.
The current local catalog contains 200 valid installed SignalRGB metadata profiles
and ten explicit generic templates. These describe ARGB topology, not additional
USB drivers or proof of support for every vendor model.

Custom effects support a base and up to eight flat overlays. Drag a pattern onto
the editor preview to append an overlay; double-click its stack entry to edit,
and drag entries to reorder. Screen, Add and Normal blending and opacity are
applied in stack order. Color source produces a soft moving light field; two
sources mix continuously where they overlap. Scroll supports eight direction
presets and an arbitrary clockwise angle (0 right, 90 down), palette colors,
cycle duration and size. Each layer can be clipped to a rectangular region in
the 1100 by 700 scene. Existing recipes remain valid without layers. Saving
preserves order, colors, stops, timing, direction, mixing and zone bounds.

Ctrl-click a component in Layout to select it within its wire chain. Drag its
body to move it, its corner to scale proportionally or its rotation arrow to
rotate it. The same geometry drives preview and hardware; sibling LED positions
and wire order are preserved by the shared editComponentGeometry path. The
separate component editor remains available. Software LED grouping is removed;
physical paired LEDs retain their actual electrical address relationship.

The heading and help area reserve consistent height between tabs. The effect
editor scrolls its controls while keeping Save and Cancel visible.

## Effect semantics and individual rotation correction (2026-10-22)

Scroll is the sole directional-gradient entry: Right, Down, Left, Up plus a
free angle. Wave, Vertical and Diagonal are retained as compatibility names
for old recipes and normalized when edited. Aurora/Ocean/Ember/Pastel/Polar/Sunset
are palettes in the editor rather than duplicate effects. Saved legacy preset
identifiers remain playable. Breathing modulates brightness without moving
palette stops, both on scene coordinates and LED paths. Comet has a trailing
head; Marquee has alternating illuminated segments; neither is edited as Sweep.
Angular rainbow and concentric rings remain separate spatial effects.

Click a component directly to select it within a chain, then use its own
rotation arrow or scale handle. Alt-drag moves the entire chain. Only the active
component handles are shown while it is selected. Baked LED coordinates and
unchanged wire addresses persist; SVG orientation follows the reset geometry
so rotation remains visible even on a circular LED arrangement. Regression
tests drag the individual rotation handle and verify the sibling is unchanged.

## Selected-layer inspector and whole-canvas breathing (2026-10-22)

The editor uses a fixed layer list including the base, with one inspector for
the selected layer and a preview of the committed composition. Changing the
pattern, colors or parameters updates that layer only. Add/drop creates and
selects an overlay; switching layers commits the previous layer before loading
controls with blocked signals. Layer mixing and opacity are visible directly.
Base cannot be removed or reordered above overlays. Save/Cancel remain fixed.

Breathing is a uniform temporal crossfade through the chosen colors, with no
spatial gradient or implicit black interval. Add black explicitly to the palette
for a fade to darkness. The built-in single-color preset uses color and black.
Scroll direction and angle never change a pattern. Circular patterns instead
show inward/outward or clockwise/counterclockwise controls; angular and spiral
patterns expose palette fan angle. Those attributes persist per layer.
Fan illustrations and their canvas selection frames have square footprints.

Wheel input over an unfocused inspector control scrolls the inspector instead
of changing the effect or numeric value. Reordering commits before the model
move and blocks selection reload until the order is settled, preserving each
layer's parameters. Interaction tests cover selection, independent pattern edits,
combined red/blue output, wheel protection and moving a zoned layer in the stack.

## Pixel-tested combined and selected previews (2026-10-22)

The editor shows the combined output and selected-layer solo preview separately.
Selection changes the inspector and solo preview, while all checked layers remain
active in the combined output. Checkboxes independently enable layers; this state
persists and affects the runtime compositor. New layers use distinct seed colors
and a color-to-black palette, so Scroll/Radial/Spiral are visible patterns rather
than constant blue. Color source uses its first color as before. Existing palettes
are retained. A widget-pixel regression switches layers, checks both previews,
toggles an overlay, saves/reloads and checks the runtime factory's combined output.

## Circular centers, numbered previews and effect labels (2026-10-22)

The solo preview identifies the selected layer number (base is Layer 0).
Drag its crosshair or click a new point to reposition a rings/color-wheel/spiral
center in 1100x700 scene coordinates. Centers persist independently per layer
and feed the same runtime sampling as the preview. Angle, fan angle, pattern
spacing and opacity have synchronized sliders and exact numeric fields.

Persisted IDs remain compatible. Visible names now describe behavior:
Radial/SwirlCircles = Concentric rings, Angular/RadialRainbow = Color wheel,
Marquee = Chase, StaticColor = Solid color, Breathing = Breathing / color crossfade.
Scroll, Spiral, Plasma, Color source and Static gradient retain their names.
Custom recipe names remain user-controlled, even when a name differs from its
selected pattern. Tests cover center translation, pointer dragging, slider
synchronization, preview numbering and settings roundtrips.

The profile audit found a saved custom recipe named Spiral containing Scroll.
Custom recipes whose names conflict with a different built-in pattern show the
actual pattern as a suffix, e.g. Spiral / Scroll. Only explicit custom IDs select
such recipes; they no longer replace a built-in effect with the same name.

## Read-only bases and equal-aspect previews (2026-10-22)

The primary effect selector contains built-in single-pattern bases only. Saved
user recipes and layered compositions have a separate Your compositions selector.
Built-ins remain immutable: Edit proposes a unique copy name, the dialog rejects
the original/reserved name, and createCustomEffect rejects all built-in IDs.
Existing recipes are kept; users can rename an old conflicting recipe when editing.

Both editor previews have equal layout weight and render a fitted 1100:700 scene,
matching the main layout canvas. The center crosshair uses that viewport for
pointer conversion; letterboxing never stretches circles or changes coordinates.
Layer name is editable in the inspector and survives selection changes and saving.

Selecting a base initializes an otherwise black global color to white, rather
than showing a blank default effect. Explicit custom palettes are retained.
A custom comet starts with a visible tail/head phase rather than zero brightness
at the first address. Tests inspect all base effects at time zero, preview aspect,
copy protection, reserved IDs, renamed layers and saved-composition selection.

## Compact controls and direct zone editing (2026-10-22)

The shared StudioControls stylesheet defines visible 26x15 logical-pixel stepper
buttons whose full area is clickable, with SVG arrows and hover feedback. It
also reduces field padding, list-row padding and inspector gaps consistently
across the main window and editor. Tests click opposing corners of both integer
and decimal up/down buttons rather than testing only their arrow centers.

Color source keeps angle and replaces direction presets with Sine, Cosine,
Ping-pong and Static movement. The selected movement persists per layer. Sine
is the legacy default. Draw zone arms a one-shot gesture on the selected-layer
preview: drag a rectangle in either direction, see the clipping update during
the drag, and retain exact numeric bounds afterward. The rectangle is clipped
to the scene and does not change any other layer's zone. Selection changes
cancel drawing mode. Frequent controls and pointer gestures respond directly,
without decorative animation, following the Emil/Apple interaction review.

The visual review also widens numeric readouts for unit suffixes, wraps long
pattern names, refreshes the solo caption when the selected pattern changes,
and isolates palette display/insertion from spatial masks and layer blending.
A regression checks that adding a palette stop on a zoned effect interpolates
the palette rather than inserting black from outside the mask.

### Canvas drag regression

Standalone devices move their canvas placement without rewriting LED coordinates. Component edits preserve sibling scene positions for every effect source, including independent effects and native mode. Regression coverage checks plain dragging with both independent and global sources.

### Gaming visual hierarchy and action sizing

Actions use content-sized widths with a minimum 32 logical pixel height. Color swatches remain expandable. Cyan active accents, graphite surfaces and restrained corner radii separate controls from the lighting field. Canvas labels appear only for the selected device to avoid overlapping nearby hardware. The effect badge uses public effect names.

Effect Studio provides a shared pause for combined and solo previews and inherits the canvas pause preference. Hidden previews stop their timers; preview pause never changes hardware timing. Reset device size is explicit and preserves saved dimensions until invoked. Keyboard and mouse footprints use the existing default dimensions; other chains retain their aspect ratio within 180 scene units.

Validation covers action width/height, shared preview pause, device size reset and existing canvas drag/rotation regressions. Direct editing remains immediate without decorative motion.

### Layer identity and layout scroll bounds

Solo previews use the saved layer name consistently after deletion, reordering and renaming. New layers choose an unused default name. Component layout scene bounds follow moved and scaled items so horizontal and vertical scrollbars can reach them. Horizontal wheel gestures retain native scrolling; vertical wheel zoom remains proportional. Regression coverage checks deleting a preceding layer, live renaming and reaching an enlarged component moved beyond the original scene.

### Maximized layout, grid and installed hardware

Component layout opens maximized with optional adjustable scene-space snapping. Concrete invoice-model templates and a RAM category now supplement installed SignalRGB metadata. Unverified pump/case counts require explicit entry. GPU and RAM cannot be attached to an ARGB header. See [installed hardware evidence and limits](INSTALLED_LIGHTING_HARDWARE.md). I/O cover follows Canvas through one volatile color zone; the exact Gigabyte AERO controller has a signature-gated NVAPI backend.

### Device eyes, complete single-zone artwork and RAM output

Device rows now provide an eye/eye-off toggle. Profile persistence records layout visibility separately from output settings; visibility changes do not re-arm hardware. Full placement rectangles represent single-zone GPUs and I/O covers so they remain visible and draggable. The two physically identified XPG Lancer Blade 24 GB modules each report eight light-bar addresses and have separate Canvas placements. A bounded protocol-v0 SDK adapter talks only to the isolated RAM helper; malformed, truncated or nonmatching descriptors are rejected. Helper configuration disables every other known detector and uses localhost only.

Device rows reserve 38 logical pixels and use 32-pixel eye targets without inherited tool-button padding. The list draws selection only; the embedded label draws each device name once. Dedicated GPU artwork shows a graphics card rather than the generic heatsink. A geometry regression asserts that the eye stays inside its item rectangle.

Effect-zone coordinates use compact 110-pixel fields with adjacent labels and trailing flexible space, keeping the label/value association clear in wide windows.

Enabling the rectangular layer restriction immediately arms mouse drawing in the selected-layer preview. Draw zone arms it again for replacing the rectangle; reverse-direction drags are supported.

Standalone device handles edit the placement rectangle, including flat RAM light bars. Component geometry handles are reserved for multi-component chains, preventing zero-height LED bounds from collapsing or inflating the saved device footprint.

Saving a standalone device LED arrangement preserves its canvas position and artwork dimensions. Only multi-component chains recompute their outer footprint; flat-to-spatial and spatial-to-flat RAM edits are covered by a regression test.
