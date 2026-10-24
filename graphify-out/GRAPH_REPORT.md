# Graph Report - msi-mystic-light  (2026-10-24)

## Corpus Check
- 112 files · ~115,040 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5 file(s) not represented in the graph (top: (none) 2, .cmake 2, .cmd 1)

## Summary
- 1786 nodes · 3627 edges · 94 communities (73 shown, 21 thin omitted)
- Extraction: 82% EXTRACTED · 18% INFERRED · 0% AMBIGUOUS · INFERRED: 640 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `ebb5ca6a`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- EffectRenderer.cpp
- MsiZoneSettings
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- NativeEffect
- HidApiDevice
- MsiDriver
- MsiGen1AdvArea
- Development Tools
- EffectRenderer.h
- MsiLightingMode
- .pushColors
- HardwarePicker::HardwarePicker
- IDriverModule
- HardwarePicker
- BuildReport0x51
- Recording
- IHidDevice
- Implementation status
- README.md
- SignalRGB repository research
- LightingCanvas
- Mystic Light — Overall Architecture
- LightingCanvas.cpp
- ARCHITECTURE.md
- Reference repository reuse review
- MSI Mystic Light - Lighting Studio
- qjsonarray
- main.cpp
- test_editor.cpp
- EffectLayer
- LightingWindow
- HidDescriptor
- .load
- Lighting editor
- OutputPanel
- prepareOutput
- ConflictProcesses.cpp
- QPointF
- ComponentCatalog.cpp
- MaskTool
- ConflictDialog
- ComponentEditor.cpp
- SignalForge editor reference
- Record
- connection
- ColorListEditor
- ChannelAssignment
- Record
- ComponentPlacement
- test_scheduler.cpp
- .connect
- Transport
- ChannelEditor::ChannelEditor
- PaletteStrip
- OutputPanel.cpp
- findProfileCandidates
- DriverInstance
- OutputScheduler
- LightingState
- .start
- profile
- Status
- LayerBlend
- authorizeHardwareOutput
- LightingWindow.cpp
- EffectKind
- OutputSession
- OutputConnection
- RecordingTransport
- LightingWindow::LightingWindow
- Tool
- DeviceProfile.cpp
- ProfileCandidate
- channelMapping
- StudioWidgets.cpp
- QPainter
- .create
- QDialog
- Studio interaction and motion review
- Connection

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 135 edges
2. `LightingCanvas` - 98 edges
3. `EffectLayer` - 52 edges
4. `LightingDocument` - 45 edges
5. `IHidDevice` - 38 edges
6. `EditorTest` - 36 edges
7. `Implementation status` - 35 edges
8. `Record` - 34 edges
9. `ColorListEditor` - 32 edges
10. `OutputPanel` - 32 edges

## Surprising Connections (you probably didn't know these)
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Inventory and pinned revisions` --references--> `main()`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/main.cpp
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp
- `Self-Review` --references--> `ChannelAssignment`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/editor/ChannelAssignment.h
- `Task 8: Canvas render loop` --references--> `ChannelAssignment`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/editor/ChannelAssignment.h

## Import Cycles
- None detected.

## Communities (94 total, 21 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.07
Nodes (21): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+13 more)

### Community 1 - "EffectRenderer.cpp"
Cohesion: 0.25
Nodes (13): effectTransform(), interpolate(), isGradient(), periodic(), polygon(), renderAtTime(), renderBaseEffect(), renderClippedBaseEffect() (+5 more)

### Community 2 - "MsiZoneSettings"
Cohesion: 0.07
Nodes (32): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness (+24 more)

### Community 3 - "Mystic Light — local network interfaces"
Cohesion: 0.25
Nodes (8): 1. TCP 26820 — local HTTP "Web API" (`MLModule.dll:CWebAPI.cs`), 2. UDP 26822 — mobile app pairing (`CBaseServer.cs`/`CBaseFN.cs`), 3. UDP 26823 — "party sync" between multiple PCs (`CSyncServer.cs`/`CSyncFN.cs`), 4. UDP 61337 — SignalRGB bridge ("LEDKeeper 2" service), Live verification (2026-10-17): the whole MLModule network layer is dead code, Mystic Light — local network interfaces, Protocol (confirmed via code, extends the `Msi.js` reconstruction), Still open

### Community 4 - "Standalone RGB Lighting App — Design"
Cohesion: 0.15
Nodes (13): Accepted effect authoring scope (2026-10-23), Anti-goals, Constraints, Goals, Hardware module architecture (2026-10-23), How it gets made (decided), Intent and why, Non-goals (v1) (+5 more)

### Community 5 - "MSI Mainboard LED Protocol"
Cohesion: 0.22
Nodes (9): MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`), Record layout (16 bytes, per zone/"area"), Report `0x50` ("Gen1"), 290 bytes — status and default configuration, Report `0x51` ("Gen1_Adv"), 727 bytes — per-LED addressing of individual segments, Report `0x90`+port / `0xB0`+port ("Gen2"/"Gen2_Adv") — newer strip model, ⚠️ Safety-relevant finding: the `Store` flag is set on every write, Upstream OpenRGB bug found while cross-checking: `FeaturePacket_PerLED_761` is 727 bytes, not 761, Verification against the existing capture template (+1 more)

### Community 6 - "NativeEffect"
Cohesion: 0.06
Nodes (29): Build and inspect, Compatibility and trust, Hardware discovery, Hardware modules, LED discovery, Output-session ownership, Profile schema 1, Responsibilities (+21 more)

### Community 7 - "HidApiDevice"
Cohesion: 0.07
Nodes (14): hid_device_, HidApiDevice, handle_, initialized_, HidRuntime, ready, initializeHid(), HID_API_CALL hid_close() (+6 more)

### Community 8 - "MsiDriver"
Cohesion: 0.13
Nodes (5): MsiDriver, armed_, channels_, state_, MsiModule

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "EffectRenderer.h"
Cohesion: 0.15
Nodes (11): EffectDefinition, id, kind, label, EffectLayer, EffectParameter, EffectLayer, id (+3 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - ".pushColors"
Cohesion: 0.09
Nodes (13): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport() (+5 more)

### Community 14 - "HardwarePicker::HardwarePicker"
Cohesion: 0.26
Nodes (5): generic(), HardwarePicker::HardwarePicker(), installedProducts(), isFan(), productIcon()

### Community 15 - "IDriverModule"
Cohesion: 0.08
Nodes (6): Adding support, IDriverModule, IncompatibleModule, MissingInterface, VirtualDriver, VirtualModule

### Community 16 - "HardwarePicker"
Cohesion: 0.11
Nodes (13): HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity, name (+5 more)

### Community 17 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "IHidDevice"
Cohesion: 0.10
Nodes (9): Completed: HID transport (2026-10-23), Task 18: Final wire-up and real-hardware verification, Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, IHidDevice, FakeHidDevice, opened_, openSucceeds, sendSucceeds (+1 more)

### Community 21 - "Implementation status"
Cohesion: 0.06
Nodes (33): Accepted effect authoring follow-ups (2026-10-23), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23) (+25 more)

### Community 23 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 24 - "LightingCanvas"
Cohesion: 0.04
Nodes (41): LightingCanvas, autoFit_, cropTool_, dragging_, drawColor_, drawing_, drawingBase_, drawingTool_ (+33 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 27 - "ARCHITECTURE.md"
Cohesion: 0.28
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "MSI Mystic Light - Lighting Studio"
Cohesion: 0.40
Nodes (5): Contents, MSI Mystic Light - Lighting Studio, Related repos, Research documents, Standalone app implementation

### Community 31 - "main.cpp"
Cohesion: 0.10
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 32 - "test_editor.cpp"
Cohesion: 0.06
Nodes (21): channelAssignmentJson(), parseChannelAssignment(), validateChannelAssignments(), DocumentEdit, apply_, LightingDocument, initialCatalog_, LightingDocument::LightingDocument() (+13 more)

### Community 33 - "EffectLayer"
Cohesion: 0.05
Nodes (40): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+32 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (65): ColorEditor, ColorListEditor, LightingWindow, advanced_, allChannels_, animation_, animationControls_, applicationPages_ (+57 more)

### Community 35 - "HidDescriptor"
Cohesion: 0.18
Nodes (9): HidDescriptor, interfaceNumber, manufacturer, path, product, productId, usage, usagePage (+1 more)

### Community 36 - ".load"
Cohesion: 0.24
Nodes (4): main(), fail(), ModuleRegistry, entries_

### Community 37 - "Lighting editor"
Cohesion: 0.11
Nodes (19): Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Layers and masks, Layout (+11 more)

### Community 38 - "OutputPanel"
Cohesion: 0.12
Nodes (11): OutputPanel, authorizing_, connections_, endpoints_, message_, public, status_, stopping_ (+3 more)

### Community 39 - "prepareOutput"
Cohesion: 0.25
Nodes (5): GUI authorization and presence, Output session backend, Verification, prepareOutput(), sameOutputScope()

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.05
Nodes (25): closeWindow(), ConflictProcess, executable, manualStop, path, pid, reason, started (+17 more)

### Community 44 - "ComponentCatalog.cpp"
Cohesion: 0.33
Nodes (7): ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), loadComponentDefinition(), requiredText(), QJsonObject

### Community 45 - "MaskTool"
Cohesion: 0.50
Nodes (3): MaskTool, Brush, Gradient

### Community 46 - "ConflictDialog"
Cohesion: 0.13
Nodes (11): ConflictDialog, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_, table_, terminateAll_ (+3 more)

### Community 47 - "ComponentEditor.cpp"
Cohesion: 0.15
Nodes (3): GeometryView, commit, scale

### Community 48 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "connection"
Cohesion: 0.16
Nodes (4): connection(), identity(), OutputUiTest, state()

### Community 51 - "ColorListEditor"
Cohesion: 0.07
Nodes (23): Component catalog schema 1, ColorEditor, hex_, picker_, public, ColorListEditor, add_, gradient_ (+15 more)

### Community 53 - "ChannelAssignment"
Cohesion: 0.06
Nodes (26): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+18 more)

### Community 54 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 55 - "ComponentPlacement"
Cohesion: 0.22
Nodes (5): ComponentPlacement, componentId, id, origin, rotation

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.26
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.15
Nodes (7): nativeKey(), validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "ChannelEditor::ChannelEditor"
Cohesion: 0.17
Nodes (8): ChannelEditor::ChannelEditor(), ComponentEditor::ComponentEditor(), applyStudioStyle(), disclosure(), installParameterTooltips(), sliderField(), studioToolIcon(), syncSliderFields()

### Community 63 - "PaletteStrip"
Cohesion: 0.13
Nodes (11): PaletteStrip, add, colors, dragging, end, gradient, move, positions (+3 more)

### Community 64 - "OutputPanel.cpp"
Cohesion: 0.26
Nodes (3): authorize_, OutputPanel::OutputPanel(), stateName()

### Community 65 - "findProfileCandidates"
Cohesion: 0.15
Nodes (9): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), readBoardIdentity(), present(), sameEndpoint() (+1 more)

### Community 66 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 67 - "OutputScheduler"
Cohesion: 0.13
Nodes (8): Output scheduler, OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 72 - "Status"
Cohesion: 0.20
Nodes (8): Status, error, generation, state, State, Connected, Disconnected, Faulted

### Community 73 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

### Community 78 - "authorizeHardwareOutput"
Cohesion: 0.14
Nodes (10): authorizeHardwareOutput(), confirmOutputReports(), loadModules(), OutputProof, controlIdentity, identity, initial, profile (+2 more)

### Community 79 - "LightingWindow.cpp"
Cohesion: 0.09
Nodes (4): button(), colorIcon(), LayerTree, reordered

### Community 81 - "EffectKind"
Cohesion: 0.09
Nodes (22): EffectKind, Burst, Circle, ConicalGradient, Cross, Diamond, Fireflies, Heart (+14 more)

### Community 83 - "OutputSession"
Cohesion: 0.09
Nodes (16): ChannelOutput, channelId, colors, native, software, OutputFrame, channels, endpoint (+8 more)

### Community 85 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 86 - "RecordingTransport"
Cohesion: 0.15
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 87 - "LightingWindow::LightingWindow"
Cohesion: 0.15
Nodes (6): isParticle(), spin, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem, chooseEffect()

### Community 89 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 90 - "DeviceProfile.cpp"
Cohesion: 0.36
Nodes (5): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text()

### Community 91 - "ProfileCandidate"
Cohesion: 0.25
Nodes (4): ProfileCandidate, device, profileId, importLegacyHardware()

### Community 94 - "StudioWidgets.cpp"
Cohesion: 0.18
Nodes (3): ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), begin

### Community 98 - "QDialog"
Cohesion: 0.22
Nodes (3): ChannelEditor, initial_, ComponentEditor

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.29
Nodes (6): Data and transport boundaries, Design principles checked, Drawing and hardware regression pass (2026-10-24), Motion audit and implementation, Regression repair, Studio interaction and motion review

### Community 102 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

## Knowledge Gaps
- **566 isolated node(s):** `channels_`, `armed_`, `state_`, `channel_`, `initialized_` (+561 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 893 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **21 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `test_editor.cpp`, `OutputPanel.cpp`, `.load`, `OutputPanel`, `.confirmDiscard`, `LightingWindow.cpp`, `HardwarePicker`, `connection`, `LightingWindow::LightingWindow`, `LightingCanvas`, `ProfileCandidate`, `main.cpp`?**
  _High betweenness centrality (0.136) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `MsiZoneSettings`, `NativeEffect`, `HidApiDevice`, `MsiDriver`, `.pushColors`, `IDriverModule`, `BuildReport0x51`, `Recording`, `SignalRGB repository research`, `qjsonarray`, `.load`, `Record`, `.connect`, `Transport`, `vector`, `OutputSession`, `RecordingTransport`, `.create`?**
  _High betweenness centrality (0.121) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `prepareOutput` to `test_editor.cpp`, `OutputScheduler`, `.load`, `OutputSession`, `README.md`, `.connect`?**
  _High betweenness centrality (0.095) - this node is a cross-community bridge._
- **Are the 24 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 24 INFERRED edges - model-reasoned connections that need verification._
- **Are the 10 inferred relationships involving `IHidDevice` (e.g. with `Completed: HID transport (2026-10-23)` and `Adding support`) actually correct?**
  _`IHidDevice` has 10 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _566 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.07459677419354839 - nodes in this community are weakly interconnected._