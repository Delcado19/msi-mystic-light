# Reference repository reuse review

Reviewed 2026-10-23 against this checkout's current component catalog, discovery,
module contract and startup gate, and the accepted Photoshop-inspired editor.
This is a targeted source review, not an implementation, security audit or
runtime qualification. Six public GitHub trees and selected source files were
downloaded at the commits below into ignored `build/research/repo-review/`.
No third-party code or assets were added to production sources.

## Decision summary

| Reference | Recommendation | Priority / current gap |
| --- | --- | --- |
| CompGen | Independently implement grid-based LED authoring and a validated SignalRGB component import adapter | High: catalog/mapping exist; editor and external import do not |
| Lumen | Reimplement scene/layer organization, layout interaction and efficient frame scheduling in Qt | High: directly relevant to the planned editor and output engine |
| LightScript Workshop | Reimplement typed effect parameter descriptors to drive contextual controls | High: introduce with actual effect types, not a speculative public SDK |
| Virtual RGB Rig | Reimplement movable/resizable sampling regions and visible LED sample positions in the Layout view | High: validates the proposed layout workflow; defer its 3D/audio stack |
| SignalBridgePlugin | Use session/lifecycle and atomic-save patterns; evaluate a script host separately | Medium: exact-path transport exists; multi-endpoint sessions and script execution do not |
| Lueur | Retain native effects as change-driven operations, separate from software rendering | High at output integration; do not port ASUS/SMBus operations to MSI |

These are recommendations, not additional user-approved feature scope. The
already accepted layers, masks, transparency, blend modes and effect authoring
scope remain authoritative. Do not adopt upstream navigation that splits our
Lighting workspace into separate top-level categories. The user's later
clarification requires separate, mutually exclusive Layout and Effects views
inside Lighting; do not enable component placement while editing effects.

## CompGen

Snapshot: `4bb27742414dd5864878c05bb04b472026ddfbe2`.
Source: [`index.js`](https://github.com/Frubi22/compGen-for-signalRGB/blob/4bb27742414dd5864878c05bb04b472026ddfbe2/index.js).

`onCanvasClick` and `updateLedArray` add/remove LEDs on a grid; `drawLeds`
shows wire indices. `load` reads `LedMapping`, `LedCoordinates` and `LedNames`;
`createFileAsJson` writes those fields plus product metadata and an image.
The key reusable concept is explicit separation of spatial position and wire
order, already present in our catalog. Add visual wire-index/name inspection
and editing in the component tools of the shared workspace. Import should be
an explicit conversion to our stable-ID schema, not a second native schema.

Do not reproduce the export behavior that filters out-of-bounds LEDs silently,
or assume its direct JSON import validates counts and permutations. Our existing
strict validation must remain the final authority, and a failed import must
preserve the current document/catalog. Image/name metadata needs an explicit
schema extension if retained; do not silently discard it while claiming a
lossless round trip. Our current schema has no such metadata fields.

The inspected license is AGPL-3.0. Recommend independent implementation of the
workflow/data conversion rather than copying its JavaScript or embedded image.
Direct code reuse needs an explicit distribution/license assessment.

## Lumen

Snapshot: `f021bb274980157641240abb0ae042d6181c29d7`.
Relevant sources:
[scene](https://github.com/rahmidaniel/Lumen/blob/f021bb274980157641240abb0ae042d6181c29d7/crates/lumen-core/src/scene.rs),
[renderer](https://github.com/rahmidaniel/Lumen/blob/f021bb274980157641240abb0ae042d6181c29d7/crates/lumen-core/src/render.rs),
[engine](https://github.com/rahmidaniel/Lumen/blob/f021bb274980157641240abb0ae042d6181c29d7/crates/lumen-core/src/engine.rs),
[canvas](https://github.com/rahmidaniel/Lumen/blob/f021bb274980157641240abb0ae042d6181c29d7/src/features/canvas/CanvasStage.tsx),
[power events](https://github.com/rahmidaniel/Lumen/blob/f021bb274980157641240abb0ae042d6181c29d7/src-tauri/src/power/win.rs).

Its Scene stores layout and ordered layers together. Each layer has a stable
UUID, effect, enabled state, opacity, blend mode and target mask. The layer UI
shows topmost first while composition proceeds bottom-to-top. The canvas has
placement rotation geometry, drag movement, pan, zoom and fit-to-layout helpers.
Reimplement these conventions in our Qt document rather than importing a
Rust/React/OpenRGB runtime. Its mask is a boolean device/zone selector, not a
paintable grayscale mask. Keep target selection distinct from our soft masks.
It implements Normal, Add, Screen and Multiply, not our required Overlay mode.

Its engine renders static scenes only when dirty, compares output against the
last sent frames, and avoids queuing more frames while the previous batch is
pending. Preview publication uses the rendered output. Adapt the principles to
our HID path: latest frame wins, bounded pending work, forced full resend after
reconnect, and no suppression of protocol-required keepalives/refreshes. A
software-frame cache must be invalidated on session/mode changes. Our editor
also needs a full canvas preview; Lumen's per-LED sampler alone cannot replace
the shared raster renderer required for images, masks and painting.

Power handling includes suspend/resume, session lock/unlock and cancelled
shutdown. Recommend explicit suspend/reconnect handling with session state
invalidation at production integration; optional lock/idle lighting policy is
a separate product decision. Existing process conflict handling already covers
the earlier detection reference; do not build a second conflict subsystem.

The inspected license is MIT. Source adaptation is a possible option with
license notices, but the different stack makes a small native implementation
more practical. Protocol support for other devices is not qualified by this review.

## LightScript Workshop

Snapshot: `e255121eaebc1f6a80b1ada7435621dc686507e6`.
Sources:
[control definitions](https://github.com/hyperb1iss/lightscript-workshop/blob/e255121eaebc1f6a80b1ada7435621dc686507e6/packages/core/src/controls/definitions.ts),
[effect lifecycle](https://github.com/hyperb1iss/lightscript-workshop/blob/e255121eaebc1f6a80b1ada7435621dc686507e6/packages/core/src/effects/base-effect.ts),
[Canvas effect](https://github.com/hyperb1iss/lightscript-workshop/blob/e255121eaebc1f6a80b1ada7435621dc686507e6/packages/core/src/effects/canvas-effect.ts).

Control metadata includes ID, label, default, tooltip, numeric bounds/step,
booleans, choices and colors. Effects separate control application from drawing
and receive time/delta time. Use a small typed descriptor set for the Qt
Properties panel and the same validation when loading saved parameters.
Keep per-layer instance state independent. This avoids hand-building every
effect's controls and gives imports a single validation boundary.

Do not port its per-effect browser animation loop: our document has one clock
and renderer. Do not inherit minimum speed/percentage conversions blindly;
zero speed and zero coverage must retain their defined meaning in our editor.
Defer WebGL/GLSL hosting, TypeScript decorators, live source reload and audio.
The inspected license is MIT; focused algorithm reuse is possible with notices,
but no TypeScript dependency is needed for the initial C++ effects.

## Virtual RGB Rig

Snapshot: `48db4101b6ca214fd3eee7bbf353b056feae79be`.
Source: [`index.html`](https://github.com/akostkasmith/signalrgb-effects/blob/48db4101b6ca214fd3eee7bbf353b056feae79be/index.html).

Devices have editable rectangles; LED points retain product-local coordinates
that are transformed when the rectangle moves/resizes. It overlays sampling
locations, supports mouse and keyboard movement/resizing, and stores layout.
Reimplement these interactions and persisted placements in our Layout view;
use actual component geometry rather than its hardcoded demonstration rig.

Its sampler rounds/clamps into a fixed 320x200 image. That is not sufficient
evidence for smooth soft-mask sampling: choose and test our interpolation and
out-of-canvas behavior explicitly. Keep document resolution independent from
viewport zoom and device capacity. Defer Three.js, desk models, ambient lighting,
audio and browser effect execution. The inspected license is MIT; the monolithic
HTML implementation is not a useful direct dependency for Qt.

## SignalBridgePlugin

Snapshot: `570c39a7ad5b635eadae325805cc6855929bf53b`.
Sources:
[endpoint session](https://github.com/Zayrick/SignalBridgePlugin/blob/570c39a7ad5b635eadae325805cc6855929bf53b/src/openrgb/EndpointSession.cpp),
[discovery](https://github.com/Zayrick/SignalBridgePlugin/blob/570c39a7ad5b635eadae325805cc6855929bf53b/src/discovery/DiscoveryService.cpp),
[configuration](https://github.com/Zayrick/SignalBridgePlugin/blob/570c39a7ad5b635eadae325805cc6855929bf53b/src/config/DeviceConfigStore.cpp),
[runtime](https://github.com/Zayrick/SignalBridgePlugin/blob/570c39a7ad5b635eadae325805cc6855929bf53b/src/runtime/QuickJsRuntime.cpp).

It groups endpoint handles, opens exact paths and closes handles with session
lifetime. Discovery suppresses duplicate physical groups; configuration uses
QSaveFile. Use Qt's QSaveFile directly for our future document/settings saves,
checking write and commit failures, rather than copying its store abstraction.
For multi-interface devices, design explicit required endpoint roles and one
host-owned session only when a supported device needs them.

Do not copy constructor-triggered opening or best-effort opening of every
related endpoint with swallowed errors: our discovery must remain read-only,
connection explicit, and required-endpoint failures visible. The selected
QuickJsRuntime.cpp does not configure execution interruption or memory budgets;
this file alone cannot establish safe untrusted-script execution. A future
script host needs bounded execution, permissions and an explicit supported API
subset. An OpenRGB plugin is not loadable as our Qt driver module.

The inspected LICENSE is GPL-2.0; prior README research describes or-later.
Verify the exact file-level terms before any direct source adaptation. Keep
QuickJS/SignalRGB compatibility as a separate design decision.

## Lueur

Snapshot: `2cca7121000a550db51089c25b6e0f1b7c37492c`.
Sources:
[effects](https://github.com/Renan-projects/lueur/blob/2cca7121000a550db51089c25b6e0f1b7c37492c/src/effect.rs),
[ASUS driver](https://github.com/Renan-projects/lueur/blob/2cca7121000a550db51089c25b6e0f1b7c37492c/src/devices/aura_usb.rs),
[README](https://github.com/Renan-projects/lueur/blob/2cca7121000a550db51089c25b6e0f1b7c37492c/README.md).

It programs native effects instead of streaming software frames; the ASUS
driver distinguishes applying settings from an explicit persistence commit.
Retain that distinction for MSI: send native settings on changes/reconnect,
leave firmware running, and keep routine writes transient. Numeric native
effect IDs are vendor-specific and must stay inside the driver/profile boundary.
Its ASUS topology requests are not evidence of MSI attached-LED discovery.
No ASUS reports, ENE/SMBus/PawnIO support or flash-persistence feature should be
ported into this increment. GPL-2.0 license text is present; exact reuse terms
still need verification before copying source.

## Other previously listed integrations

The [SignalRGB-to-OpenRGB bridge README](https://github.com/Fefedu973/SignalRGB-To-OpenRGB-Bridge)
was refreshed: it requires running SignalRGB and an OpenRGB SDK server. Its
identity limitations reinforce using stable device metadata instead of list
indices. An optional SDK backend could be useful later, but would not satisfy
the standalone direct-HID goal as the primary backend.

The [Home Assistant integration README](https://github.com/hyperb1iss/signalrgb-homeassistant)
was refreshed: effect presets/layout selection are possible future automation
ideas, but the implementation controls a running SignalRGB HTTP API. Do not
add it as a runtime dependency. `signalrgb-python` was not source-reviewed in
this pass; its raw README web fetch failed. No claim of new validation is made
for it or for the remaining hardware-specific topic repositories. The separate
16-project GitLab inventory remains the source for that scope; it was not
re-audited here.

## Implementation and validation sequence

1. Shared document, stable layer/component-instance identities, grayscale masks,
   existing blend modes, and typed effect parameters. Test mask/alpha math,
   duplication independence, undo/redo and save/load preservation.
2. Integrated layout tools, wire-order inspection and explicit SignalRGB JSON
   conversion. Test malformed arrays, bounds, mappings, import cancellation and
   preservation of the last valid document; keep metadata losses visible.
3. One renderer/clock and sampling transform for canvas preview and LED output.
   Test gradients at fractional coordinates, transformed products, out-of-bounds
   points, per-channel targeting and correct native/software separation.
4. Explicit connection/reconnect lifecycle, latest-frame scheduling and atomic
   saves using Qt. Test transport failure/retry, cache invalidation and failed
   saves before real-device validation under the existing confirmation gate.
5. Add accepted image/motion/particle features incrementally. Script drivers,
   3D, audio, network integrations and other hardware remain separate decisions.

No upstream applications or test suites were installed/run. License names above
come from inspected files and are provenance notes, not a blanket determination
that all source/assets may be redistributed. A source-level comparison does
not qualify image quality, frame rate, physical colors or device behavior.
