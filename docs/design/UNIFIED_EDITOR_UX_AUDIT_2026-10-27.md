# Lighting Studio: whole-interface and unified-editor review

Date: 2026-10-27. Status: UI implementation approved and delivered locally; see UNIFIED_EDITOR_IMPLEMENTATION_2026-10-27.md. Windows service work is deferred. Original evidence below describes the audit snapshot.

## Evidence and limits

Reviewed the current working tree, including uncommitted termination/color-picker
fixes. Read LightingWindow, LightingCanvas, EffectRenderer, StudioWidgets,
ParameterSpinBox, ColorPicker, ChannelEditor, ComponentEditor, OutputPanel and
PawnIoRam, plus the previous GUI and editor interaction reports.
Inspected docs/images/studio-workspace.png and hardware-workspace.png. These are
existing repository captures, not new runtime evidence. No application launch,
hardware output, process termination, benchmark or new test run occurred.
The personal project and existing implementation changes were not modified.

Applied ui-ux-pro-max interaction guidance, Apple HIG desktop principles,
emil-design-eng interaction/motion guidance and design-taste-frontend app-screen
review guidance. Apple visual conventions and web-specific CSS prescriptions are
not requirements for this Windows Qt application. A browser inspector cannot
validate native Qt geometry. No contrast or frame-rate measurements are claimed.

## Main decision

Merge Layout and Effects into one Editor workspace. Keep Hardware separate.
The user's task is one loop: compose light, arrange sampling geometry, inspect
the result, then adjust either side. Current workspace navigation interrupts it.
The lower hardware canvas is explicitly preview-only in LightingWindow, so its
inability to arrange components is a real interaction boundary, not just naming.

Do not simply enable both object systems on every click. Effects and components
occupy the same coordinates; unrestricted hit testing would make selection worse.
Use two explicit selection targets inside the editor: Effects and Components.
Preserve each target's selection when switching. Keep time, camera, zoom, active
layer and output preview scope stable. Both targets share the same document.

## Proposed workspace

Header: compact Lighting Studio branding at the far left, then Editor / Hardware,
then project actions. Full scene artwork belongs in About or a welcome surface.
Left: Layers and Components tabs. Layer rows expand into editable objects;
component rows group placements by their output channel. Selecting either tab
sets the matching selection target. Show the active target near the canvas too.
Center: one primary canvas with effects and optional component overlays.
Right: selection properties; never unrelated controls for a previously selected
object. Show object type and parent layer, or component and output channel.
Bottom: preview playback, output summary and contextual interaction hints.

View choices: Effect / Composite / Components. View visibility is independent
of the selection target. Hide-and-lock unavailable targets visibly rather than
silently accepting gestures against invisible objects. Preserve a split preview
as an optional view, not the mandatory way to work. The current splitter changes
space allocation; it does not establish a unified editing context.

Example workflow: create Layer 1, draw two objects, choose its output channels,
show Composite, select Components, arrange a fan group, select Effects, adjust
the gradient. No workspace change or animation restart is required.

## Findings and priorities

| Priority | Area | Evidence / problem | Required outcome |
| --- | --- | --- | --- |
| P0 | Workflow | Separate canvases and preview-only hardware pane | Arrange components within the editor |
| P0 | Selection | Move press calls selectAt before dragging unless a handle is hit | A selected covered object remains draggable through its handles |
| P0 | Gradient | G can edit an existing source; sources menu reuses its icon | G creates a new object; visible grips edit existing gradients; distinct generator icon |
| P0 | Motion geometry | effectTransform applies drift; gradient rendering retains drift as texture motion | Bounds and handles describe actual geometry at the same preview time |
| P0 | Playback | Gesture start stores a boolean, pauses and sometimes resets time to zero | Capture playback state once per gesture; restore after commit/cancel without time jumps |
| P0 | Picker | Popup is clamped on initial opening; restoration and RGB expansion resize/show it | Re-clamp after every size/visibility transition; test screen edges and mixed DPI |
| P1 | Routing | Preview selector and layer channel targets are separate concepts | Name preview scope and destinations independently; neither silently changes the other |
| P1 | Snapping | Grid and 15-degree rotation exist; no alignment command/guides found | Distinct grid, alignment and distribution commands; visible guides |
| P1 | Layer model | Containers and child objects exist; legacy roots retain implicit sources | Consistent layer/object terminology; no forced layer per new shape |
| P1 | Shortcuts | Labels currently use H in event filter and tooltip | Restore requested L consistently; check conflicts and text-field focus |
| P1 | Hardware | Add/Remove/Refresh lack context in isolation | Add components, remove assignment and scan hardware clearly differ |
| P1 | Output | Preview pause, per-channel pause and global stop differ | Explicit Preview and Device output states; stopped output does not imply LEDs are off |
| P2 | Visual hierarchy | Multiple toolbar rows and split canvas consume working area | Compact stable shell, larger canvas and contextual options |
| P2 | Component authoring | Coordinate table exposes up to 17 significant digits | Friendly displayed units without destroying stored precision |

The playback and picker rows identify suspicious paths, not reproduced root causes.
The older picker hotfix already separates screen overlays; do not discard it.
Existing zoom/fit actions already have icons in source and repository screenshots.
Treat reports of missing icons as a build/version check before changing them again.

## Editor interaction contract

| Input | Proposed behavior |
| --- | --- |
| V | Select/move within the active target |
| Plain click | Select the visible hit object; empty space clears selection |
| Shift-click | Add/remove from selection |
| Empty-space drag with V | Marquee selection in the active target |
| Drag selected handle | Transform that selection even when another object covers it |
| Right-click overlap | Offer named objects at the pointer; avoid guessing the intended object |
| Shape-tool drag | Create an object in the active layer after the drag threshold |
| Shape-tool click | Select; do not create an empty object or undo entry |
| G drag | Create another gradient object, as explicitly agreed with the owner |
| Gradient grip drag | Edit that gradient; preserve object and layer counts |
| Shape fill property | Choose Solid / Gradient separately from creating gradient objects |
| Brush / eraser | Operate on the explicit raster or mask target; preserve intentional brush dots |
| Space-drag / middle-drag | Pan temporarily without losing tool or selection |
| Escape | Cancel gesture and restore preview state; no document edit |
| Delete / Ctrl+D | Delete / duplicate current selection, with text-input focus protection |
| Arrow / Shift-arrow | Nudge selection by 1 / 10 canvas units; document snap interaction |
| L held | Temporarily show component labels; eye toggle remains the saved preference |

Do not advertise uniform click-to-select for brush/fill tools: their clicks draw.
Selection must also be available through the object tree without changing content.
Hidden or locked items do not intercept canvas hits. Multi-selection transforms,
locking and overlap menus are proposed acceptance conditions, not verified features.
Retain Alt as the existing temporary snap bypass; do not overload it casually.

Alignment: expose left/center/right, top/middle/bottom and equal distribution.
State whether the reference is selection bounds, key object or canvas. Default to
selection bounds; distribution needs at least three items. Keep device wire order
unchanged. Apply the same transform semantics to effects and component groups.

## Layers, colors, numeric controls and appearance

Keep Layer -> Objects. Layer owns routing, visibility, opacity and blend; objects
own source, geometry and motion. A gradient fill belongs to its shape. A separate
gradient object is independently composited. Display both relationships clearly.
New layer stays empty. Generator insertion and tool selection must look different.
Root-level legacy sources require compatibility handling, not silent flattening.

Keep the HSV square/hue control, hex entry, optional RGB and recent colors.
A color wheel would not solve the current workflow defects. Improve popup bounds,
keyboard operation, invalid-input feedback and Apply/Cancel behavior first.
Palette stops need clear selection, position and opacity controls. Distinguish
canvas eyedropper from screen eyedropper; the latter samples outside the app.

Use integer display units with units visible: percent, degrees, px, px/s and cycles
per minute. ParameterSpinBox already preserves precise legacy values underneath.
Do not replace meaningful signed motion ranges with an arbitrary universal 0-100.
Use symmetric speed ranges with a clear zero; logarithmic mapping may suit large
duration ranges only after checking actual useful values. Keep manual entry and
reset available. Slider drag should be one undo operation, not one per tick.

Retain graphite surfaces and cyan selection with the approved icy LS identity.
Reserve strong RGB for actual light content and status colors for named states.
Avoid decorative animation on the working canvas. Use brief panel feedback only;
never animate manipulation handles behind the pointer. Respect reduced motion.
Icon buttons retain accessible names, tooltips and shortcuts. Verify Windows
keyboard/focus behavior instead of copying macOS shortcuts or touch target sizes.

Hardware owns discovery, physical connections, channel mapping and diagnostics.
Editor Components owns spatial placement. The custom component/LED editor remains
an advanced dialog; distinguish editing a definition from moving an instance.
Keep discovery failures, permission failures and disconnected saved devices distinct.
Conflict termination must show pending/failure/cancel states and verify original
process identity. Do not silently stop other lighting software.

## Elevation and stable hardware inventory

An unelevated process cannot grant itself administrative access without an already
authorized privileged component. Installation can request UAC once to install a
small hardware service. Later GUI sessions communicate with that installed service
without another elevation prompt, subject to its access policy and system policy.
Installing/updating privileged components can require UAC again.

PawnIoRam::pushColors performs SMBus transfers for every frame. Therefore caching
scan results does not remove the need for privileged runtime writes. Current code
opens PawnIO in-process; no service implementation was found in the inspected paths.
Current verified-address discovery cache is process-local, not a persistent inventory.

Recommendation: normal-user GUI and plugins; narrowly scoped PawnIO service;
HID/NVAPI remain direct where device permissions permit. Validate service account
access to PawnIO before choosing LocalService or a more privileged account.
Use protected installation paths, explicit local IPC permissions and authenticated
client authorization. Accept validated device/color commands, never arbitrary DLL,
module-path, register-write or shell requests from the GUI. Preserve SMBus locking.

Persist configured identities, mappings and last-known capabilities. On startup,
reconnect known devices and perform cheap presence/capability checks; do not assume
cached paths prove identity or write to an unverified replacement. Manual Scan
performs broader discovery. Mark absent devices offline without deleting layout.
Handle unplug/replug and resume. A cached inventory is an optimization, not proof
that hardware is unchanged. A scan-only elevated helper would not support later
RAM animation unless a privileged host remains available for those writes.

## Implementation order and acceptance

1. Fix selection, gradient creation/handles, playback and picker bounds first.
2. Introduce the unified editor while preserving project data and offline editing.
3. Add alignment, consistent shortcuts and contextual inspector/tool options.
4. Validate complete tasks at minimum window size and 125/150/200% DPI.
5. Implement and independently validate the privileged hardware boundary.

Task test: two layers, two gradients in one layer, two separately routed channels;
arrange components in Composite, edit a covered object, adjust its fill and motion,
undo/redo, cancel a gesture, save/reload. Assert layer/object counts, geometry,
channel destinations, playback continuity and equivalent rendered LED samples.
Picker tests include RGB expansion, accept/cancel on another display and edge
anchors. Performance tests compare identical projects and preview scopes; target
responsive manipulation, but do not claim 60 fps without measurement.

## References

- Krita shape selection and fill controls: https://docs.krita.org/de/reference_manual/tools/shape_selection.html
- Krita selection concepts: https://docs.krita.org/en/user_manual/selections.html
- Windows UAC: https://learn.microsoft.com/en-us/windows/security/application-security/application-control/user-account-control/how-it-works
- IPC security: https://learn.microsoft.com/en-us/windows/win32/ipc/named-pipe-security-and-access-rights
- Service accounts: https://learn.microsoft.com/en-us/windows/win32/services/localsystem-account
- Local skill references: apple-hig-review/references/hig/{accessibility,layout,
  typography,color,designing-for-macos,toolbars,focus-and-selection,pointing-devices}.md;
  especially focus-and-selection (selection feedback), toolbars (task commands)
  and layout (stable structure). design-taste-frontend/references/app-screens.md
  and critique.md; ui-ux-pro-max search: dragging movements / keyboard alternatives.
