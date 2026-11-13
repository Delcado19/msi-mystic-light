# Editor interaction audit and resolution - 2026-11-13

Status: implemented and validated locally. See HEADER_MASK_CLARITY.md for the
complete Release/native evidence and remaining hardware/installation limits.

| Reported problem | Resulting contract | Behavioral proof |
| --- | --- | --- |
| Mixed root/child creation | New layer creates paint; New group creates a source-free container. Selected groups receive children, selected children receive siblings, standalone sources create roots. Legacy parents preserve ownership. | explicitGroupCreationRules, objectLayersAndGestures, legacy-render round-trip |
| Unclear group benefit | Children compose first; parent mask, opacity, blend and channel routing apply to their combined result. No cross-layer parameter modulation or nested groups is claimed. | group-composition and routing regressions; LIGHTING_EDITOR.md |
| Duplicate gradient identity/deep menus | Drawing Gradient remains G; plus opens one searchable library with flat category headings. Down/Enter navigation and empty-query-result behavior are explicit. | compactInteractionContracts |
| Shifting options | Color/stroke/fill slots retain geometry; irrelevant fields disable. | compactInteractionContracts across tools |
| Inconsistent dropdowns/missing corners | One centered SVG chevron, transparent rounded drop-down subcontrol, common command sizes/separators. SVG image-format plugin ships with runtime. | actual decoding, native screenshots, package/installer checks |
| Different panel transparency | Both surfaces use alpha180; blank Navigator viewport paints through once. | matched-DPR pixel comparisons for Navigator and Inspector |
| Small/replaced logo | Original supplied turbine PNG is embedded unchanged, fitted 360x120 DIP at right; separate 4K photo covers window. | asset hashes, windowBackgroundResizes, native captures |
| Preview appears ineffective | Selected UUID reaches hardware preview; other placements dim using actual display color. All restores pixels; output/history unchanged. | channelScopesAndClocks |
| Nonstandard crop | Pending adjustable frame; Enter/double-click commits, Escape cancels; image/lock/mask context controls availability. | cropImageEditorContract, compactInteractionContracts |
| Invisible source selection | Extent feedback across tools; Move supplies geometry grips; group children retain their own extents. | native capture and selection regressions |
| No contextual actions | Canvas and Navigator menus reuse duplicate/delete, rename/visibility/order, lock/unlock, image crop and empty-space view/add actions. Overlap picking lives under Select object. | compactInteractionContracts, overlapAndSmartGuides, deleteSelections |
| Unclear pan/pipette | Hand moves the view; H/Space/middle mouse inputs and scope are stated. Color Picker samples the desktop with a pipette icon/cursor. | pan controls, real two-monitor RGB patch sampling |
| Window teardown abort | Cursor reset no longer reads document geometry; canvases hide before document destruction. | previously crashing scope case and active Move/Pan/Crop/fullscreen teardown |

At 1440 DIP the full command row remains visible; at 1060 DIP Panels remains
reachable and menu equivalents retain optional view commands. Filename is shown
in the window title; Fit is direct and Zoom groups its steps.

Canvas drawing retains the active group when an empty click clears object
selection. The effect library targets the explicitly selected group or selected
child's parent; select the destination in Navigator before adding a source.
The two workflows are documented rather than silently equating a paint source
with a group. Source image data and existing documents are preserved.

## Design and collaboration

UI/UX Pro Max was used during inspection and refinement, including stable layout,
consistent controls, clear disabled states and functional grouping. Its initial
toolbar searches did not yield applicable results; those proposals instead used
source evidence and drawing-app references. This is not a claim of a universal
accessibility audit or complete pixel parity with another application.

Claude reviewed bounded decisions: creation rules, pre-margin grip handling,
viewport-palette vs pixel evidence, member/child teardown lifetime, compact command
allocation, and final actual helper/destructor snippets. Final micro-review
`do-mwepaukl-b9g65j` found no blocking issue in its supplied snippets; it did not
inspect the whole repository. Codex verified claims against current files and
real tests. The type-dependent root suggestion was rejected; the legacy-parent
warning was retained and tested. Existing schemas were not changed.

References: [Krita group composition](https://docs.krita.org/en/reference_manual/layers_and_masks/group_layers.html),
[Krita layers](https://docs.krita.org/en/reference_manual/dockers/layers.html),
[GIMP tool terminology](https://docs.gimp.org/2.10/en/key-reference-tools.html).
