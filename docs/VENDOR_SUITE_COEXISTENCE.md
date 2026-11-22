# Vendor suite coexistence (concept, not built)

Status: proposal from 2026-11-22, awaiting owner decision. Nothing here is implemented.
Termination behaviour in `src/startup/` is unchanged.

## Problem

The conflict dialog stops any process that sends lighting writes to one of our devices
(see `DeviceConflicts.h`). For peripheral suites (Razer Synapse, SteelSeries GG, Roccat
Swarm, Logitech G Hub, MSI Center, G-Menu) that process also owns key remapping, macros,
DPI stages and profile switching. Terminating it costs the user those functions until the
suite is restarted. Owner decision 2026-11-16 (CONTINUE.md): such suites must not be
stopped. The current dialog has no way to tell them apart.

Constraint: no executable name list as the trigger (owner decision 2026-11-16).

## What is lost on termination

| Function | Survives a killed suite? |
|---|---|
| Settings stored in device onboard memory (DPI, hardware macros, onboard profiles) | Yes, the device keeps them |
| Software macros, app-linked profile switching, key remaps done by the suite driver/filter | No |
| Suite-side features (battery, firmware update, sync) | No, until restart |

So the damage depends on whether the user relies on software-side features. The tool cannot know
that, so it must ask instead of defaulting to termination.

## Proposal

### 1. Classify writers by evidence already captured (no name list)

`captureUsbWrites` already records every HID SET_REPORT as `(pid, device, interface)` before it
is filtered to our endpoints. Derive per writer:

- **Lighting-only writer**: every write it sent during the capture targets an endpoint we drive.
  Typical for RGB-only tools (SignalRGB, LEDKeeper2). Keep today's behaviour: preselected, stopped
  by "Terminate all".
- **Multi-function writer**: it also wrote to a device or HID interface we do not drive
  (e.g. the macro/config interface of a keyboard, or a headset). Treat as a vendor suite.

Second, independent signal: the conflicting endpoint belongs to a peripheral (keyboard/mouse
profile category) rather than a lighting controller (motherboard, ARGB hub). A writer on a
peripheral endpoint is treated as a suite even without the multi-function evidence, because a
5 s capture can miss sporadic config writes.

Limits, stated up front: a suite idle during the capture looks lighting-only; interrupt-transfer
writes (Vulcan) are not captured at all (known gap in CONTINUE.md).

### 2. Change the dialog for suites, not the stop mechanics

For writers classified as suites:

- Not preselected; "Terminate all" skips them. Terminating stays possible through an explicit
  per-row choice with a warning ("macros and key remapping stop until you restart <program>").
- Offer, per conflicting device:
  - **Turn lighting off in <program>**: instructions plus a recheck button that reruns the
    capture. Proven path for MSI Center: Mystic Light off, LEDKeeper2 sent no writes in 90 s
    (2026-11-16).
  - **Leave this device to <program>**: Lighting Studio excludes that endpoint from output for
    this session (or persistently, per device). No process is touched; the suite keeps full
    control of that device, and our effects drive the rest.

The second option is the safe default because it needs no cooperation from the suite.

### 3. Phase 2 (research, unverified)

Several suites expose a lighting SDK that lets a third party draw while the suite keeps running
(Razer Chroma SDK, Corsair iCUE SDK, Logitech LED SDK, SteelSeries GameSense). Driving those
devices through the SDK instead of raw HID would remove the conflict entirely and keep macros.
Licensing, exclusivity behaviour and latency have not been checked; look up current docs before
committing to any of them.

## Open questions for the owner

1. Default for a suite conflict: "leave device to the suite" or "ask every time"?
2. Should "leave device to the suite" persist per device across sessions?
3. Is phase 2 (vendor SDK output) in scope at all?
