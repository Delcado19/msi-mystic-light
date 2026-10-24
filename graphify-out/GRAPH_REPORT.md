# Graph Report - msi-mystic-light  (2026-10-24)

## Corpus Check
- 112 files · ~115,837 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5 file(s) not represented in the graph (top: (none) 2, .cmake 2, .cmd 1)

## Summary
- 1796 nodes · 3666 edges · 102 communities (74 shown, 28 thin omitted)
- Extraction: 82% EXTRACTED · 18% INFERRED · 0% AMBIGUOUS · INFERRED: 650 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `9194e2a6`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- test_editor.cpp
- MsiColor
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- IDriverModule.h
- HidApiDevice
- MsiDriver
- MsiGen1AdvArea
- Development Tools
- .edit
- MsiLightingMode
- .pushColors
- NativeEffect
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
- stdexcept
- main.cpp
- LightingDocument
- EffectLayer
- LightingWindow
- ConflictProcesses.cpp
- .load
- Lighting editor
- OutputPanel
- LightingWindow.h
- ConflictProcess
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
- QTimer
- Lighting Studio — Design Doc (v2)
- Record
- ComponentPlacement
- .latestDuringSlowWrite
- .connect
- HardwarePicker.cpp
- Transport
- ChannelEditor::ChannelEditor
- PaletteStrip
- StartupTest
- findProfileCandidates
- DriverInstance
- OutputScheduler
- LightingState
- .start
- ModulesTest
- Status
- LayerBlend
- Hardware modules
- IConflictProcesses
- OutputProof
- LightingWindow.cpp
- BuildKoneColorsReport
- EffectKind
- BuildApex3ColorsPacket
- BuildBasiliskColorsReport
- VirtualModule
- RecordingTransport
- renderEffect
- DocumentEdit
- Tool
- DeviceProfile.cpp
- .setStops
- channelMapping
- closeWindow
- StudioWidgets.cpp
- QPainter
- validateChannelAssignments
- .create
- ChannelAssignment
- .release
- Studio interaction and motion review

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 136 edges
2. `LightingCanvas` - 100 edges
3. `EffectLayer` - 52 edges
4. `LightingDocument` - 45 edges
5. `IHidDevice` - 38 edges
6. `OutputPanel` - 38 edges
7. `EditorTest` - 36 edges
8. `Implementation status` - 36 edges
9. `Record` - 34 edges
10. `LightingWindow::LightingWindow()` - 32 edges

## Surprising Connections (you probably didn't know these)
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Task 18: Final wire-up and real-hardware verification` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → src/hid/IHidDevice.h
- `Inventory and pinned revisions` --references--> `main()`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/main.cpp
- `Output scheduler` --references--> `OutputScheduler`  [INFERRED]
  docs/OUTPUT_SESSION.md → src/output/OutputScheduler.h
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp

## Import Cycles
- None detected.

## Communities (102 total, 28 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.10
Nodes (19): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+11 more)

### Community 1 - "test_editor.cpp"
Cohesion: 0.15
Nodes (7): document_, addEffect(), catalog(), channelProfile(), EditorTest, EditorTest::drawingAndAnimationRegressions(), private

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

### Community 6 - "IDriverModule.h"
Cohesion: 0.13
Nodes (9): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+1 more)

### Community 7 - "HidApiDevice"
Cohesion: 0.06
Nodes (23): hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer, path (+15 more)

### Community 8 - "MsiDriver"
Cohesion: 0.13
Nodes (5): MsiDriver, armed_, channels_, state_, MsiModule

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - ".edit"
Cohesion: 0.26
Nodes (3): channelAssignmentJson(), parseChannelAssignment(), state_

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - ".pushColors"
Cohesion: 0.13
Nodes (7): feature(), output(), PeripheralDriver, channel_, initialized_, BuildVulcanColorPackets(), BuildVulcanEnableDirectReport()

### Community 14 - "NativeEffect"
Cohesion: 0.12
Nodes (12): Responsibilities, NativeEffect, brightness, colors, id, inward, speed, RgbColor (+4 more)

### Community 15 - "IDriverModule"
Cohesion: 0.15
Nodes (4): Adding support, IDriverModule, IncompatibleModule, MissingInterface

### Community 16 - "HardwarePicker"
Cohesion: 0.13
Nodes (13): HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity, name (+5 more)

### Community 17 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "IHidDevice"
Cohesion: 0.10
Nodes (8): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, IHidDevice, FakeHidDevice, opened_, openSucceeds, sendSucceeds, sentReports

### Community 21 - "Implementation status"
Cohesion: 0.06
Nodes (34): Accepted effect authoring follow-ups (2026-10-23), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23) (+26 more)

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
Cohesion: 0.09
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 32 - "LightingDocument"
Cohesion: 0.09
Nodes (8): validateEffect(), LightingDocument, initialCatalog_, LightingDocument::LightingDocument(), profiles_, public, validate(), layer()

### Community 33 - "EffectLayer"
Cohesion: 0.05
Nodes (41): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+33 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (50): LightingWindow, advanced_, allChannels_, animation_, animationControls_, applicationPages_, blend_, catalog_ (+42 more)

### Community 35 - "ConflictProcesses.cpp"
Cohesion: 0.25
Nodes (8): conflictReason(), creationTime(), matchesProcessIdentity(), normalized(), processPath(), requiresManualStop(), SystemConflictProcesses, winError()

### Community 36 - ".load"
Cohesion: 0.21
Nodes (4): main(), fail(), ModuleRegistry, entries_

### Community 37 - "Lighting editor"
Cohesion: 0.10
Nodes (20): Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Gradient travel and blend verification (2026-10-24), Images and crop, Layers and masks (+12 more)

### Community 38 - "OutputPanel"
Cohesion: 0.06
Nodes (23): Connection, approved, generation, scheduler, OutputConnection, initial, present, OutputPanel (+15 more)

### Community 39 - "LightingWindow.h"
Cohesion: 0.11
Nodes (16): ColorEditor, ColorListEditor, QCheckBox, QComboBox, QDoubleSpinBox, QLabel, QLineEdit, QListWidget (+8 more)

### Community 41 - "ConflictProcess"
Cohesion: 0.10
Nodes (14): ConflictProcess, executable, manualStop, path, pid, reason, started, FakeProcesses (+6 more)

### Community 42 - "authorizeHardwareOutput"
Cohesion: 0.24
Nodes (7): readBoardIdentity(), authorizeHardwareOutput(), loadModules(), present(), previewOutputReports(), sameEndpoint(), vulcanControlIdentity()

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
Cohesion: 0.11
Nodes (4): ComponentEditor, GeometryView, commit, scale

### Community 48 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "test_output_ui.cpp"
Cohesion: 0.16
Nodes (7): prepareOutput(), configure(), connection(), identity(), OutputUiTest, profile(), state()

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
Cohesion: 0.06
Nodes (32): Automatic GUI output and presence, Output scheduler, Output session backend, Verification, ChannelOutput, channelId, colors, native (+24 more)

### Community 58 - "HardwarePicker.cpp"
Cohesion: 0.11
Nodes (5): generic(), HardwarePicker::HardwarePicker(), installedProducts(), isFan(), productIcon()

### Community 61 - "ChannelEditor::ChannelEditor"
Cohesion: 0.29
Nodes (6): ChannelEditor::ChannelEditor(), ComponentEditor::ComponentEditor(), applyStudioStyle(), installParameterTooltips(), sliderField(), syncSliderFields()

### Community 63 - "PaletteStrip"
Cohesion: 0.13
Nodes (11): PaletteStrip, add, colors, dragging, end, gradient, move, positions (+3 more)

### Community 65 - "findProfileCandidates"
Cohesion: 0.11
Nodes (10): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), ProfileCandidate, device, profileId (+2 more)

### Community 66 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 67 - "OutputScheduler"
Cohesion: 0.13
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
Cohesion: 0.20
Nodes (6): OutputProof, controlIdentity, identity, initial, profile, reports

### Community 79 - "LightingWindow.cpp"
Cohesion: 0.08
Nodes (6): button(), colorIcon(), LayerTree, reordered, LightingWindow::LightingWindow(), scan_

### Community 80 - "BuildKoneColorsReport"
Cohesion: 0.25
Nodes (3): Task 10: Roccat Kone Aimo driver, BuildKoneColorsReport(), BuildKoneInitReport()

### Community 81 - "EffectKind"
Cohesion: 0.06
Nodes (44): EffectDefinition, id, kind, label, EffectKind, Burst, Circle, ConicalGradient (+36 more)

### Community 83 - "BuildApex3ColorsPacket"
Cohesion: 0.36
Nodes (3): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), BuildApex3BrightnessPacket(), BuildApex3ColorsPacket()

### Community 84 - "BuildBasiliskColorsReport"
Cohesion: 0.46
Nodes (3): BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport(), CalculateCrc()

### Community 86 - "RecordingTransport"
Cohesion: 0.15
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 87 - "renderEffect"
Cohesion: 0.50
Nodes (3): isParticle(), renderEffect(), chooseEffect()

### Community 89 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 90 - "DeviceProfile.cpp"
Cohesion: 0.21
Nodes (9): ComponentDefinition::fromJson(), integer(), requiredText(), DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text() (+1 more)

### Community 92 - "channelMapping"
Cohesion: 0.18
Nodes (4): channelMapping(), loadComponentCatalog(), loadComponentDefinition(), ComponentsTest

### Community 94 - "StudioWidgets.cpp"
Cohesion: 0.16
Nodes (4): ColorListEditor::ColorListEditor(), disclosure(), begin, studioToolIcon()

### Community 98 - "ChannelAssignment"
Cohesion: 0.13
Nodes (13): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+5 more)

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.22
Nodes (8): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Gradient/output feedback correction (2026-10-24), Motion audit and implementation, Regression repair, Studio interaction and motion review

## Knowledge Gaps
- **573 isolated node(s):** `channels_`, `armed_`, `state_`, `channel_`, `initialized_` (+568 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 902 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **28 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `LightingDocument`, `findProfileCandidates`, `test_editor.cpp`, `.load`, `OutputPanel`, `LightingWindow.h`, `.edit`, `LightingWindow.cpp`, `test_output_ui.cpp`, `LightingCanvas`, `main.cpp`?**
  _High betweenness centrality (0.128) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `MsiColor`, `IDriverModule.h`, `HidApiDevice`, `MsiDriver`, `.pushColors`, `NativeEffect`, `IDriverModule`, `BuildReport0x51`, `Recording`, `SignalRGB repository research`, `.load`, `Record`, `.connect`, `Transport`, `vector`, `BuildApex3ColorsPacket`, `VirtualModule`, `RecordingTransport`, `.create`?**
  _High betweenness centrality (0.126) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `.connect` to `LightingDocument`, `OutputScheduler`, `.load`, `README.md`?**
  _High betweenness centrality (0.091) - this node is a cross-community bridge._
- **Are the 25 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 25 INFERRED edges - model-reasoned connections that need verification._
- **Are the 10 inferred relationships involving `IHidDevice` (e.g. with `Completed: HID transport (2026-10-23)` and `Adding support`) actually correct?**
  _`IHidDevice` has 10 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _573 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.09782608695652174 - nodes in this community are weakly interconnected._