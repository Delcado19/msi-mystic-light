# Editor interaction implementation

## Result

New layer creates an empty transparent container. The layer tree expands to show
its independently editable objects. Toolbar flyouts add generators, shapes,
particles and imported images to the active layer. Rectangle and ellipse drags
create editable filled shape objects in new containers; outlined shapes and lines
remain raster objects. Brush strokes continue in the selected raster object.

Legacy layers retain their existing source, ID and exact parameters, and can also
contain objects. Their original source is represented by the layer row itself;
there is no destructive flattening or automatic project-file replacement.

Output targets belong to the containing layer. The layer's visibility, mask,
opacity and blend apply to the composed group; each object also has its own
visibility, mask, opacity, blend and source animation. Layer containers do not
offer source motion or geometry controls. Reorder objects within a layer, or move
objects between layers. Nested containers are deliberately unsupported and rejected.
Deleting a layer removes its objects in the same undo operation. Duplication copies
its objects with new IDs and preserves ownership.

## Pointer behavior

- Plain clicks with shape/gradient tools select the topmost painted object.
  Clicking empty space clears selection while retaining the active destination layer.
- Drawing begins after the platform drag threshold. A click never creates a layer.
  New objects remain transient until release; Escape leaves no object/history entry.
- Move selects objects directly; existing resize/rotation handles retain precedence.
- Gradient drags recolor the selected shape without replacing its geometry.
  Existing gradient endpoint handles remain editable and snapping applies to both ends.
- Raster brush dots, bucket fills and erasing retain deliberate paint behavior.
- Space held while dragging pans; releasing restores the selected tool.

## Rendering and output status

Per-frame compositions are cached by document revision, global time, effective
channel time and included root-layer IDs. Channels with identical compositions
share the same raster. Layout, Live Hardware and output sampling reuse the cache.
Hardware preview sampling visits each channel's actual placements rather than
resampling every component for every channel. Working pointer previews remain
transient and bypass the committed document cache.

The preview selector is visible only in Effects and explicitly labelled Preview.
It changes the inspected composition, not the destination of edits. Starting an
edit no longer silently resets this selector to the global composition. Global preview
includes all layers; a channel preview includes global layers plus its targeted
layers. Topmost tree entries are composited in front, both within and across layers.

Output summary counts active, intentionally paused, failed, connecting and stopped
channels separately, omitting zero secondary counts. Clicking the summary opens
Hardware. Selected-device details name channel states and report scheduler errors.
Live Hardware always shows the complete layout; Focus devices is removed.

## Identity and color sampling

The application toolbar uses the transparent LS header asset derived from the
selected banner. Hardware has no banner. About retains the original full artwork.
Zoom uses magnifier-minus/plus icons and Fit uses a fit-view icon with tooltips.

Screen eyedropper directly captures the desktop into a temporary selection overlay.
Click samples a color; Escape or right-click cancels. Captures are not saved.
Per-screen geometry maps logical coordinates to captured pixel dimensions, including
different DPI scales. Capture failure is shown explicitly in the picker.

## Persistence and limits

Schema 13 adds explicit `parentId` and `container` fields and editable rectangles.
Schemas 1 through 12 load with their original source identity and values. Older
readers reject schema 13. Invalid parents, nesting, malformed metadata and attempts
to disguise new fields under an older schema fail atomically. The existing total
budget of 64 source/layer entries and 16 megapixels of images remains in force;
containers count toward that budget. Files still save through atomic replacement.

Research and reference rationale:
[Effect editor interaction research](EFFECT_EDITOR_INTERACTION_RESEARCH_2026-10-27.md).
Validation and distribution evidence are recorded in CONTINUE.md.
