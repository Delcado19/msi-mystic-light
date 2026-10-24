# Graph Report - msi-mystic-light  (2026-10-24)

## Corpus Check
- 113 files · ~119,399 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5 file(s) not represented in the graph (top: (none) 2, .cmake 2, .cmd 1)

## Summary
- 1820 nodes · 3780 edges · 108 communities (80 shown, 28 thin omitted)
- Extraction: 82% EXTRACTED · 18% INFERRED · 0% AMBIGUOUS · INFERRED: 687 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `b7c42b73`
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
- FakeHidDevice
- Implementation status
- README.md
- SignalRGB repository research
- LightingCanvas
- Mystic Light — Overall Architecture
- .cancelGesture
- `Mystic Light Online Data.dat` — decryption and content
- Reference repository reuse review
- MSI Mystic Light - Lighting Studio
- ComponentCatalog.cpp
- main.cpp
- test_editor.cpp
- EffectLayer
- LightingWindow
- ConflictDialog.cpp
- .load
- Lighting editor
- OutputPanel
- RgbColor
- ConflictProcesses.cpp
- IHidDevice
- LightingCanvas.cpp
- MsiZoneSettings
- QWidget
- ConflictDialog
- ComponentEditor.cpp
- SignalForge editor reference
- Record
- test_output_ui.cpp
- ColorListEditor
- EffectRenderer.cpp
- Lighting Studio — Design Doc (v2)
- Record
- ComponentPlacement
- test_scheduler.cpp
- test_output.cpp
- HardwarePicker::HardwarePicker
- Transport
- MsiModule
- PaletteStrip
- ComponentEditor.h
- chooseEffect
- OutputScheduler
- LightingState
- .start
- importLegacyHardware
- Status
- LayerBlend
- Hardware modules
- LightingDocument.h
- OutputProof
- LightingWindow.cpp
- EffectKind
- OutputPanel.cpp
- BuildBasiliskColorsReport
- RecordingTransport
- ConflictProcess
- .connect
- .pushColors
- Tool
- Connection
- EffectParameter
- StartupTest
- StudioWidgets.cpp
- QPainter
- FakeProcesses
- .create
- ChannelAssignment
- DeviceProfile.cpp
- Studio interaction and motion review
- BuildApex3ColorsPacket
- VirtualModule
- ComponentsTest
- OutputConnection
- IConflictProcesses
- closeWindow

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 142 edges
2. `LightingCanvas` - 102 edges
3. `EffectLayer` - 52 edges
4. `LightingDocument` - 46 edges
5. `OutputPanel` - 45 edges
6. `Implementation status` - 39 edges
7. `IHidDevice` - 38 edges
8. `EditorTest` - 37 edges
9. `Record` - 34 edges
10. `LightingWindow::LightingWindow()` - 32 edges

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

## Communities (108 total, 28 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.10
Nodes (18): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+10 more)

### Community 1 - "OutputFrame"
Cohesion: 0.18
Nodes (10): previewOutputReports(), ChannelOutput, channelId, colors, native, software, OutputFrame, channels (+2 more)

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
Cohesion: 0.11
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 7 - "HidApiDevice"
Cohesion: 0.05
Nodes (23): hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer, path (+15 more)

### Community 8 - "MsiDriver"
Cohesion: 0.22
Nodes (4): MsiDriver, armed_, channels_, state_

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "LightingWindow::LightingWindow"
Cohesion: 0.16
Nodes (5): spin, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem, syncSliderFields()

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "stdexcept"
Cohesion: 0.22
Nodes (3): Task 10: Roccat Kone Aimo driver, BuildKoneColorsReport(), BuildKoneInitReport()

### Community 14 - "OutputSession"
Cohesion: 0.12
Nodes (11): OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_, State (+3 more)

### Community 15 - "IDriverModule"
Cohesion: 0.14
Nodes (4): Adding support, IDriverModule, IncompatibleModule, MissingInterface

### Community 16 - "HardwarePicker"
Cohesion: 0.12
Nodes (13): HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity, name (+5 more)

### Community 17 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "FakeHidDevice"
Cohesion: 0.17
Nodes (7): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, FakeHidDevice, opened_, openSucceeds, sendSucceeds, sentReports

### Community 21 - "Implementation status"
Cohesion: 0.05
Nodes (37): Accepted effect authoring follow-ups (2026-10-23), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23) (+29 more)

### Community 23 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 24 - "LightingCanvas"
Cohesion: 0.04
Nodes (41): LightingCanvas, autoFit_, cropTool_, dragging_, drawColor_, drawing_, drawingBase_, drawingTool_ (+33 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 26 - ".cancelGesture"
Cohesion: 0.11
Nodes (4): LightingCanvas::LightingCanvas(), MaskTool, Brush, Gradient

### Community 27 - "`Mystic Light Online Data.dat` — decryption and content"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "MSI Mystic Light - Lighting Studio"
Cohesion: 0.33
Nodes (6): Contents, MSI Mystic Light - Lighting Studio, Per-device LED arrangement, Related repos, Research documents, Standalone app implementation

### Community 30 - "ComponentCatalog.cpp"
Cohesion: 0.20
Nodes (8): channelMapping(), ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), loadComponentDefinition(), requiredText(), QJsonObject

### Community 31 - "main.cpp"
Cohesion: 0.09
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 32 - "test_editor.cpp"
Cohesion: 0.06
Nodes (22): channelAssignmentJson(), parseChannelAssignment(), validateChannelAssignments(), isParticle(), DocumentEdit, apply_, LightingDocument, initialCatalog_ (+14 more)

### Community 33 - "EffectLayer"
Cohesion: 0.05
Nodes (40): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+32 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (65): ColorEditor, ColorListEditor, LightingWindow, advanced_, allChannels_, animation_, animationControls_, applicationPages_ (+57 more)

### Community 36 - ".load"
Cohesion: 0.05
Nodes (24): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), readBoardIdentity(), main(), DriverInstance (+16 more)

### Community 37 - "Lighting editor"
Cohesion: 0.09
Nodes (22): Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Gradient travel and blend verification (2026-10-24), Images and crop, Layers and masks (+14 more)

### Community 38 - "OutputPanel"
Cohesion: 0.09
Nodes (15): OutputPanel, authorize_, authorizing_, automatic_, configured_, connections_, endpoints_, message_ (+7 more)

### Community 39 - "RgbColor"
Cohesion: 0.25
Nodes (5): Responsibilities, RgbColor, b, g, r

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.29
Nodes (7): conflictReason(), creationTime(), matchesProcessIdentity(), normalized(), processPath(), requiresManualStop(), winError()

### Community 42 - "IHidDevice"
Cohesion: 0.18
Nodes (3): Task 18: Final wire-up and real-hardware verification, IHidDevice, VirtualDriver

### Community 44 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 45 - "QWidget"
Cohesion: 0.20
Nodes (5): ComponentEditor::ComponentEditor(), applyStudioStyle(), disclosure(), installParameterTooltips(), studioToolIcon()

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
Cohesion: 0.28
Nodes (6): configure(), connection(), identity(), OutputUiTest, profile(), state()

### Community 51 - "ColorListEditor"
Cohesion: 0.07
Nodes (23): Component catalog schema 1, ColorEditor, hex_, picker_, public, ColorListEditor, add_, gradient_ (+15 more)

### Community 52 - "EffectRenderer.cpp"
Cohesion: 0.25
Nodes (13): effectTransform(), interpolate(), isGradient(), periodic(), polygon(), renderAtTime(), renderBaseEffect(), renderClippedBaseEffect() (+5 more)

### Community 53 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 54 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 55 - "ComponentPlacement"
Cohesion: 0.22
Nodes (5): ComponentPlacement, componentId, id, origin, rotation

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.26
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - "test_output.cpp"
Cohesion: 0.24
Nodes (5): frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 58 - "HardwarePicker::HardwarePicker"
Cohesion: 0.26
Nodes (5): generic(), HardwarePicker::HardwarePicker(), installedProducts(), isFan(), productIcon()

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 63 - "PaletteStrip"
Cohesion: 0.15
Nodes (10): PaletteStrip, add, colors, dragging, gradient, move, positions, remove (+2 more)

### Community 67 - "OutputScheduler"
Cohesion: 0.10
Nodes (12): Automatic GUI output and presence, Output scheduler, Output session backend, Transient layout sampling and per-channel pause, Verification, OutputScheduler, mutex_, pending_ (+4 more)

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

### Community 77 - "LightingDocument.h"
Cohesion: 0.14
Nodes (5): ProfileCandidate, device, profileId, LightingState, EffectLayer

### Community 78 - "OutputProof"
Cohesion: 0.20
Nodes (6): OutputProof, controlIdentity, identity, initial, profile, reports

### Community 79 - "LightingWindow.cpp"
Cohesion: 0.09
Nodes (3): colorIcon(), LayerTree, reordered

### Community 81 - "EffectKind"
Cohesion: 0.09
Nodes (22): EffectKind, Burst, Circle, ConicalGradient, Cross, Diamond, Fireflies, Heart (+14 more)

### Community 83 - "OutputPanel.cpp"
Cohesion: 0.22
Nodes (4): channelKey(), OutputPanel::OutputPanel(), stateName(), prepareOutput()

### Community 84 - "BuildBasiliskColorsReport"
Cohesion: 0.46
Nodes (3): BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport(), CalculateCrc()

### Community 85 - "RecordingTransport"
Cohesion: 0.14
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 86 - "ConflictProcess"
Cohesion: 0.15
Nodes (7): ConflictProcess, executable, manualStop, path, pid, reason, started

### Community 87 - ".connect"
Cohesion: 0.23
Nodes (3): nativeKey(), sameOutputScope(), validateFrame()

### Community 88 - ".pushColors"
Cohesion: 0.13
Nodes (7): feature(), output(), PeripheralDriver, channel_, initialized_, BuildVulcanColorPackets(), BuildVulcanEnableDirectReport()

### Community 89 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 90 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 92 - "EffectParameter"
Cohesion: 0.18
Nodes (10): EffectDefinition, id, kind, label, EffectParameter, EffectLayer, id, label (+2 more)

### Community 94 - "StudioWidgets.cpp"
Cohesion: 0.16
Nodes (4): ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), begin, sliderField()

### Community 96 - "FakeProcesses"
Cohesion: 0.20
Nodes (7): FakeProcesses, denied, failScan, forced, graceful, restart, running

### Community 98 - "ChannelAssignment"
Cohesion: 0.13
Nodes (13): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+5 more)

### Community 99 - "DeviceProfile.cpp"
Cohesion: 0.36
Nodes (5): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text()

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.20
Nodes (9): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Gradient/output feedback correction (2026-10-24), Layout feedback and hardware output controls (2026-10-24), Motion audit and implementation, Regression repair (+1 more)

### Community 101 - "BuildApex3ColorsPacket"
Cohesion: 0.36
Nodes (3): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), BuildApex3BrightnessPacket(), BuildApex3ColorsPacket()

### Community 107 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

## Knowledge Gaps
- **579 isolated node(s):** `channels_`, `armed_`, `state_`, `channel_`, `initialized_` (+574 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 908 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **28 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `test_editor.cpp`, `.load`, `OutputPanel`, `LightingWindow::LightingWindow`, `LightingDocument.h`, `LightingWindow.cpp`, `HardwarePicker`, `test_output_ui.cpp`, `OutputPanel.cpp`, `LightingCanvas`, `main.cpp`?**
  _High betweenness centrality (0.135) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `MsiColor`, `NativeEffect`, `HidApiDevice`, `MsiDriver`, `OutputSession`, `IDriverModule`, `BuildReport0x51`, `Recording`, `FakeHidDevice`, `SignalRGB repository research`, `.load`, `Record`, `Transport`, `MsiModule`, `qjsonarray`, `RecordingTransport`, `.connect`, `.pushColors`, `.create`, `BuildApex3ColorsPacket`, `VirtualModule`?**
  _High betweenness centrality (0.107) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `OutputScheduler` to `test_editor.cpp`, `OutputFrame`, `.load`, `README.md`, `.connect`, `test_output.cpp`?**
  _High betweenness centrality (0.088) - this node is a cross-community bridge._
- **Are the 29 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 29 INFERRED edges - model-reasoned connections that need verification._
- **Are the 7 inferred relationships involving `OutputPanel` (e.g. with `.applicationDevicePage()` and `.cancelAndChangedReview()`) actually correct?**
  _`OutputPanel` has 7 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _579 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.10276679841897234 - nodes in this community are weakly interconnected._