# Lighting Studio UI redesign

Date: 2026-10-21. Platform: Windows, Qt 6 Widgets. Scope: desktop interface,
preview rendering and interactions; no new hardware driver or protocol changes.

## Design thesis

A lighting workspace should make the relationship between a physical zone,
its effect and its color immediately visible. The spatial preview is the
signature element; navigation and editing stay quiet and predictable.

## Audit and resulting changes

| Before | After | Reason |
| --- | --- | --- |
| Ten dense rows expose all channel controls at once | Device sidebar and one selected inspector | Progressive disclosure and clear ownership of an edit |
| 22 px color-only swatches | Large color buttons with readable hex values | Black is visible as an explicit choice; color is not the only information |
| Fixed-size canvas clips saved placements | Fitted scene with inverse pointer transform | Resizing changes presentation, not persisted coordinates or hardware sampling |
| Tiny scaled device labels | Screen-space labels and full names in the sidebar | Retain readable text in compact windows |
| Global effect behind white labels | Subdued effect preview with opaque label surfaces | Keep controls and content legible over arbitrary RGB output |
| Native zones have empty preview outlines | Representative color chips | Do not imply that a zone is unlit merely because it uses firmware effects |
| Continuous per-block painter calls at 60 Hz | OpenGL-composited filtered field at up to 60 Hz, suspended while hidden or paused | Lower preview overhead without changing the hardware rendering cadence |
| Profile actions mixed into the channel grid | Profile workspace above device navigation | Stable location for switching, adding, renaming and deleting profiles |

## References and interpretation

- [SignalRGB layouts](https://docs.signalrgb.com/guides/device-configuration/about-layouts):
  spatial relationships and selecting a device in its layout.
- [OpenRGB](https://openrgb.org/): lightweight local control and explicit device settings.
- [MSI Mystic Light](https://us.msi.com/support/technical_details/DT_SW_Mystic_Light):
  device-specific supported effects and profile workflows.
- Emil design engineering: immediate response, restrained transitions, deliberate
  spacing and interruption-safe feedback. Reviewed with the emil-design-eng skill.
- Apple design and HIG skills: grouping, hierarchy, native pointer/keyboard
  controls and progressive disclosure, translated to Windows rather than
  copying macOS window chrome.

HIG references read from the apple-hig-review skill: accessibility.md / Vision;
layout.md / Best practices and Visual hierarchy; typography.md; color.md;
designing-for-macos.md; sidebars.md; split-views.md; dark-mode.md; motion.md;
focus-and-selection.md / Best practices. These are design principles here,
not a claim of Apple-platform compliance.

## Motion audit, opportunities and implementation

The improve-animations and find-animation-opportunities passes were read-only;
implementation then used animate and was checked with review-animations.
Animation-vocabulary supplied terminology, not additional animation scope.

| Candidate | Frequency and purpose | Decision |
| --- | --- | --- |
| Pointer selection highlight | Frequent, direct editing | Immediate; unused fade state removed |
| Channel dragging | Continuous direct manipulation | Track pointer 1:1; never ease or spring the hardware placement |
| Keyboard device navigation | Frequent, functional selection | Immediate; no animation |
| Inspector content changes | Frequent, functional editing | Immediate; avoid moving controls under the pointer |
| Profile menus and color dialogs | Native focused interactions | Keep Qt controls and their focus behavior; no custom animated dropdown |
| Ambient panel shimmer and decorative entrance sequence | Repeated decoration without functional purpose | Rejected |

Windows SPI_GETCLIENTAREAANIMATION starts the
on-screen preview paused when client animations are disabled. A visible pause
control affects only preview repainting, never hardware output. A hidden
window does not keep its preview repaint timer running. This Qt painter
implementation makes no browser-compositor or GPU-only animation claim.

## Contracts preserved

- Existing profile names, channel identifiers, settings schema, effect IDs and
  change signals remain stable; selecting a device does not write settings.
- Mode choices still come from IDriver capabilities, including native-only
  IOShield and JRGB1. Irrelevant form rows are hidden.
- Custom saved sizes and positions remain unchanged; only exact legacy 70x40
  peripheral sizes are upgraded for readability. Newly created channel entries
  receive separated default positions instead of all overlapping at the origin.
- The canvas is a representative preview, not device discovery, exact firmware
  animation emulation or a statement that physical LEDs changed.
- GPU lighting remains unsupported and is labeled accordingly.

## Verification and limitations

Tests cover device-list/inspector selection, canvas selection synchronization,
scaled drag behavior through existing integration tests, preview pause without
settings changes, and all existing color/profile/capability interactions.
The optional LIGHTING_STUDIO_SCREENSHOT environment variable exports a
hardware-free demo at 1440x880 and 1080x700 logical pixels, using the real Qt
widgets and draw code. Demo screenshots never load or modify the user's profile.

Visual review checks spacing, labels, focus states, dropdown affordances and
both window sizes. Dark surfaces intentionally provide a neutral surround for
RGB inspection. A separate light/high-contrast theme, full screen-reader audit,
and real reduced-motion toggle testing remain follow-up work. Native firmware
animations are represented by their configured color, not simulated exactly.

Final validation: all 21 CTest targets passed in Debug on 2026-10-21,
including the Windows autostart test. Both exported window sizes were visually
inspected. UI screenshots are synthetic fixtures, not hardware verification.


## Component/region follow-up

The later [component and effect implementation](COMPONENTS_AND_EFFECTS.md)
supersedes the original single-color chips and decorative case/desk backdrop.
The desktop now shows registered region geometry; a separate detail tab shows
readable keys or regions. A bounded, OpenGL-filtered color field replaces the
earlier CPU-heavy preview. USB rendering has its own serialized thread.
Explicit fan/strip chains and custom recipes are configurable and persisted.
The visual editor replaces text-based palettes with draggable patterns/stops,
a live preview, and a shared color-wheel/RGB/hex picker. See the linked document
for exact rendering limits and migration behavior.

## Spatial layout and physical calibration follow-up

The fixed 70x40 footprint is now a legacy default, not the only sampling size.
Desktop manipulation includes resize/rotation handles and exact dimensions.
A transactional graphics editor adds movable component/LED geometry, software
LED grouping, four layout presets and per-component reset. Installed component
metadata and custom definitions populate the hardware list. Explicit empty chains
stay empty. All views and hardware share the geometry transform and wire mapping.
Vulcan wire calibration is based on physical single-address probes; reference
OpenRGB tables are a starting point whose correctness must still be tested.


## Workflow review: effects and physical chains

| Before | After | Why |
| --- | --- | --- |
| Hardware actions below the inspector fold | Dedicated Hardware chains tab and visible shortcut | Adding/removing physical products is a primary task. |
| One ambiguous new/edit button | Separate New canvas effect and Edit selected effect | Preserve the selected palette and avoid accidental default-name overwrite. |
| Temporal palette preview only | Explicit per-component, whole-chain and canvas paths | Preview the LED behavior the user intends to configure. |
| Generic LED slot cloud | Component cards, address intervals and spatial geometry | Make cable order and physical placement distinguishable. |
| Claimed autosave without failure feedback | Immediate recipe save with editor/footer feedback | Save must have an observable outcome. |

The native Qt interaction model remains. No decorative transitions were added;
direct manipulation and continuously rendered previews provide the feedback.


## Corrected layout intent

| Before | After | Why |
| --- | --- | --- |
| LED dots replace the spatial field for LED-path recipes | Continuous full-color field underneath neutral hardware silhouettes | Users place hardware by the visible flow of color. |
| GlobalFollow can ignore hardware position | Preview and shared output sample the same scene coordinates | Moving a device must change the colors it samples. |
| Catalog combo plus generic add/remove actions | Channel-scoped searchable product picker and per-component Remove | Keep the target channel visible throughout the action. |
| Effect selector lives only on another tab | Shared selector stays above the layout | Tune an effect while arranging hardware. |

Emil/Apple/animate remain the active design constraints: direct pointer tracking,
clear action targets, native focus and keyboard handling, no cosmetic movement
on repeated actions. The continuous effect serves as placement information;
existing pause/reduced-motion support remains independent of hardware timing.


## Screenshot-driven structure and artwork review

| Before | After | Why |
| --- | --- | --- |
| Hardware shortcut duplicates Hardware tab | Three primary sections: Layout / Effects / Hardware | One navigation target per task. |
| New effect and effect picker compete with placement | Effect creation/selection/editing only in Effects | Keep the placement task focused. |
| Placement inspector remains beside unrelated tasks | Inspector only in Layout; LED details in a device dialog | Show controls in context. |
| Fan outline, oval mouse, rectangular key grid | Original cached SVG bodies with descriptor-based keys/light markers | Make hardware types recognizable. |
| Tiny 70x40 keyboard/mouse footprints | Readable legacy-size migration; custom sizes retained | Preserve deliberate placements while fixing unreadable defaults. |
| Unconfigured connectors appear as fake layout devices | Configure them in Hardware before they appear | Do not imply a known physical product. |
| Effect preview reconstructs a similar pattern | Preview shares the selected effect instance | Prevent visible preview divergence. |

Apple HIG references applied as Windows design principles: layout.md / Best
practices and Visual hierarchy (group related controls, separate controls/content),
accessibility.md / Vision (readability), typography.md / Ensuring legibility,
color.md / Best practices, sidebars.md, toolbars.md, icons.md and split-views.md.
Emil review: direct response, fewer competing actions, one consistent illustration
system. Motion review removed unused selection-fade state; dragging and selection
remain immediate. The functional color field retains pause/Windows-motion support.
A complete screen-reader and alternative-theme audit remains outside this change.
