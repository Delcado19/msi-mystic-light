# Studio interaction and motion review

This pass restores the original studio's searchable hardware picker and draggable
gradient editor while keeping Layout and Effects as separate workspaces in one app.
The working references were the original worktree's HardwarePicker/HardwareLibrary
and GradientStops, Emil design engineering, Apple design/HIG, and the animate,
find-animation-opportunities, improve-animations and review-animations skills.

| Before | After | Why |
| --- | --- | --- |
| New layer opens an effect-selection dialog | New layer immediately creates a transparent paint source | Layer creation has one predictable result |
| Twenty effect buttons masquerade as tools | Standard pencil, line, rectangle, ellipse, gradient, fill, eraser, move and crop | Match established drawing-app behavior |
| Shapes and particles hidden in a general gallery | Direct Shapes and Particles toolbar flyouts; Other effects only holds gradients/patterns | Make source choices discoverable without duplication |
| Only generic fan/strip quantities | Searchable product cards, categories, quantities and original M25 definitions | Restore the original hardware-selection workflow |
| One hex row per gradient color | Draggable color-stop strip, RGB sliders, selected-color picker, Add/Remove | Edit the gradient spatially and continuously |
| Tiny composition thumbnail; hardware in another view | Persistent live component preview below the effect canvas | See the effect's purpose while editing |
| Advanced motion only has numeric fields | Sliders beside every advanced value and explicit all-visible-layer scope | Keep adjustment and animation scope consistent |
| Every motion number shown at once | Main motion/particle sliders; explicit Advanced motion disclosure | Prioritize controls that are used most |
| Full-width foreground hex field | Bounded color control; contextual tool options | Keep workspace chrome proportionate |

## Design principles checked

Apple HIG local references reviewed: layout.md (Best practices, Visual hierarchy),
typography.md (Ensuring legibility, Conveying hierarchy), color.md, accessibility.md,
designing-for-macos.md, toolbars.md, sidebars.md and motion.md (Best practices,
Providing feedback). The resulting choices are familiar tool semantics, enough space
for the active canvas and inspector, direct manipulation, system typography,
keyboard access and continuous feedback. Native Qt controls retain focus behavior;
custom stop strips expose keyboard selection and Delete. Hardware attachment remains
separate from permission to write reports.

## Motion audit and implementation

Qt Widgets/QPainter supplies the existing rendering and input system. No animation
library, decorative spring, delayed tool switch or animated layout was added.

| Before | After | Why |
| --- | --- | --- |
| Preview requires discovering Play first | Preview starts playing; Pause remains explicit | Make lighting motion immediately observable |
| Static particle thumbnail in the preset chooser | Only the selected particle preview updates at 100 ms intervals | Explain the actual source without animating the whole gallery |
| Effect selection has no playback control | Animate selected preset can be disabled; system style controls its default | Optional motion; no animation is required to select a preset |
| Transforming an animated source keeps changing time | Pause/reset during the gesture; resume the previous playback state | Stable base coordinates for editing |
| Frequent tools/keyboard actions risk cosmetic delay | Immediate state change and pointer tracking | Drawing is a high-frequency functional interaction |

Frequency gate: drawing, sliders, tools and keyboard actions reject ornamental
animation. The surviving motion is functional lighting preview (state indication)
and optional selected-preset preview (explanation). Both are interruptible. Canvas
and stop drags follow the pointer directly; no easing is interposed. Source/mask
selection, tab changes and layer reorder remain immediate. No transform/opacity
entrance effect needs a separate motion token or reduced-motion implementation.

Review verdict: approve after native Windows checks. Active particle sliders fit,
gradient stops/RGB controls and the hardware pane remain visible, and compact sizes
retain both authoring and preview areas. The actual widget screenshots are
[workspace](images/studio-workspace.png), [hardware picker](images/hardware-picker.png)
and [motion](images/lighting-properties.png). No physical-device color/readback or
clean-machine qualification is implied by these captures.

## Data and transport boundaries

Paint gestures and gradient-stop edits use document validation and undo. Schema 11
stores paint pixels and explicit stop positions; older projects retain their prior
rendering. Installed component metadata is bounded, validated, normalized and
selected data only; no external scripts or images are executed/copied. Existing
output review, exact endpoint matching, report consent, live RGB scope checks and
stop-on-topology-change remain intact. UI tests use fake endpoints/transports.

## Regression repair

| Before | After | Why |
| --- | --- | --- |
| Layout conceals the effect raster | Animated raster beneath physical groups | Show where each component samples color |
| Group movement without visible frame | Dashed group frames and resize/rotation handles | Direct manipulation needs visible affordances |
| Component library crowds Layout | Hardware workspace contains library and real output controls | Keep each workspace focused on its task |
| Gradient direction needs another transform tool | Persistent draggable gradient arrow | Follow Paint.NET's established control-nub interaction |
| Fill requires identical pixel values | Connected fill with edge tolerance and explicit visited pixels | Handle antialiased colors safely |
| Technical Motion labels and detached cycle rate | Action-oriented labels, examples in tooltips, palette rate beside colors | Explain the visible result and its activation |
| MSI-only host loses keyboards/mice | Original peripheral encoders under the reviewed output pipeline | Restore hardware functionality alongside the editor |

References: https://mail.getpaint.net/doc/latest/GradientTool.html and
https://mail.getpaint.net/doc/latest/MoveTools.html. Tool gestures remain immediate;
no ornamental transitions are added to frequently used editing actions.

## Drawing and hardware regression pass (2026-10-24)

| Before | After | Why |
| --- | --- | --- |
| Fill leaves original colored antialias edges | Unpremultiplied color matching preserves original coverage | Recolor the complete painted region |
| Gradient gesture replaces a shape with a background | Independent shape-local fill arrow; paint uses SourceIn | Preserve the object being edited |
| Paint palette has no visible activation | Enable color animation starts cycling and playback | Immediate feedback |
| Hardware attachment without an inventory | Shared Layout tree model and selection, Add/Remove below | One authoritative hardware overview |
| Tiny LED-only preview with unclear timing | Full composition backdrop and live time, shared sampling | Relate canvas movement to component colors |

No decorative animation was added to repeated drawing gestures. Undo/redo use a
continuous curved arrow path with a visible arrowhead. Real hardware writes retain
the existing exact-report review rather than starting from a drawing operation.

## Gradient/output feedback correction (2026-10-24)

Travelling gradients now interpolate a closed palette in source coordinates rather
than wrapping a rendered rectangle. The arrow remains the spatial direction. No
new animation control or dependency was added. Starting an animated output resumes
paused playback after approval. Hardware status and the global output tooltip show
actual state instead of a permanent stopped message. Blend tooltip explains that a
bottom layer has no visible backdrop; asymmetric RGB and inspector tests verify all
four modes without changing the existing Qt compositor.

## Direct group dragging and automatic output (2026-10-24)

The visible group label is its drag target; paint and hit testing share the same
bounds calculation. It selects the complete chain before member/transform hit tests.
Canvas output now defaults to automatic after restoration/discovery, without report
approval UI. Configuration changes reconnect; Stop/Resume remain explicit and Stop
cancels queued reconnects. Status and fault feedback remain visible.

## Layout feedback and hardware output controls (2026-10-24)

| Before | After | Why |
| --- | --- | --- |
| Group selection captures member clicks | Member bodies select individually; labels drag groups | Selection follows the target under the pointer |
| Hardware samples old positions until release | Temporary layout transforms drive output immediately | Physical feedback matches the visible gesture without undo spam |
| Output status duplicates Hardware navigation | Read-only status beside Undo/Redo | A status indicator has one purpose |
| 100-pixel output list above unused space | Controller/channel tree consumes available height | Small inventories remain readable |
| Globally styled Resume command | Neutral, selection-scoped Stop/Resume enabled by actual state | Controls explain what can happen now |

No decorative animation is added to continuously updated device state. Selection
is preserved across status refreshes, and transport errors remain available in
status text/tooltips. Native firmware motion is not stopped by pausing submissions.

## Endpoint list correction and restored backends (2026-10-24)

| Before | After | Why |
| --- | --- | --- |
| Right side repeats the header inventory | Flat Endpoint / Profile / State table | Technical connections retain their distinct purpose |
| IOShield exposes Native only | Restored shared-color Canvas fallback | Preserve the working original output path |
| GPU adapter absent | Qualified NVAPI adapter restored | Preserve the original supported output path |
| Unavailable native targets disappear | Not detected rows | Missing endpoints remain visible |

Left selection scopes the action to its header; clicking an endpoint row scopes
it to that connection. No new decorative animation or duplicated hardware hierarchy.

RAM now uses the user-approved direct PawnIO transport. The previous OpenRGB
helper remains removed. Missing access rights are shown as Run as administrator
in the flat hardware table; actual RAM lighting still needs physical validation.
