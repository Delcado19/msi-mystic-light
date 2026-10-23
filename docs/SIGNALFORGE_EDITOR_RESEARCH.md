# SignalForge editor reference

Reviewed 2026-10-23 at upstream commit
`dddf36ddf1a220cc1379df7a165c2ae8fb8ceb8a`.
Repository: https://github.com/flowerpot-byte/SignalForge

## Evidence and useful concepts

Inspected the published `docs/screenshots/02-layers.png`, renderer shell and
preview source, engine source, package manifest, export parity test source,
and additional license terms. No upstream code was installed or executed;
upstream tests were read, not run. The screenshot is a published example,
not a verified rendering of the current commit.

The screenshot gives most space to the effect preview, with an inspector on
the right containing layers and selection-specific properties. Preset tiles
sit below the preview; project actions sit in a bottom bar. The current shell
source explicitly removes redundant left navigation. These principles fit
our effect editor: prioritize the work surface and expose properties in the
context of the selection rather than duplicating navigation.

The preview source loads `SignalForgeEngine`; the package bundles the engine
once and the export parity tests compare preview/export rendering. This is a
useful architectural reference, not proof that our Qt renderer can consume
its JavaScript implementation. SignalForge uses Electron and Canvas; this
project remains C++/Qt. No framework migration is proposed.

## Proposed adaptation for this app

The user's subsequent editor requirement takes precedence over SignalForge's
shell: all editors follow Photoshop interaction conventions. Layout, effects
and canvas share one Lighting category with mutually exclusive Layout and
Effects editing views. A central viewport may be reused, but component placement
and effect authoring must never be active simultaneously. Each new effect layer
owns an effect instance and an initially white editable grayscale mask immediately.
Masks support soft painting and gradients with painting-app-style thumbnails
and editing feedback. See the unified editor contract in
[`superpowers/specs/2026-10-18-standalone-lighting-app-design.md`](superpowers/specs/2026-10-18-standalone-lighting-app-design.md).

- Keep the effect preview large, with a right inspector for layers and their
  properties, and compact playback/project controls below.
- Provide separate Layout and Effects editing views within Lighting. Layout
  arranges components; Effects edits canvas/layers/masks. Preserve each view's
  selection and viewport state. Any LED overlay in Effects is read-only.
- Add an optional LED overlay to the effect preview so users can see which
  pixels reach their configured LEDs. Do not confuse a canvas picture with
  physical device output.
- Select the target channel explicitly and show its Native / Software mode.
  Native mode exposes only supported firmware controls. Each channel retains
  its independent assignment; do not replace this with one global effect.
- Use one renderer and clock for preview and eventual software output. The
  output path samples that rendered frame through validated component mappings;
  the preview must not implement a second approximation of the effect.
- Keep preview playback separate from enabling hardware output. Configuration
  editing and preview must remain usable without opening devices.

This is design direction, not an implemented editor. The first editor increment
should demonstrate a Static Color effect layer stack, component placement,
a selected channel, LED overlay, editable per-layer masks with soft brushes and
gradients, a dedicated layer Transparency slider, Normal/Multiply/Screen/Overlay
blend modes, contextual properties and undo/redo before adding images or particles.
Current catalog/configuration work remains the prerequisite for real layouts.

The user subsequently accepted image, shape, gradient and particle effects,
motion controls and color animation as follow-up authoring increments. They
share the same layer/mask editor; see the accepted effect authoring scope in
the design spec for delivery order and boundaries. This acceptance does not
establish upstream file compatibility or require SignalRGB export.

## Reuse boundary

The upstream declares GPL-3.0-or-later and additional attribution terms in
`ADDITIONAL-TERMS.md`, plus a runtime exception for generated effect files.
This review copies no code, artwork, branding, or styles. Any future source
reuse needs a separate license review against this project's distribution
terms; the runtime exception must not be assumed to cover editor code.

## Sources

- [Published layers screenshot](https://github.com/flowerpot-byte/SignalForge/blob/dddf36ddf1a220cc1379df7a165c2ae8fb8ceb8a/docs/screenshots/02-layers.png)
- [Editor shell](https://github.com/flowerpot-byte/SignalForge/blob/dddf36ddf1a220cc1379df7a165c2ae8fb8ceb8a/app/renderer/components/shell.js)
- [Preview](https://github.com/flowerpot-byte/SignalForge/blob/dddf36ddf1a220cc1379df7a165c2ae8fb8ceb8a/app/renderer/components/preview.js)
- [Parity tests](https://github.com/flowerpot-byte/SignalForge/blob/dddf36ddf1a220cc1379df7a165c2ae8fb8ceb8a/test/export/parity.test.js)
- [Additional terms](https://github.com/flowerpot-byte/SignalForge/blob/dddf36ddf1a220cc1379df7a165c2ae8fb8ceb8a/ADDITIONAL-TERMS.md)
