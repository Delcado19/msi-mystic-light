# Effect editor interaction research

Date: 2026-10-27. Status: research proposal; implementation and compatibility details are recorded in EDITOR_INTERACTION_IMPLEMENTATION_2026-10-27.md.

## Scope and evidence

The owner reports accidental layer creation, ineffective shape gradients, unclear
channel previews, apparent stacking errors and stuttering after channel routing.
This review combines current source inspection with official editor manuals.
No application code, personal project or hardware state was changed.
The running executable has not been compared with the current source in this review.

## Findings in current source

- `EffectLayer` combines one procedural source, raster image, fill, mask, motion
  and channel targets. It has no collection of independently selectable objects.
- `LightingCanvas::mousePressEvent` dispatches non-Move tools to drawing before
  normal effect selection. `beginDrawing` may create a layer during mouse press
  when the selected source cannot accept the operation. Selection and creation
  therefore have different rules depending on the active tool and source.
- Current source already contains `gradientFill` and shape-local gradient endpoints
  in the canvas, document and renderer. The reported installed-build failure must
  be reproduced before treating this as entirely missing functionality.
- `LightingWindow::updatePreview` sends the global composition to Layout and the
  selected channel composition to Effects. The channel selector does not select
  a component or set the destination of a newly created effect.
- `sampleChannels` renders every software channel separately if any layer has
  restricted routing. This can increase work sharply; timing and call counts are
  required before identifying it as the measured stutter cause.
- Rendering traverses layers forward; the layer list displays them in reverse.
  Those conventions are compatible with topmost UI rows being composited last.
  An inverted stack is not established by this static inspection. Compare global
  preview, channel preview and sampled output using the same frame and targets.

## Reference patterns

1. Krita distinguishes paint, vector, fill and group layers. New paint layer is a
   direct action. Raster marks and editable vector objects have different semantics.
   https://docs.krita.org/en/user_manual/getting_started/basic_concepts.html
   https://docs.krita.org/en/reference_manual/layers_and_masks/paint_layers.html
2. After Effects permits multiple shapes in one shape layer. Shape groups contain
   geometry, fills, strokes and transforms. A gradient can be a shape fill rather
   than another independently composited source.
   https://helpx.adobe.com/after-effects/using/creating-shapes-masks.html
   https://www.adobe.com/learn/after-effects/web/add-animate-shapes
3. Premiere exposes effects for the selected clip in Effect Controls; expandable
   groups and effect ordering keep editing attached to the selected content.
   https://helpx.adobe.com/premiere/desktop/add-video-effects/apply-video-effects/view-effects-in-the-effect-controls-panel.html
4. Inkscape separates drawing tools, selection, navigation and contextual tool
   controls. Its conventions are useful references, not a requirement to copy
   every shortcut or tool mode into a lighting application.
   https://inkscape.org/de/en/doc/tutorials/basic/tutorial-basic.html/

## Recommended content model

Use Layer -> Objects -> Fill / modifiers, rather than one layer per drawn item.

- A layer organizes objects and owns output targets, visibility, opacity and blend.
- An object is an editable shape, raster surface, imported image or generator.
- A shape owns its geometry and solid or gradient fill.
- Motion and color modifiers attach to a selected object; a layer may expose
  shared transforms separately and explicitly.
- Multiple procedural generators are objects, not indistinguishable modifiers.
- Object order determines composition within a layer; layer order determines
  composition across layers. Topmost visible row means visually in front.
- Routing initially belongs to the layer. Objects requiring different destinations
  belong on separate layers. Avoid object-level routing until a real need exists.
- Existing layers migrate to a container with one equivalent object. Preserve IDs
  where referenced, colors, transforms, masks, animation, targets and exact values.
  Version the schema and reject unsupported saves; never silently flatten content.

This requires a document/rendering/undo migration, not only a toolbar adjustment.
A smaller interim patch can repair selection, gradients and creation while keeping
one source per layer, but it must not be presented as multi-object editing.

## Interaction contract

| Action | Result |
| --- | --- |
| New layer | Create one transparent, empty layer; no effect chooser |
| Plain click with selection/shape/gradient tools | Select the topmost hit object; never create a layer |
| Click empty canvas | Clear object selection; retain active layer |
| Drag with selection tool | Move selected object; empty-space drag selects objects |
| Drag with shape tool | Create an object in the active layer after drag threshold |
| Drag with gradient tool on selected shape | Edit its fill, preserving geometry and layer count |
| Gradient on empty active layer | Create a gradient object only after a deliberate drag |
| Brush stroke | Paint on the active raster object; a dot is a deliberate paint action |
| Brush on incompatible object | Explain the incompatibility and offer an explicit raster object action |
| Escape | Cancel gesture and temporary previews without document/history mutation |
| Space held + drag | Pan temporarily; releasing restores the active tool |
| Tool button | Select a tool; do not insert content merely by selecting it |

An unconditional click-to-select rule conflicts with brush dots and bucket fills.
Keep drawing tools explicit. Shape/gradient gestures can distinguish click from
drag; raster tools must retain their normal paint behavior. Avoid hidden modifiers
that users must discover before they can select or draw reliably.

## Interface and identity

- Compact LS mark and wordmark in the application shell; no banner on Hardware.
  Use the full supplied scene in About or a welcome view. Prepare an actual compact
  branding asset instead of squeezing the full scene into a corner.
- Graphite work surfaces, cyan selection and restrained gaming identity. Reserve
  bright color for light content, active controls and meaningful output state.
- Left: expandable layers and objects. Center: canvas. Right: selected object's
  properties, with visible layer output targets. Contextual tool options above.
- Object selection and canvas selection stay synchronized in both directions.
- Zoom out/in use magnifier-minus/plus icons. Fit uses a distinct fit-view icon.
  Keep tooltips, accessible names, focus indicators and a readable zoom percentage.
- Effects selector label becomes `Preview: All outputs / <channel>`; Layout hides it.
  A prominent preview scope indicator prevents channel composition being mistaken
  for the complete project. Preview scope never silently changes routing.
- Keep global playback distinct from channel stop/resume. Do not add a full video
  timeline merely to imitate a video editor; current animation needs come first.

## Acceptance and implementation gates

1. Reproduce routing with two overlapping opaque sources and two channels. Assert
   correct UI stack, global frame, channel frames and sampled LED values.
2. Measure global/scoped preview and sampling with identical content, time and
   channel counts. Count duplicate renders. Reuse identical compositions within a
   frame and avoid rendering hidden previews where behavior permits.
3. Verify clicks, drags below/above threshold, Escape, undo/redo and locked objects.
   Selection and canceled gestures must not add layers or mark the document dirty.
4. A shape gradient must remain clipped to the shape, follow its transform, survive
   save/load and produce one undoable operation without creating another layer.
5. Verify multiple independently selectable objects, object/layer stacking and
   migration equivalence before advertising multi-object layers.
6. Inspect compact branding and icon controls at minimum window size and 125%,
   150% and 200% DPI. Verify focus, tooltips and keyboard alternatives.

Recommended order: diagnose rendering/routing; agree content and interaction model;
implement the document transition and editor behavior; finish shell branding/icons.
Owner approval of this proposal is the checkpoint before implementation.
