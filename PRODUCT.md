# Product

<!-- impeccable:product-schema 1 -->

## Platform

desktop

<!-- Native desktop app (C++ / Qt 6 Widgets), not web, iOS or Android; Impeccable's platform values do not cover it. Windows now, Linux is a later, not yet scoped target (owner, 2026-11-26). -->

## Users

The public: people who own RGB lighting hardware from several vendors (motherboard headers, fans, AIOs, LED strips, RAM, GPU, keyboards, mice) and want one tool to set it up and run it. That includes beginners without technical knowledge, who must get along without a manual. Today the maintainer is the main user and the only hardware tester (owner, 2026-11-26).

## Product Purpose

Lighting Studio configures and drives RGB lighting across motherboards and peripherals from one app. Users see which hardware is connected, place their components on a layout, design layered effects, and let a background service play them. Success means the whole setup lights up as designed, smoothly, from boot on, while the PC stays as unburdened as it would be without the tool.

## Positioning

Confirmed by the owner (2026-11-26):

- No third-party RGB software needed at runtime: Lighting Studio ships its own reverse-engineered drivers as plugins instead of wrapping a vendor SDK or SignalRGB/OpenRGB.
- Smoother effects: per-LED frames are streamed continuously, including on motherboard ARGB headers, without abrupt colour jumps.
- Lighting from system start: a Windows service plays the lighting from boot, with the editor closed.
- Comfortable assignment: adding hardware to RGB channels should be at least as comfortable as in SignalRGB (goal, not yet reached).

## Operating Context

- Windows PCs with mixed-vendor RGB hardware. Vendor suites (Razer Synapse, MSI Center, iCUE, Armoury Crate, NZXT CAM, L-Connect and others) are often installed because they also provide macros, key mapping, DPI or fan control.
- Installed mode: a LocalSystem service owns hardware output; the unelevated editor talks to it over an owner-authenticated named pipe. The editor has no tray icon and no logon autostart: closing it ends the editor, the service keeps playing. A portable mode drives hardware from the editor itself.
- Typical workflow: Devices (what hardware is connected, which components hang on which connector), Layout (where the components sit), Effects (what they show); the service then plays the result.

## Capabilities and Constraints

- Three work areas with one job each: Devices = which hardware, including the library of component types; Layout = where it sits; Effects = what it shows. Every function has exactly one entry point (owner, 2026-11-26).
- Low system load is a requirement: Lighting Studio is a lighting control, not a CPU/GPU stress test. No extra background checks or polling beyond what the output needs (owner, 2026-11-26).
- Coexistence with vendor software: conflicts are detected by behaviour on the device (how often another process sends lighting commands), never by a list of program names. Vendor software is not uninstalled or stopped by default; the preferred path is to switch its lighting off and keep it running for its other functions (confirmed with Razer Synapse 4: brightness OFF, 2026-11-26). Stopping a process happens only on request and only for the current session.
- Known gap: conflict detection sees only HID SET_REPORT control transfers; software writing through interrupt OUT, SMBus or its own USB protocol is not detected.
- Hardware support is per module with an explicit "hardware-confirmed" status (see README). Confirmed: MSI mainboard JARGB headers, MSI I/O shield, Gigabyte AERO OC GPU (NVAPI), ADATA XPG RAM (via the signed PawnIO driver); Razer Basilisk V3 output observed by the owner on 2026-11-26. Other peripherals and boards are implemented but not confirmed.
- License: original code GPL-2.0-or-later (README). Undecided: distribution beyond local ZIPs and GitHub releases; code signing (binaries are unsigned); Linux scope.

## Brand Commitments

- Name: Lighting Studio (the repository is still called msi-mystic-light).
- Existing assets in `assets/branding/` (logo/banner, header, background images, `lighting-studio.ico`).

## Evidence on Hand

- Hardware support table with confirmation status: `README.md`.
- Transparent hardware photos for component previews: `assets/hardware/photos/` (fans, AIOs, GPU, RAM, keyboard, mouse, LED strip, I/O shield).
- Screenshots of earlier builds: `docs/images/`.
- Protocol and vendor research: `docs/` (e.g. `VENDOR_SUITE_COEXISTENCE.md`, `HARDWARE_MODULES.md`).
- No user testimonials, download numbers, reviews or benchmarks exist; do not invent them.

## Product Principles

1. Own the hardware path, coexist with the rest: everything that touches our devices is ours, and vendor software keeps its non-lighting functions.
2. Smooth and light: output looks fluid, and the tool costs the PC as little as possible.
3. Lighting outlives the editor: the service is the source of truth for playback.
4. One place for each job: Devices, Layout and Effects stay separate, every function has one entry point, and a beginner understands it without a manual.
5. Honest status: hardware support, test state and limits are shown as they are, never overstated.
