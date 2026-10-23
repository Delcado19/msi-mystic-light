# Offline lighting editor

The first editor increment provides one Lighting workspace with separate Layout
and Effects tabs. Start `build/ninja/Release/msi_mystic_light.exe --editor` to
skip the process-conflict dialog for offline editing. Normal startup still
checks conflicts first. Neither editor mode connects to devices or sends HID
reports. `--inspect` and `--scan` remain terminating diagnostic commands.

## Layout

Choose a catalog component and Add component. Click/drag one of its LED points
to move that instance, or use arrow keys (Shift: 10 units). Wire indices are
shown beside LEDs. Remove selected component removes only the instance.
Wheel zooms around the pointer; middle-button drag pans. Layout displays the
effect composition behind the configured components, but cannot edit effects
or paint masks. Switching tabs retains each canvas's viewport and selection.

Scan supported devices enumerates descriptors and OS board identity only.
Every matching profile/exact-path pair remains selectable; no candidate is
selected automatically. This is a staged, session-only choice, not a connection
or component-to-channel assignment. Rescanning clears stale choices. Unplugged
endpoints are not monitored automatically in this increment.

## Effects

New layer creates an independent Static Color effect with a white grayscale
mask. The topmost panel entry is the topmost composited layer. Rename by editing
the layer name, toggle its visibility checkbox, drag to reorder, duplicate or
delete. Effect and mask thumbnails select the editing target; the left toolbox
also offers Edit effect / Edit mask. Effect color opens the color picker.

Each layer has a dedicated Transparency slider (0% visible, 100% transparent)
and Normal, Multiply, Screen or Overlay blending against the composition below.
Rendering retains RGBA: a lone Multiply layer over transparency remains visible.
Blend calculations use Qt's raster composition in the current encoded color
space; Photoshop pixel parity or calibrated physical LED appearance is not
claimed. There is no second layer-opacity control.

For masks, white reveals, black hides, and gray partially reveals lower layers.
Mask alpha multiplies effect alpha and inverse transparency before composition.
Choose Mask brush or Mask gradient. The brush has diameter, hardness, opacity
and gray value; gradients run black to white along the drag direction. Show
grayscale mask previews the mask itself, while Enable mask can bypass it without
discarding data. Fill white/black and Invert are non-destructive effect operations.
Escape or switching views cancels an unfinished stroke; a completed stroke is
one undo step. Masks currently stay in canvas coordinates; effect transforms
and linked/unlinked mask transforms are not implemented.

Undo/Redo cover document edits, including independent copies of duplicated masks.
Viewport/selection are editing context, not undoable lighting changes. Layer
targeting currently covers the entire offline canvas; per-channel/component
targets and native-mode controls remain pending.

## Project files

File > Save/Open stores `.lighting.json` schema 1 documents: component instance
IDs/catalog IDs/positions and ordered Static Color layers with IDs, color,
visibility, transparency, blending and lossless embedded PNG grayscale masks.
Saving uses QSaveFile atomic replacement, and write/commit errors remain visible.
Opening validates the entire candidate before replacing the document and history.
Unknown component IDs, malformed masks or invalid properties leave the current
document intact. Documents depend on the matching local component catalog;
definitions are not embedded, so editing a catalog can change project geometry.

This increment uses a fixed 640x400 document, up to 64 layers and 256 component
instances, a 32 MiB input limit and 40 undo steps. Raster dimensions are checked
before mask decoding. Missing mask data is an error, not an invented white mask.
Closing or opening another project prompts Save / Discard / Cancel for changes.
There is no autosave, automatic last-project restoration or persisted device
selection yet. Saving a project never enables hardware output.

## Validation and remaining work

Qt tests cover alpha/mask multiplication, transparent backdrops, all four blend
modes, layer order, soft brush/gradient values, independent mask copies, undo,
atomic invalid edits, separate editing contexts, layer panel actions, explicit
candidate selection, project round trips and failed loads/saves. The UI test
writes Layout/Effects screenshots to the build's `editor-artifacts` directory.
Offscreen tests can lack Windows fonts in the sandbox; native Windows tests
were also run to inspect legible screenshots.

Images, gradients/shapes as effect types, motion, particles, layout resizing/
rotation, component geometry editing/import, effect targeting, native controls,
hardware output and renderer-to-wire sampling remain future increments. The
current raster renderer is the basis for those paths, not proof of output parity.
