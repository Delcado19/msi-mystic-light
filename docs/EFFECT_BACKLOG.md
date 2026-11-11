# Effect implementation checklist

Source review: 144 local SignalRGB effect HTML files inspected on 2026-10-28.
Their local creation dates are 2026-10-25; no installation-history comparison
established which effects were newly added relative to an earlier collection.
The checklist tracks native Lighting Studio functionality, not source copying
or exact parity with every SignalRGB control.

## First implementation batch

- [x] ~~Gradient Pinwheel~~ â€” closed, repeating palette with editable center,
  repetitions and rotation rate.
- [x] ~~Spiral Rainbow / Custom Spiral~~ â€” native Spiral with editable colors,
  turns, center and rate; use a rainbow palette for rainbow variants.
- [x] ~~Visor~~ â€” sweeping soft band with editable width, palette and rate.
- [x] ~~Sine Wave~~ â€” visible wave with frequency, amplitude, width and rate.
- [x] ~~DNA~~ â€” paired waves with connecting rungs and shared wave controls.
- [x] ~~Pixel Fill~~ â€” configurable square grid, Rows/Snake/Spiral order,
  gradual leading cell and looping fill.

All six use the shared renderer, movable palette stops, mask/blend/geometry
controls and absolute-time evaluation. The first batch introduced schema-16 settings; current files use schema 17.
Schemas 1..16 retain their previous defaults. Editor and service must be updated
together. Pixel Fill has no separate fade-out/restart-delay controls; DNA uses
fixed rung spacing. Full upstream option parity is outside this batch.

## Second batch and remaining candidates

- [x] ~~Voronoi~~ â€” moving sites and distance metrics; benchmark raster cost first.
- [x] ~~Fireplace~~ â€” coherent animated noise and flame shaping.
- [x] ~~Fireworks~~ (New Years scene extras remain open) â€” launch, explosion, lifetime and trails.
- [ ] Chase â€” deliberate fan/strip geometry and layout mapping.
- [x] ~~Multizone~~ â€” independently configured region effects.
- [ ] Soft Ambience â€” screen capture, cropping and image processing.
- [ ] Hydrogen / audio-reactive effects â€” audio input and analysis.
- [ ] Hot and Cold â€” sensor data and normalization.

## Verification

Regression coverage in `EditorTest::nativePatterns` checks fifteen distinct animated
rasters, repeatable absolute-time rendering, freeze/reverse, controls and undo,
save/load, invalid values, old-schema defaults and atomic schema rejection.
Grid samples distinguish the three Pixel Fill orders. Native Windows captures
are generated as `native-patterns.png` and `native-pattern-controls.png` in the
editor artifacts directory. Hardware output is not part of these tests.

Release and Debug CTest: 19/19 passed each. Full native Windows editor: 129/129
passed, no skips. Both prepared portable binaries match the tested Release build;
SDK-free editor inspection and simulation-host pipe checks passed without hardware
access. Earlier EXEs are retained with a `.before-native-patterns` suffix.

## Editing follow-up

- [x] Repetitions uses a slider and numeric field.
- [x] Sine Wave and DNA close their repeating color palettes.
- [x] New native patterns start as independent layers rather than implicit children.
- [x] Color point step explains snapping and Alt bypass.
- [x] Effect-specific practical ranges replace the generic -600..600 pattern rate;
  effect fields omit suffixes and slider-range rows.
- [ ] Investigate the reported immediate disappearance when changing Rotation in
  the user's project. Full-angle raster tests and native Motion edits retain the
  visible effect; no affected Sine Wave/DNA project snapshot is currently available.

Practical ranges constrain new UI edits without narrowing stored validation
limits. Existing saved extreme values survive refresh/load/undo. No existing
hierarchy is migrated automatically.

Follow-up validation: Release and Debug CTest passed 19/19 each; Windows-native
editor passed 130/130 without skips. Updated portable editor and host match the
tested Release binaries, and package smoke checks pass. The Rotation report
remains open independently of those checks.

## Second batch and color picker

Voronoi, Fireplace, Radar and Fireworks are independent native implementations.
The Multizone action is an editor template: four fixed initial quadrants, with
normal source selection, palettes, geometry and mask editing per region. It does
not reproduce every upstream split slider or nested scene option. Radar currently
has no contact blips; Fireworks has four launchers rather than an independently
configurable upstream launch-rate system. Fireplace noise scrolls continuously
rather than repeating an identical short loop.

New controls include seeded randomization, site count, Euclidean/Manhattan/
Chebyshev distance, flame detail and flame height. Practical ranges and plain
numeric fields remain in force. Schema 17 adds those parameters; authentic old
schema-16 files without the keys remain readable. Editor and host must be updated
together.

The Hue slider explicitly clears inherited cyan sub-page/add-page backgrounds.
A native pixel regression first reproduced the cyan overlay at hue 359 and now
checks that the rainbow track stays unchanged while the chosen color approaches
red. Apply/cancel and live color history tests remain in the gate.

Second-batch validation: Release/Debug CTest each passed 19/19; full Windows-native
editor passed 132/132 with no skips. Native captures of the picker, four new
sources and Multizone were inspected. Both prepared EXEs match the tested Release
build; SDK-free inspection and simulation-host smoke checks pass without hardware
access. Previous EXEs are retained with a `.before-extended-effects` suffix.


## Additional native patterns

- [x] ~~Plasma~~ — continuous animated sine-field color blending.
- [x] ~~Ripples~~ — expanding/contracting concentric closed-palette rings.

These live under Organic and noise / Gradients and patterns respectively.
No upstream JavaScript or assets are imported. New source IDs require schema 18.

Authoring update validation: Release CTest 19/19 and Windows editor 133/133 passed.
Debug editor exceeded its former 120-second suite limit; with 180 seconds it
passed in 126.92 seconds. The other 18 Debug tests passed in the full run.
Portable editor/host match Release; SDK-free inspection and simulation-host smoke
checks passed. No hardware access.


## Additional native source batch

- [x] ~~Aurora~~ — legacy renderer retained; removed from new-source menus because its sine ribbon is redundant.
- [x] ~~Lava Lamp~~ — deterministic merging metaballs with count and seed.
- [x] ~~Checkerboard~~ — animated cell grid.
- [x] ~~Kaleidoscope~~ — mirrored radial sine fields with configurable symmetry.

These are native source implementations, not imported SignalRGB scripts or claims
of full upstream parity. New source IDs require schema 19. Organic/noise and
pattern menus expose only relevant controls.

Final UX batch: native Windows editor 136/136, Release CTest all 19 green after
the offscreen wide-mode test correction. Debug editor passed in 187.87 seconds;
its 240-second limit accommodates the fifteen-source gallery. Package inspection
and authenticated simulation-host smoke passed; no hardware writes.


## Capability gate and particle follow-up

- [x] ~~Particle Burst~~ — finite ballistic cohorts, fading round sprites and a labelled editable palette.
- [x] ~~Lightning~~ — seeded branching flashes with bounded bolt count and finite flash lifetime.
- [x] ~~Perspective Starfield~~ — depth projection, approach acceleration and distance-scaled stars.
- [x] Removed Change selected effect from the inspector; source creation stays in the tool rail.
- [x] Removed Aurora from new-source menus while retaining old project rendering.

Only add sources that cannot reasonably be authored by combining existing sources
and motion/compositing. Color/shape variants and presets do not count as new effects.
Lightning and Starfield add schema-20 source IDs without new dependencies or
upstream code imports. Physical output remains a separate hardware check.
