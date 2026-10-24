# Graph Report - msi-mystic-light  (2026-10-24)

## Corpus Check
- 112 files · ~117,294 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5 file(s) not represented in the graph (top: (none) 2, .cmake 2, .cmd 1)

## Summary
- 1801 nodes · 3693 edges · 101 communities (75 shown, 26 thin omitted)
- Extraction: 82% EXTRACTED · 18% INFERRED · 0% AMBIGUOUS · INFERRED: 661 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `83265806`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- OutputFrame
- MsiColor
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- NativeEffect
- HidApiDevice
- MsiDriver
- MsiGen1AdvArea
- Development Tools
- LightingWindow::LightingWindow
- MsiLightingMode
- stdexcept
- OutputSession
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
- ComponentCatalog.cpp
- main.cpp
- test_editor.cpp
- EffectLayer
- LightingWindow
- HidDescriptor
- .load
- Lighting editor
- OutputPanel
- LightingWindow.h
- ConflictProcesses.cpp
- authorizeHardwareOutput
- QPointF
- MsiZoneSettings
- MaskTool
- ConflictDialog
- ComponentEditor.cpp
- SignalForge editor reference
- Record
- test_output_ui.cpp
- ColorListEditor
- ConflictDialog.cpp
- Lighting Studio — Design Doc (v2)
- Record
- ComponentPlacement
- .latestDuringSlowWrite
- .connect
- HardwarePicker::HardwarePicker
- Transport
- ChannelEditor::ChannelEditor
- PaletteStrip
- findProfileCandidates
- DriverInstance
- OutputScheduler
- LightingState
- .start
- profile
- Status
- LayerBlend
- Hardware modules
- LightingDocument.h
- OutputProof
- LightingWindow.cpp
- test_hid_backend.cpp
- EffectKind
- OutputPanel.cpp
- BuildBasiliskColorsReport
- RecordingTransport
- PairedTransport
- OutputSession.cpp
- PeripheralDriver
- Tool
- Connection
- Output session backend
- ComponentEditor
- importLegacyHardware
- StudioWidgets.cpp
- QPainter
- HidRuntime
- .create
- ChannelAssignment
- Studio interaction and motion review

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 137 edges
2. `LightingCanvas` - 100 edges
3. `EffectLayer` - 52 edges
4. `LightingDocument` - 46 edges
5. `IHidDevice` - 38 edges
6. `OutputPanel` - 38 edges
7. `EditorTest` - 37 edges
8. `Implementation status` - 37 edges
9. `Record` - 34 edges
10. `LightingWindow::LightingWindow()` - 32 edges

## Surprising Connections (you probably didn't know these)
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Inventory and pinned revisions` --references--> `main()`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/main.cpp
- `Output scheduler` --references--> `OutputScheduler`  [INFERRED]
  docs/OUTPUT_SESSION.md → src/output/OutputScheduler.h
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp
- `Self-Review` --references--> `ChannelAssignment`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/editor/ChannelAssignment.h

## Import Cycles
- None detected.

## Communities (101 total, 26 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.10
Nodes (17): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+9 more)

### Community 1 - "OutputFrame"
Cohesion: 0.12
Nodes (13): OutputConnection, factory, initial, present, ChannelOutput, channelId, colors, native (+5 more)

### Community 2 - "MsiColor"
Cohesion: 0.11
Nodes (19): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness (+11 more)

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
Cohesion: 0.09
Nodes (20): Responsibilities, DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error (+12 more)

### Community 7 - "HidApiDevice"
Cohesion: 0.13
Nodes (5): HidApiDevice, handle_, initialized_, initializeHid(), HidBackendTest

### Community 8 - "MsiDriver"
Cohesion: 0.13
Nodes (5): MsiDriver, armed_, channels_, state_, MsiModule

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "LightingWindow::LightingWindow"
Cohesion: 0.19
Nodes (4): spin, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "stdexcept"
Cohesion: 0.09
Nodes (10): Task 10: Roccat Kone Aimo driver, Task 9: SteelSeries Apex 3 driver (T-Zone protocol), feature(), output(), BuildKoneColorsReport(), BuildKoneInitReport(), BuildVulcanColorPackets(), BuildVulcanEnableDirectReport() (+2 more)

### Community 14 - "OutputSession"
Cohesion: 0.11
Nodes (11): OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_, State (+3 more)

### Community 15 - "IDriverModule"
Cohesion: 0.08
Nodes (6): Adding support, IDriverModule, IncompatibleModule, MissingInterface, VirtualDriver, VirtualModule

### Community 16 - "HardwarePicker"
Cohesion: 0.11
Nodes (15): ChannelEditor, initial_, HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice (+7 more)

### Community 17 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "IHidDevice"
Cohesion: 0.09
Nodes (10): Completed: HID transport (2026-10-23), Task 18: Final wire-up and real-hardware verification, Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, Task 3: Canvas-global placement and "follow global effect", IHidDevice, FakeHidDevice, opened_, openSucceeds (+2 more)

### Community 21 - "Implementation status"
Cohesion: 0.06
Nodes (35): Accepted effect authoring follow-ups (2026-10-23), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23) (+27 more)

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
Cohesion: 0.33
Nodes (6): Contents, MSI Mystic Light - Lighting Studio, Per-device LED arrangement, Related repos, Research documents, Standalone app implementation

### Community 30 - "ComponentCatalog.cpp"
Cohesion: 0.07
Nodes (14): channelMapping(), ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), loadComponentDefinition(), requiredText(), DeviceProfile::fromJson() (+6 more)

### Community 31 - "main.cpp"
Cohesion: 0.10
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 32 - "test_editor.cpp"
Cohesion: 0.06
Nodes (26): channelAssignmentJson(), parseChannelAssignment(), validateChannelAssignments(), isParticle(), renderEffect(), validateEffect(), DocumentEdit, apply_ (+18 more)

### Community 33 - "EffectLayer"
Cohesion: 0.05
Nodes (40): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+32 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (50): LightingWindow, advanced_, allChannels_, animation_, animationControls_, applicationPages_, blend_, catalog_ (+42 more)

### Community 35 - "HidDescriptor"
Cohesion: 0.13
Nodes (10): hid_device_, HidDescriptor, interfaceNumber, manufacturer, path, product, productId, usage (+2 more)

### Community 36 - ".load"
Cohesion: 0.24
Nodes (4): main(), fail(), ModuleRegistry, entries_

### Community 37 - "Lighting editor"
Cohesion: 0.10
Nodes (21): Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Gradient travel and blend verification (2026-10-24), Images and crop, Layers and masks (+13 more)

### Community 38 - "OutputPanel"
Cohesion: 0.10
Nodes (13): OutputPanel, authorizing_, automatic_, configured_, connections_, endpoints_, message_, public (+5 more)

### Community 39 - "LightingWindow.h"
Cohesion: 0.11
Nodes (15): ColorEditor, ColorListEditor, QCheckBox, QComboBox, QDoubleSpinBox, QLabel, QLineEdit, QListWidget (+7 more)

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.05
Nodes (25): closeWindow(), ConflictProcess, executable, manualStop, path, pid, reason, started (+17 more)

### Community 42 - "authorizeHardwareOutput"
Cohesion: 0.18
Nodes (8): deviceIdentity(), readBoardIdentity(), authorizeHardwareOutput(), loadModules(), present(), sameEndpoint(), vulcanControlIdentity(), PeripheralTest

### Community 44 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 45 - "MaskTool"
Cohesion: 0.50
Nodes (3): MaskTool, Brush, Gradient

### Community 46 - "ConflictDialog"
Cohesion: 0.19
Nodes (10): ConflictDialog, busy_, conflicts_, ignore_, status_, table_, terminateAll_, QLabel (+2 more)

### Community 47 - "ComponentEditor.cpp"
Cohesion: 0.16
Nodes (3): GeometryView, commit, scale

### Community 48 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "test_output_ui.cpp"
Cohesion: 0.16
Nodes (4): connection(), identity(), OutputUiTest, state()

### Community 51 - "ColorListEditor"
Cohesion: 0.07
Nodes (23): Component catalog schema 1, ColorEditor, hex_, picker_, public, ColorListEditor, add_, gradient_ (+15 more)

### Community 53 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 54 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 55 - "ComponentPlacement"
Cohesion: 0.22
Nodes (5): ComponentPlacement, componentId, id, origin, rotation

### Community 56 - ".latestDuringSlowWrite"
Cohesion: 0.23
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.19
Nodes (6): validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 58 - "HardwarePicker::HardwarePicker"
Cohesion: 0.18
Nodes (5): generic(), HardwarePicker::HardwarePicker(), installedProducts(), isFan(), productIcon()

### Community 59 - "Transport"
Cohesion: 0.14
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "ChannelEditor::ChannelEditor"
Cohesion: 0.15
Nodes (8): ChannelEditor::ChannelEditor(), ComponentEditor::ComponentEditor(), applyStudioStyle(), disclosure(), installParameterTooltips(), sliderField(), studioToolIcon(), syncSliderFields()

### Community 63 - "PaletteStrip"
Cohesion: 0.13
Nodes (12): PaletteStrip, add, begin, colors, dragging, end, gradient, move (+4 more)

### Community 65 - "findProfileCandidates"
Cohesion: 0.14
Nodes (7): BoardIdentity, manufacturer, model, findProfileCandidates(), ProfileCandidate, device, profileId

### Community 66 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 67 - "OutputScheduler"
Cohesion: 0.14
Nodes (7): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 72 - "Status"
Cohesion: 0.33
Nodes (4): Status, error, generation, state

### Community 73 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

### Community 74 - "Hardware modules"
Cohesion: 0.20
Nodes (9): Build and inspect, Compatibility and trust, Hardware discovery, Hardware modules, LED discovery, Output-session ownership, Profile schema 1, Startup process conflicts (+1 more)

### Community 78 - "OutputProof"
Cohesion: 0.13
Nodes (8): OutputProof, controlIdentity, identity, initial, profile, reports, previewOutputReports(), prepareOutput()

### Community 79 - "LightingWindow.cpp"
Cohesion: 0.11
Nodes (3): colorIcon(), LayerTree, reordered

### Community 80 - "test_hid_backend.cpp"
Cohesion: 0.21
Nodes (6): HID_API_CALL hid_close(), HID_API_CALL hid_error(), HID_API_CALL hid_free_enumeration(), HID_API_CALL hid_send_feature_report(), HID_API_CALL hid_write(), hid_device_

### Community 81 - "EffectKind"
Cohesion: 0.06
Nodes (44): EffectDefinition, id, kind, label, EffectKind, Burst, Circle, ConicalGradient (+36 more)

### Community 83 - "OutputPanel.cpp"
Cohesion: 0.27
Nodes (3): authorize_, OutputPanel::OutputPanel(), stateName()

### Community 84 - "BuildBasiliskColorsReport"
Cohesion: 0.46
Nodes (3): BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport(), CalculateCrc()

### Community 86 - "PairedTransport"
Cohesion: 0.31
Nodes (3): PairedTransport, control, leds

### Community 88 - "PeripheralDriver"
Cohesion: 0.33
Nodes (3): PeripheralDriver, channel_, initialized_

### Community 89 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 90 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 91 - "Output session backend"
Cohesion: 0.50
Nodes (4): Automatic GUI output and presence, Output scheduler, Output session backend, Verification

### Community 98 - "ChannelAssignment"
Cohesion: 0.18
Nodes (10): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+2 more)

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.22
Nodes (8): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Gradient/output feedback correction (2026-10-24), Motion audit and implementation, Regression repair, Studio interaction and motion review

## Knowledge Gaps
- **574 isolated node(s):** `channels_`, `armed_`, `state_`, `channel_`, `initialized_` (+569 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 902 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **26 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `test_editor.cpp`, `findProfileCandidates`, `.load`, `OutputPanel`, `LightingWindow.h`, `LightingWindow::LightingWindow`, `LightingWindow.cpp`, `test_output_ui.cpp`, `OutputPanel.cpp`, `LightingCanvas`, `HardwarePicker::HardwarePicker`, `main.cpp`?**
  _High betweenness centrality (0.134) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `MsiColor`, `NativeEffect`, `HidApiDevice`, `MsiDriver`, `stdexcept`, `OutputSession`, `IDriverModule`, `BuildReport0x51`, `Recording`, `SignalRGB repository research`, `HidDescriptor`, `.load`, `Record`, `.connect`, `Transport`, `test_hid_backend.cpp`, `RecordingTransport`, `PairedTransport`, `PeripheralDriver`, `.create`?**
  _High betweenness centrality (0.116) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `Output session backend` to `test_editor.cpp`, `OutputFrame`, `OutputScheduler`, `.load`, `README.md`, `.connect`?**
  _High betweenness centrality (0.102) - this node is a cross-community bridge._
- **Are the 26 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 26 INFERRED edges - model-reasoned connections that need verification._
- **Are the 10 inferred relationships involving `IHidDevice` (e.g. with `Completed: HID transport (2026-10-23)` and `Adding support`) actually correct?**
  _`IHidDevice` has 10 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _574 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.1038961038961039 - nodes in this community are weakly interconnected._