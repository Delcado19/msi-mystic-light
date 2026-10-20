# Standalone RGB Lighting App — Design

## Intent and why

MSI Mystic Light's effect selection is too limited, and the user doesn't
want to depend on MSI Center (unreliable driver updates, bare-minimum
functionality) to light their own hardware. SignalRGB already does this
well — full device detection, a rich effect library, good UX — but the
features the user actually wants (richer lighting control, later:
temperature/fan monitoring tied to lighting) sit behind SignalRGB's paid
Pro tier. The goal is a free, standalone app that reaches at least
SignalRGB/Mystic-Light-level lighting quality, built on protocol work
already reverse-engineered this session, with an architecture capable of
growing to more devices and manufacturers later without a rewrite.

This is v1 of a multi-phase plan. Monitoring/fan control is explicitly
deferred to a later phase and is a non-goal here.

## Goals

- A standalone Windows app (Linux-portable by construction, not required
  to run on Linux in v1) that drives the user's MSI MPG B850 Edge TI WiFi
  motherboard (VID:PID `0DB0:0076`, chip-ID family shared by ~135 MSI
  boards) over raw USB HID, with no MSI Center, LEDKeeper2, or any other
  vendor service running.
- Smooth, full custom per-LED effects (software-rendered, not limited to
  Mystic Light's ~11 built-in firmware modes), with transition quality at
  least matching Mystic Light's native animations and SignalRGB's canvas
  effects — a real engineering target (steady frame-rate render loop, not
  a given), not an automatic hardware benefit.
- **MSI's native firmware modes (`0x50`: Wave, Breathing, ColorRing,
  Meteor, GodLike, ...) stay available as a first-class option alongside
  custom canvas effects** — per channel/zone, the user picks either "run
  a canvas effect" (per-LED, `0x51`) or "run a native device mode"
  (device-side animation, `0x50`). Both matter for v1; neither is a
  fallback for the other. This deliberately means the device driver layer
  *does* need to know about native modes, not only raw colors — see the
  Canvas section below for how that's scoped without breaking the
  effect/device separation for canvas-driven channels.
- **Multiple effects can run concurrently, independently per channel/zone**
  (e.g. a wave on the fan headers, a native Breathing mode on the IO
  Shield, a static color on the EZ Connector, all at once) — not one
  global effect for the whole canvas.
- **Lifecycle**: the app runs as a tray application with OS-level
  autostart; on launch (including after a reboot/login) it restores the
  last-applied lighting configuration automatically. No Windows service
  and no requirement to light anything before Windows login — the gap
  between power-on and the user logging in is out of scope.
- An architecture that can grow to the user's other already-owned
  peripherals (Roccat Vulcan 100 keyboard, Roccat Kone AIMO mouse, Razer
  Basilisk V3 mouse) and, beyond that, to other manufacturers/devices in
  general, following the device-abstraction model below from day one —
  without those devices needing to work in v1.
- Cross-platform-capable technology choices (C++, Qt, hidapi) so a future
  Linux port is a packaging/driver-detail exercise, not a rewrite.
- **A later public release (e.g. GitHub) is a real possibility, not just
  a private tool** — licensing and provenance of any reference material
  used during development must stay clean from day one (see Constraints).

## Non-goals (v1)

- Temperature/fan monitoring or fan curve control. (Known concrete lead
  for later: report `0xF0` observed as periodic sensor telemetry on this
  board, per `OpenRGBResearch/msi-b850/PROTOCOL_NOTES.md`; also consider
  LibreHardwareMonitorLib as a more portable alternative to decoding it.)
- Macros, game integrations, audio visualizers, or any other
  SignalRGB-Pro-style non-lighting feature. (Audio-reactive lighting may
  well be the first non-lighting-only feature in a later phase, but it's
  out of scope for this document.)
- Actually implementing Roccat/Razer device plugins. The architecture must
  not preclude them; writing them is not part of v1.
- A Linux build. The stack must not block one later.

## Anti-goals

- Depending on MSI Center, LEDKeeper2, OpenRGB, or SignalRGB being
  installed or running at runtime. Reference material from all of the
  above may inform the implementation; none may be a runtime dependency.
- Reimplementing device support that's a solved problem elsewhere (e.g.
  Razer/Roccat, already well-supported in OpenRGB's open-source code) when
  it isn't needed for v1 — not an excuse to skip the extensible
  architecture, but a reason not to pre-build drivers nobody's using yet.
- Sending untested or unconfirmed byte sequences to the real board. Every
  write path must be checked against already-confirmed protocol facts
  (`docs/MB_800_HID_PROTOCOL.md`) before it ever reaches real hardware;
  anything not yet confirmed needs the explicit, plain-language
  confirm-before-send step already established this session.

## Constraints

- Keep all documentation, code, comments, and commit messages in English
  regardless of chat language (standing rule, see
  `../../.claude-memory` equivalent — `docs_always_english` memory).
- No AI co-authorship credit in commits.
- Don't depend on decompiled or decrypted proprietary vendor material
  (MSI's own code, SignalRGB's JS plugins) at runtime — they're reference
  material used during development only, same posture already used for
  the OpenRGB contributions earlier this session.
- **Public release is a real possibility, so license cleanliness starts
  now, not retroactively.** Qt must be used under its LGPL terms (dynamic
  linking, no static-link-triggered commercial obligation). Anything
  structurally adapted from OpenRGB's own code (GPL-2.0-or-later) — as
  opposed to protocol *facts* learned by reading it, which aren't
  copyrightable — means this project's own license needs to be
  compatible (GPL-2.0-or-later is the simplest safe default, matching
  OpenRGB itself). SignalRGB's JS plugins are proprietary and closed
  source; nothing from them is ever copied as code, only observed
  protocol/architecture facts, exactly as already practiced this
  session.

## How it gets made (decided)

**Language/platform**: C++, chosen explicitly over C#/.NET and Python for
Linux-portability without a GUI-framework rewrite. Qt for UI and 2D
canvas rendering (QPainter/QML), `hidapi` for USB HID I/O — the same pairing
OpenRGB and SignalRGB themselves both use, independently proven on this
exact class of hardware on both Windows and Linux.

**Device abstraction model**, adapted from SignalRGB's architecture
(confirmed by reading its installed, signed plugins for this exact board
family, a keyboard, and the public developer docs — see
`docs/MB_800_HID_PROTOCOL.md` and this session's research; not copied
code, only the architecture and observed protocol facts):

- **Fixed devices** (keyboard, mouse, GPU — one specific permanent
  physical LED layout): the device driver declares LED names and 2D
  positions directly, and samples its own canvas region once per render
  frame at those coordinates. No further indirection.
- **Modular devices** (motherboard ARGB/EZ headers, fan hubs — the
  physical product attached can vary): a **Component** is pure data (LED
  count, 2D positions, names, one physical product) in a human-editable
  format (JSON or similar); a **Channel** is the controller's output
  chain that one or more Components get assigned to (by the user, in a
  picker UI); the device driver only ever writes to a named Channel and
  never needs to know which Components are attached to it.
- **Canvas, revised for concurrent per-channel effects and native modes**:
  not one single global canvas. Each channel/zone has its own assignment,
  independently, to one of:
  - a **canvas effect** — its own 2D render surface that an effect draws
    to; the device driver samples color at its declared LED/Component
    coordinates every render frame, same separation as before (effect
    layer never knows about devices).
  - a **native device mode** — the device driver sends the board's own
    `0x50` zone settings (`LightingMode=Wave/Breathing/...`, speed,
    colors) once on selection/change, with no per-frame canvas sampling
    for that channel at all.
  This means the device driver for a modular device *does* need to know
  "is this channel in native-mode or canvas-mode" and branch accordingly
  — an accepted, deliberate exception to "the device layer never knows
  about effects," scoped to exactly the native-mode case. Canvas-mode
  channels keep the full separation. A steady, correctly-paced render
  loop for canvas-mode channels is real engineering, not a given — that
  remains the core of the "smooth custom effects" goal.
- New devices should be addable later as new Components (pure data) or
  new fixed-device drivers, without changing the Canvas/render-loop core.

**MSI motherboard protocol** (modular device, this board's HID driver):
fully specified in `docs/MB_800_HID_PROTOCOL.md` — report `0x50` (18×16-byte
zone records, selects the firmware's native animation modes; **confirmed
working live against real hardware on 2026-10-20**) and report `0x51`
(727-byte `Gen1_Adv`, true per-LED color data — also **confirmed working
live on 2026-10-20**, on two independent zones, `JARGB1`/`JARGB2`). The
"confirmed working" claim for `0x51` was originally based only on reading
SignalRGB's plugin source, never an observed effect; the live test found
the missing precondition: the firmware only honors `0x51` per-LED data for
a zone whose `0x50` record has `LightingMode=Advanced` (`9`), not `Off`
(the default) or `Steady` (tried first, insufficient) — see
`docs/MB_800_HID_PROTOCOL.md`'s `0x51` section for the full finding. The
app's motherboard driver now sends one `0x50` arming report to set a
zone's mode to `Advanced` whenever that zone enters Canvas mode, before
any `0x51` push. Gen2
(`0x90+`/`0xB0+`) is confirmed unused for this board's VID (`0x0DB0`) from
three independent sources and is not part of v1. Exact per-zone LED counts
for the user's board variant (`MPG B850 EDGE TI WIFI`, MS-7E62) are known:
3 ARGB headers (≤120 LEDs each), 1 EZ Connector (≤120 LEDs), IO Shield (11
LEDs), JRGB1 (1 non-addressable LED).

## Settled during review (builder-check pass)

A fresh-subagent review of the first draft surfaced five open questions;
the four that genuinely changed the plan were brought back to the user
directly and are now folded into Goals/Constraints/Canvas above
(lifecycle, native-modes-vs-canvas scope, concurrent per-channel effects,
public-release licensing posture). One lower-priority item from that
review is intentionally deferred rather than settled: **what's physically
connected to JARGB1-3 and the EZ Connector right now** (real product and
LED count) isn't needed to design the architecture — the builder can ship
a generic/placeholder Component for these and adjust once real hardware
is being exercised. Ask the user for their actual setup when building the
MSI Component definitions, not before.

## What's left to the builder

- Exact Qt widget/QML layout for the UI, the layout/arrangement editor,
  and how the Canvas is actually painted (QPainter onto a QImage vs. a
  QML Canvas item) — any reasonable choice that meets the render-loop
  timing goal is fine.
- Exact Component file format (JSON is the obvious choice, mirroring
  SignalRGB's field set since those are descriptive facts, not
  copyrighted code, but the precise schema is an implementation detail).
- Exact plugin/driver interface in C++ — static linked classes for v1 is
  reasonable; a dynamically loaded plugin system (matching the
  "extensible by design" goal) can come later without this document
  dictating when.
- Project layout and build system (CMake is the obvious default, matching
  OpenRGB's own convention, but not mandated here).
- Whether effects are hardcoded C++ classes for v1 or get a minimal
  scripting/config layer — SignalRGB's JS-webpage approach is inspiration,
  not a requirement; v1 can ship with a handful of native C++ effects.
- The exact render-loop pacing mechanism (fixed-rate timer, vsync-driven,
  etc.) as long as it demonstrably produces smooth transitions comparable
  to Mystic Light's native animations — this is a concrete testable bar,
  not a specific implementation technique.
- Any unit/integration test approach for the HID write path; must at
  minimum keep the existing safety rule (explicit human confirmation
  before any new/unconfirmed byte sequence reaches real hardware) intact
  and enforced somewhere in the dev workflow, not just as a verbal
  reminder.
- Tray-app framework details (Qt's own system-tray APIs are the obvious
  choice, already cross-platform) and exactly where/how "last-applied
  configuration" is persisted between runs (a local settings file is the
  obvious choice; format is an implementation detail).
- The real physical Components attached to JARGB1-3/EZ Connector today
  (see above) — ask the user when building the MSI device definitions.
