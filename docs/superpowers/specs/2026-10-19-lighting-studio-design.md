# Lighting Studio — Design Doc (v2)

## Intent and why

v1 (`worktree-v1-skeleton`, merged into the plan at
`docs/superpowers/plans/2026-10-18-v1-skeleton.md`) shipped a tray-only app
that drives the MSI motherboard's RGB headers from a hardcoded/JSON-edited
config. There is no UI to actually see or change anything — you edit
`settings.json` by hand. This project adds that UI, and in the same pass
extends control from "the motherboard" to "everything in the room that
lights up": the motherboard headers plus the keyboard and mice already in
use.

The centerpiece is a SignalRGB-style canvas: a 2D scene containing every
controllable device, arranged to mirror where they actually sit. A running
effect is visible *in the canvas itself* — you can see where a rainbow wave
currently is, or drag a radial gradient's center point — and that visible
position is what the real hardware then shows. The canvas is the editor,
not just a settings form.

## Goals

- One window ("Lighting Studio") to see and control every light-capable
  device: the 6 existing motherboard channels, plus keyboard and mice.
- A 2D canvas, laid out to mirror the user's real desk/tower (chosen over
  an abstract grid — confirmed via mockup).
- Effects are visible and positioned in the canvas: dragging an effect's
  reference point (e.g. a radial gradient's center) changes what the
  canvas — and therefore the hardware — shows, live.
- Per region (= per device/zone), independently choosable: either "follow
  whatever the global canvas effect produces at this position" or "run its
  own independent effect/static color", simultaneously across different
  regions.
- New devices supported with the same safety discipline as v1: no
  unconfirmed byte sequence to real hardware without showing the exact
  bytes and getting plain-language confirmation first.
- Profiles: save/restore more than one full configuration, switchable
  from the tray without opening the main window.

## Non-goals (v1 of this feature)

- Audio-reactive effects (needs its own audio-capture pipeline).
- GIF/image/video playback as an effect source.
- Layer/compositing of multiple effects onto one region (OpenRGB's
  `Layers`/`Stack`/`Mask`).
- SignalRGB-style "Game Integrations" (per-game event hooks) — not
  relevant for a personal standalone app.
- Discovering/adding arbitrary new third-party devices from a library.
  The device roster is fixed to what's actually owned (below); there is
  no "add device" flow.
- Any additional RAM/GPU/fan controllers — confirmed everything beyond the
  motherboard headers is driven through the motherboard's own JARGB
  headers already; no separate controller exists to support.

## Anti-goals

- A settings form with no visual feedback (that's what v1 already is;
  this project exists specifically to replace that experience).
- Depending on vendor software being installed/running (Razer Synapse/
  Chroma Connect, SteelSeries GG). Confirmed reason: user doesn't want to
  install Chroma for one device, and the installed SteelSeries GG/Prism
  suite is currently unstable (open/close crash loop). The app must talk
  to every device directly over HID, the same way v1 already does for the
  motherboard.

## Device roster (confirmed)

| Device | Role | Protocol source |
|---|---|---|
| Mainboard: JARGB1, JARGB2, JARGB3, EZConnector, IOShield, JRGB1 | existing, v1 | already implemented (`src/device/MsiMotherboardDriver.*`) |
| Roccat Vulcan 100/120 Aimo | keyboard | OpenRGB `Controllers/RoccatController/RoccatVulcanKeyboardController` (GPL-2.0-or-later) |
| Roccat Kone Aimo / Aimo 16K | mouse | OpenRGB `Controllers/RoccatController/RoccatKoneAimoController` (GPL-2.0-or-later) |
| Razer Basilisk V3 | mouse | OpenRGB `Controllers/RazerController` (GPL-2.0-or-later) |
| SteelSeries Apex 3 (base, no per-key) | keyboard | OpenRGB `Controllers/SteelSeriesController/SteelSeriesApex3Controller` (GPL-2.0-or-later) |

All four peripherals already have a working, open HID driver in the
user's local `OpenRGB` checkout — confirmed by reading the source, not
assumed. **No reverse engineering is needed for any of the four
peripheral protocols.** REA/reverse-engineering stays available for any
gap found during implementation (e.g. a firmware revision OpenRGB doesn't
cover), not as a planned up-front step.

Earlier conversation referred to the keyboard as "Razer Vulcan" — that was
wrong and is corrected here: Vulcan is a Roccat product line; Razer makes
no keyboard by that name. Only the Basilisk V3 is Razer.

## Licensing consequence (important, not optional)

OpenRGB is **GPL-2.0-or-later** (checked in the source headers directly,
not assumed — it is not MIT as initially misstated mid-conversation).
Porting/adapting its device-controller code into this app means the
combined binary must be GPL-2.0-or-later. This matches what the original
v1 design doc already anticipated ("Qt LGPL/GPL-2.0+"), so it's a
confirmation rather than a new constraint: once these controllers are
in, the whole app's license is GPL-2.0-or-later, and Qt continues to be
used under its LGPL dynamic-linking terms (no change needed there, since
GPL-2.0-or-later + LGPL dynamic linking is the normal, compatible
combination).

## Canvas architecture

### Spatial model (extends, doesn't replace, v1's effect model)

v1 already has almost the right shape for this:

- `IEffect::colorAt(x, y, elapsedMs)` (`src/effects/IEffect.h`) is already
  a pure function of local coordinates and time.
- `Component` (`src/device/Component.h`) already carries per-LED `[x,y]`
  coordinates.
- `Channel` (`src/device/Channel.h`) already resolves a global LED index
  to its component's `[x,y]` via `ledPosition()`.
- `Canvas` (`src/canvas/Canvas.h`) already ticks every channel once per
  frame and already distinguishes per-channel Native vs. Canvas mode.

What's missing is one level above today's per-channel-local coordinates:
a **canvas-global placement** for each channel/device (an offset + the
existing local coordinates), so that:

- A **global effect** can be assigned to the canvas as a whole and
  sampled in global coordinates — every region in "follow" mode reads
  its color from the device's placed position in that global effect.
- A region in "independent" mode keeps working exactly like v1 today:
  its own `IEffect` instance, sampled in its own local coordinates,
  ignoring the global effect entirely.

This is additive to the existing `ChannelAssignment`/`Canvas::tick()`
*model*. The current `Canvas` constructor takes a concrete
`MsiMotherboardDriver&`, though, not an interface — wiring in four more,
heterogeneous peripheral drivers does need an `IDevice`/`IDriver`-style
abstraction introduced underneath `Canvas` (confirmed via builder-check
against the real v1 code). That's a builder decision on the concrete
interface shape, not a new requirement — called out here so it isn't
mistaken for a simple field addition.

### Layout (confirmed via mockup, see "Decisions already made" below)

The canvas mirrors the user's actual physical layout: a tower outline
containing the motherboard zones, and a keyboard/mouse row below it —
not an abstract free grid. Placement is still drag-adjustable; the
case-shaped outline is a visual anchor, not a hard constraint.

### Effect reference points

An effect that has a spatially meaningful parameter (a radial gradient's
center, a comet's current head position, a wave's direction) exposes that
parameter as a draggable point directly in the canvas, confirmed via the
radial-gradient mockup. Effects without such a parameter (flat fill,
plain breathing) have nothing to drag.

## Effect library (staged)

Ported from OpenRGB's `OpenRGBEffectsPlugin/Effects` (GPL-2.0-or-later,
used as an implementation reference, not copied verbatim without
adaptation to this app's own `IEffect` interface and color model):

**v1 of this feature** — `GradientWave`/`RainbowWave` (already exists),
`RadialRainbow`, `Breathing`, `Comet`, `Spiral`, `SwirlCircles`,
`Fill`/`StaticColor` (already exists), `Marquee`.

**Explicitly deferred** (own subsystem each, own future design pass):
Audio-reactive effects (`AudioBubbles`, `AudioSync`, `AudioVisualizer`,
...), `GifPlayer`, and the `Layers`/`Stack`/`Mask` compositing effects.

## Application structure

- The v1 tray shell (`src/ui/TrayApp.*`) stays as the background
  process/autostart/quit surface — unchanged in role.
  A new menu entry opens the main window.
- New main window, "Lighting Studio": canvas + per-region effect/assignment
  panel. Canvas rendering uses Qt Quick/QML (not Widgets) for smooth
  per-frame animation and drag interaction; this is an implementation
  choice made here, not something the user was asked to pick.
- Visual/motion design for the new window follows the project's existing
  skill guidance (`emil-design-eng`, `apple-design`, the `animate` skills)
  at implementation time — e.g. spring-based drag release instead of
  linear snapping, respecting reduced-motion, restrained use of
  translucency for panels over the canvas. Exact values are an
  implementation detail, not specified here.
- Profiles: `Settings` gains a named list of full configurations instead
  of a single implicit one; the tray gets a quick-switch submenu listing
  them.

## Decisions already made (do not re-derive)

- Canvas layout: **physical-mirror layout**, confirmed via side-by-side
  mockup (chosen over an abstract free grid).
- Radial-gradient drag-the-center-point interaction: confirmed via mockup
  ("ja, genau so").
- Per-region mode: **both** global-follow and independent-effect must be
  selectable, simultaneously, per region — not an either/or app-wide
  switch.
- Device roster is fixed to the five rows in the table above; nothing
  else needs supporting in this pass.
- No RE of Razer Chroma/Synapse or SteelSeries GG/Prism — not needed,
  not installed for this purpose.
- JRGB1 stays Native-mode-only, as already established in v1 (it has no
  Gen1Adv area; `setChannelMode(..., Canvas)` already returns false for
  it). It never participates in "follow global effect" — this isn't a
  new restriction, it's the existing hardware model carried forward
  unchanged.
- **Peripherals are direct-mode only** (confirmed, post builder-check):
  Vulcan/Apex 3/Kone/Basilisk V3 show the app's chosen lighting only
  while this app is running. Closing the app (or booting before it
  starts) reverts them to their own onboard default — no flash-write
  protocol to any peripheral is implemented in this pass. This keeps the
  peripheral work to direct per-LED HID pushes only, matching how OpenRGB
  itself treats "Direct" mode for these same devices, and avoids a new,
  higher-risk category of persistent writes. (The existing motherboard
  Native mode is unaffected and unrelated: it already runs standalone on
  the board's own firmware without needing any flash write from this
  app — nothing changes there.)
- **Per-key/per-zone sampling in follow mode** (confirmed, post
  builder-check): when a peripheral region is set to "follow", each of
  its individually addressable LEDs/zones samples *its own* canvas
  position — not one averaged color for the whole device. Concretely,
  each peripheral is modeled the same way motherboard zones already are:
  as one or more `Component`s with per-LED `ledCoordinates`, placed into
  the canvas's global coordinate space. The Vulcan (per-key addressable)
  needs a real per-key coordinate map; the Apex 3/Kone/Basilisk (zoned,
  not per-key) need one per zone. This is the bigger of the two
  builder-check findings effort-wise and should be sized accordingly in
  the implementation plan.

## What's left to the builder

- Exact canvas-global placement representation (pixel offset? normalized
  0..1 coordinates? grid cells?) and where it's persisted in `Settings`.
- Exact QML component breakdown and file layout for the new window.
- Verifying each peripheral's real VID/PID/report IDs against the
  OpenRGB source *and* the actual connected device before any HID send
  (same safety-gate discipline as v1: exact bytes shown, plain-language
  confirmation, before anything reaches real hardware) — the table above
  names the source files, not verified byte values.
- Exact adaptation of each ported OpenRGB effect's math into this app's
  `IEffect::colorAt` shape.
- Profile data-model/UI details (naming, how many, tray submenu wording).
- Process: same subagent-driven TDD approach as v1 — this design doc
  feeds a `superpowers:writing-plans` task-plan next, executed in a fresh
  worktree, following the same per-task implement-then-review loop and
  the same real-hardware safety gate established in v1.
