# Lighting Studio workspace redesign

The previous UI exposed almost every implementation parameter in one scrolling
Properties panel. Layer management, mask painting and effect settings competed
with each other, while empty documents lacked an actionable canvas.

Design references are Adobe's [workspace overview](https://helpx.adobe.com/photoshop/desktop/get-started/learn-the-basics/workspace-overview.html),
[Layers panel](https://helpx.adobe.com/photoshop/desktop/create-manage-layers/get-started-layers/work-with-the-layers-panel.html),
and Apple's [Pixelmator layers guide](https://support.apple.com/guide/pixelmator-pro/intro-to-layers-pixea4964220/mac).
These provide interaction patterns, not copied assets or application code.
The Emil design engineering, Apple design/HIG and animation skills informed this pass.

| Before | After | Why |
| --- | --- | --- |
| Layers embedded in an inspector scroll | Persistent left layer stack; source and mask thumbnails, effect subtitle | Identify each layer without opening its settings |
| All parameters visible simultaneously | Effect / Motion / Mask tabs; complete compact property grids | Show controls for the current task |
| Empty checkerboard with no next step | Actionable empty canvas and searchable visual effect gallery | Start with a visible result |
| Fixed 100% canvas | Fit on resize until manual pan/zoom; Ctrl+0 restores fit | Keep composition visible at compact sizes |
| Editing LED coordinate rows | Draggable LED geometry view; table retained in a precision tab | Arrange spatial data directly |
| Native colors as typed CSV | Color swatches, picker and add/remove controls; exact values optional | Use visual color selection |
| Large output dock steals canvas height | Output window opened from the toolbar | Keep the entire height for each authoring workspace |
| Gallery is the only visual source picker | All effect sources in a permanent two-column tool strip | One-click access without a modal |
| Properties overflow and colors use mixed input styles | Compact property grids, picker + RGB hex everywhere, Add color palettes | Show meaningful controls without array syntax |
| Layout displays an effect raster | Neutral layout with physical component geometry only | Keep layout and source authoring separate |

Apple HIG checks (local skill references): `layout.md > Best practices` says
"Make essential information easy to find by giving it sufficient space";
`sidebars.md > Best practices` supports a visible, optionally hidden sidebar;
`toolbars.md > Best practices` says "Choose items deliberately to avoid overcrowding".
`accessibility.md > Vision`, `typography.md > Best practices`, `color.md > Best practices`
and `designing-for-macos.md` informed readable system type, focus, labels and contrast.
`motion.md > Best practices` says "Add motion purposefully, supporting the experience
without overshadowing it". Native Qt controls retain keyboard behavior. Tool icons
have names and shortcuts; selected source/mask is outlined independently of layer name.

## Motion audit, opportunities and review

Stack: Qt Widgets/QPainter with an explicit 33 ms effect-preview timer. No web
animation library, CSS transitions or new UI animation engine is introduced.

| Location | Purpose | Frequency | Decision |
| --- | --- | --- | --- |
| Buttons and tool selection | Feedback | Tens per day | Immediate hover/pressed/selected color states |
| Canvas transforms and mask strokes | Direct manipulation | Hundreds per day | Follow pointer immediately; no easing or lag |
| Layer selection and inspector tabs | Navigation | Hundreds per day, keyboard included | No transition |
| Effect preview | Explain the actual lighting result | Explicit Play/Pause | Existing clock-driven renderer; no automatic playback |

No additional motion opportunity survives the frequency/function gate. Rejected:
layer reorder springs would obscure the exact stacking result; inspector crossfades
would delay keyboard selection; canvas zoom springs would separate pointer and
position; animated empty-state decorations would compete with lighting previews.
The calm workspace needs no decorative animation. Manual Play/Pause provides
control over effect motion; stopping a preview is immediate. This also avoids
introducing vestibular UI motion or a new reduced-motion preference to maintain.

## Output connection and verification

The workspace edits the existing LightingDocument. The same preview signal feeds
OutputPanel -> prepareOutput -> OutputScheduler -> OutputSession -> real driver
module. Main installs the panel and connects the toolbar action and status. There
is no independent mock editor or second effect state. Conflict resolution,
exact-path discovery and report consent still precede opening hardware.

Regression checks cover gallery search/cancel/add, compact canvas visibility,
effect identification, precision disclosure, mask tools, live preview/undo,
component geometry dragging and channel settings. Output tests use the actual MSI
serializer and fake transport to verify submitted frames, live edits, stop-on-configuration-change, failures
and explicit reconnect. Effect/mask edits update approved software RGB payloads
live; channel identities, modes, counts and native commands remain frozen until
reviewed again. Native Windows captures inspect the actual widgets.
Physical-device/color calibration and clean-machine deployment remain separate
qualification; no hardware writes were performed for the redesign.

The current implementation restores the original studio's header sidebar, central
workspace and selected-item inspector. Layout is the startup view. Read-only
discovery populates all supported headers automatically; users attach fan/strip
quantities directly rather than selecting raw endpoint/profile pairs. Saved chains
remain editable offline. Effects retains its independent canvas, tools and layers.
Output controls open on demand without reserving any permanent canvas height.
Session state format 2 discards obsolete toolbar/dock arrangements while retaining
projects and window geometry.
