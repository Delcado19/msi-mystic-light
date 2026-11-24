# Lighting Studio UX redesign - approved plan

Source: the plan approved with Codex on 2026-11-21 (German original:
`%USERPROFILE%\.codex\attachments\22874a83-f211-4610-a845-be943f53b452\approved-plan.md`).
This file is an English translation, copied into the repository on 2026-11-24 so the plan is no
longer stored only in a local Codex folder. It describes function and interaction. Visual design
follows `DESIGN.md` and `PRODUCT.md` on branch `docs/impeccable-product-design`, as the plan states.
It was implemented in `ae0a68d` (builds GS-06-R32 / R33). For the status, see the end of this file.

## Goal and order

Work happens on `feat/lighting-studio-ux-redesign` in three stages: first **Devices** and the
separate component library, then **Layout**, **Wiring** and the **Component Lab**, then the
**Effect Designer** and custom effects. The first start opens **Devices**. On later starts the
existing project and session recovery applies.

## Devices and Custom components

- **Devices** lists detected devices with their connector names, for example `JARGB1` and
  `JARGB2`. Each connector shows its configured components, its connection status and the used
  versus available LED addresses. An empty connector offers **Add component**. Without detected
  hardware, a supported controller can be chosen for offline planning.
- Components get visible **Edit**, **Identify** and **Add component** actions in context.
  **Duplicate** and **Remove** are in the component menu. Duplicate adds a separate instance on
  the same connector if its capacity allows it. Remove detaches the instance from connector and
  layout. Both can be undone. The generic PNGs from `feat/hardware-photos` serve as preview images.
  LED positions come from the component definition.
- **Identify** marks the component in the preview and, while hardware output runs, briefly flashes
  its addresses. Other addresses keep playing. If several components share the addresses, the UI
  marks all affected LEDs and explains why.
- **Custom components** stores definitions per user in the application data. Layout files contain
  stable references, instance data, placement and connector assignment. **Edit** changes the
  concrete instance. Changes to reusable LED geometry or internal wiring are saved as a new variant
  and can be applied to the selected instance. Definitions can be imported and exported separately.
- Opening an old layout moves its embedded custom definitions into the library. Identical
  definitions are reused. On ID conflicts every variant is kept and the old layout resolves to its
  own definition. The source file stays unchanged when opened. If a definition is missing when a new
  layout is opened, a dialog lists the missing components and offers to import them. The service
  handoff carries the required definitions separately in the internal playback bundle, so the
  Windows service can play after a restart regardless of the signed-in user.

## Layout, Wiring and Component Lab

- **Layout** shows the spatial arrangement. Components can be moved, rotated and snapped to the
  grid on the canvas. **Wiring** shows the configured data order per connector. Components can be
  moved between connectors, reordered within a chain and put onto the same addresses with
  **Share colors with…**. Capacity and shared addresses are visible before applying. Drag and drop
  has keyboard-reachable alternatives.
- The **Component Lab** opens from **Create component** in **Custom components**. Fan, ring and
  strip templates give an editable starting point from the LED count. In **Place LEDs** light
  points are set and moved on the canvas. In **Wire LEDs** points are connected in data order, or
  several points are grouped into one zone with a shared address. **Previous address** and
  **Next address** highlight all light points of one address for checking. Coordinate and wiring
  tables are dropped.
- The component definition distinguishes physical light points from logical addresses. On the
  Phanteks M25, twelve light points show six independently addressable zones. In the reference
  orientation the cable side at the hub is 6 o'clock. Zone 1 is at about 5 and 7 o'clock, zone 2 at
  4 and 8 o'clock, and zone 6 near 12 o'clock at about 1 and 11 o'clock. Both points of a zone show
  the same color. In the described setup the four fans and the first six strip LEDs share these six
  addresses. Strip LEDs from position 7 on stay independent of the fans. The view shows the observed
  address mapping; the electrical wiring stays open.

## Effect Designer

- **Effects** offers a searchable library with an animated preview on the user's own layout. While
  browsing existing effects, frequently needed controls such as color and speed come first.
  Source-specific parameters stay reachable at their source. Existing projects keep their effect
  behavior when opened.
- **Source**, **Motion** and **Mask** follow the selected layer. The mask has a thumbnail showing
  its edit state and the brush actions **Hide** and **Reveal**. By default the result is visible
  while painting. **Mask only**, **Invert** and **Follow effect movement** are available when needed.
- **Save as custom effect** stores a selected layer or group with sources, motion, masks, blend
  modes and required image data in the separate **Custom effects** library. Inserting creates an
  independently editable copy with new layer IDs. Device-specific target assignments are chosen in
  the target layout.
- **Chase** adds a running-light source along the logical data order of each selected connector.
  Color, speed, length and direction are directly adjustable. All light points on the same address
  show the same color. Preview, portable output and service use the same calculation. The first
  version allows custom effects through layers, masks and motion without programming; it stays
  within the confirmed scope **save layers + Chase**.
- **Tooltips** explain icon buttons, LED addresses versus light points, shared colors, the mask
  mode and the difference between preview and hardware output. Central actions have visible labels.
  Reasons for disabled actions are shown at the affected connector or control.

## Acceptance

- Qt tests cover adding, duplicating, removing and moving components, capacity limits, undo, and
  old and new project files. Migration, ID conflicts, missing definitions and a service restart
  with custom components are covered. New layout files are checked to contain only component
  references.
- Rendering and output tests cover the six Phanteks zones, the shared color of the four fans and
  strip LEDs 1-6, the independent strip LEDs from position 7, and Chase in both directions.
  Separate tests check that **Identify** keeps the rest of the playback running and that saved
  custom effects including masks are inserted editable again.
- Operation is checked in the running Windows build with mouse and keyboard, from first start
  through adding, arranging and wiring a component to masking and saving a custom effect. Physical
  flashing and the Phanteks mapping are listed as confirmed only after a test on the real setup.

## Implementation status (2026-11-24)

Method: code reading and grep on `feat/lighting-studio-ux-redesign` at `98c1982`. Nothing was run
for this review. "Present" means the control or code path exists; it does not mean the behavior
was verified in the running app or on hardware.

| Plan item | Status | Evidence / note |
| --- | --- | --- |
| Devices: connectors, components, offline planning, Add component | Present | `LightingWindow.cpp` (hardware page, "Add component", offline empty state); test `offlineDevicesWorkflow` |
| Duplicate / Remove with capacity limit and undo | Present | `LightingDocument::duplicateComponent` and `removeComponents` run through `edit()` (undoable, validated); capacity overflow throws, tested in `connectorEditsAndChase` |
| Identify keeps other addresses playing; shared addresses explained | Present, partly | Overlay after composition (`LightingDocument.cpp` ~661). Shared addresses are explained only as a 2 s status-bar message; whether all affected LEDs are marked was not checked. Physical flash not qualified |
| Per-user library, variants, import/export, migration, missing-definition dialog | Present | `registerComponent`, `applyComponentVariant`, "Import component JSON", "Export component definition", `MissingComponents` dialog; tests `componentLibraryMigration`, `customComponentsSurviveRestart` |
| Service bundle carries definitions | Present | `componentDefinitions` in `ServicePanel.cpp` / `ServiceRuntime.cpp` |
| First start opens Devices; later starts use recovery | Fixed 2026-11-24 | `LightingWindow.cpp:1732` still defaults to Devices; `AppSession` now saves the open workspace tab on close and restores it on later starts. Test `sessionRestoresWorkspaceTab` (offscreen); not checked in the installed app |
| Wiring: move between connectors, reorder, Share colors with…, keyboard alternatives | Present, partly | "Move to connector...", "Share colors with..." (`ChannelEditor.cpp`), up/down buttons in the chain. Whether every drag-and-drop action has a keyboard path was not checked |
| Component Lab: templates, Place LEDs, Wire LEDs, Previous/Next address, no tables | Present | `ComponentEditor.cpp`; the coordinate `QTableWidget` still exists as the internal model but is hidden (`table->hide()`) |
| Light points vs. addresses, M25 12 points / 6 zones | Present in code | Component schema 2; test `phanteksSharedAddresses`. Not qualified on the real setup |
| Effects library with preview, Source/Motion/Mask, Hide/Reveal, Mask only/Invert/Follow | Present | `LightingWindow.cpp`; test `effectLibraryCategorized`. The mask thumbnail was not checked |
| Save as custom effect, Custom effects library | Present | "Save as custom effect"; test `customEffectRoundTrip` |
| Chase | Present | `EffectKind::Chase`, composed over logical addresses in `LightingDocument`; tests `chaseInspector`, `connectorEditsAndChase` |
| Tooltips and disabled-action reasons | Not checked | No systematic review done |
| Acceptance walkthrough in the running build, mouse and keyboard | Open | Only native Qt tests on fixtures (`CONTINUE.md`, 2026-11-22) |
| Physical flash and Phanteks mapping on hardware | Open | Not tested on the real setup |
