# Unified Editor implementation

The owner approved the UI audit, with Windows service work explicitly deferred.
This change extends the existing Qt editor and document rather than replacing them.
Earlier Layout/Effects navigation and gradient-creation instructions are superseded
by this document. Hardware output remains in the existing process.

## Workspace and interaction

The application header starts with the compact approved LS branding, followed by
Editor and Hardware. Inside Editor, Components and Layers choose the selection
target and contextual navigator/inspector. Existing internal canvas pages remain
for output integrations; they are no longer separate top-level workspaces.
Camera position/zoom, effect selection, component selection, animation time and
preview channel survive target changes. Composite is the default canvas view;
Effect only and Components only disable the unavailable selection target.
Split hardware preview is optional in View. Its divider retains the old behavior. Canvas view and alignment also remain reachable through View when toolbar widgets overflow. Inspector layouts grow scroll content instead of compressing fields at high DPI (Qt 6.8 QScrollArea size-constraint guidance).

Layers contain independently selectable objects. G drags create new gradient
objects, including when a gradient or shape is selected. Gradient endpoints edit
existing gradients with G or V. Shapes choose Solid fill or Gradient fill in Source;
this preserves geometry and avoids conflating fill editing with object creation.
Generator insertion has a separate icon. New layer still creates an empty layer.

V selects/moves, Shift-click toggles selection and an empty-space drag selects
contained objects. A click on an overlapped object chooses the topmost hit; a drag
can retain the selection chosen in the tree. Grips have priority over hit testing.
Right-click lists hit objects by name. The trees support extended selection.
Multiple effect objects and components can be moved, resized and rotated together.
Selection operations remain separate from document history; Escape cancels drafts.
Brush dots and fill clicks retain their drawing semantics.

Align offers left/center/right, top/middle/bottom and horizontal/vertical center
distribution, relative to selection bounds. Align requires two items; distribution
requires three. Smart guides align moving edges/centers within six viewport pixels.
They are enabled in the unified editor and can be disabled in Align. Alt bypasses
both grid and smart guides. Grid spacing and 15-degree rotation are unchanged.
Neither arranging components nor aligning them changes physical LED wire order.

View provides Lock selection and Unlock selection. Locked objects remain visible
and output remains active; select them through the navigator to unlock. Layer locks
also protect children. Ctrl+D duplicates the active target selection; Delete removes
it and cleans routing references. Validation failures are shown in the status bar.
An attempt to delete a parent containing locked children is rejected visibly.

Hold L to reveal component labels without changing the saved eye-toggle preference.
The line tool moves to I to avoid the previous L conflict. Text controls retain
their typing keys. Space-drag and middle-drag pan temporarily.

## Rendering, color and numeric behavior

Gradient drift remains texture movement; it no longer moves geometric handles or
linked masks. Endpoint editing accounts for animated rotation/zoom. Editing pauses
the current preview frame without resetting the animation clock; playback state is
captured once per gesture and restored on commit, cancellation or view changes.
Composite reuses component artwork and LED sampling rather than creating another
hardware preview implementation. It remains a software preview, not device readback.

Color picking keeps HSV, hex, optional RGB and recent colors. The popup clamps to
the owner's available screen on showing/resizing, including sampler restoration and
RGB expansion. Invalid hex input shows an explanation. Independent screen overlays
and their previous accept/cancel behavior remain intact. Component coordinate table
display rounds with an approximation marker; exact underlying/editable values remain.

## Project compatibility

Schema 14 adds required Boolean `locked` values to layers/objects and placements.
Schemas 1 through 13 load with unlocked defaults. Lock fields under older schema
numbers and malformed/missing schema-14 lock values are rejected atomically.
Older application versions reject schema 14 instead of discarding editing locks.
Opening an older project does not rewrite it. Existing IDs, precise values, object
ownership, channel targets, masks and physical mappings are preserved.

## Verification

Regression coverage includes two gradients in one routed layer, target/view changes,
camera and selection continuity, playback restoration, group movement, alignment,
distribution, shift/marquee selection, covered-object dragging and overlap selection,
smart-guide/Alt behavior, gradient drift/handles, lock save/load, duplication/deletion
and picker bounds. Existing renderer, routing, legacy-schema, output and startup
tests remain part of the acceptance gates. Final results are recorded in CONTINUE.md.
Native Windows screenshots supplement tests; offscreen captures are not promoted.
No real hardware writes, process termination or Windows service installation is
part of these tests. Physical hardware behavior still requires owner validation.
