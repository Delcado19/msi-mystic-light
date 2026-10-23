# Graph Report - msi-mystic-light  (2026-10-23)

## Corpus Check
- 89 files · ~103,673 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5 file(s) not represented in the graph (top: (none) 2, .cmake 2, .cmd 1)

## Summary
- 1659 nodes · 3297 edges · 108 communities (79 shown, 29 thin omitted)
- Extraction: 83% EXTRACTED · 17% INFERRED · 0% AMBIGUOUS · INFERRED: 563 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `cf6dba97`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- ChannelAssignment
- File Structure
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- IDriverModule.h
- test_hid_backend.cpp
- OutputSession
- MsiGen1AdvArea
- Development Tools
- MsiZoneSettings
- MsiLightingMode
- StudioWidgets.h
- MsiDriver
- IDriverModule
- HardwarePicker
- BuildReport0x51
- MaskTool
- Recording
- IHidDevice
- Implementation status
- README.md
- SignalRGB repository research
- LightingCanvas
- Mystic Light — Overall Architecture
- .cancelGesture
- `Mystic Light Online Data.dat` — decryption and content
- Reference repository reuse review
- MSI Mystic Light - Lighting Studio
- stdexcept
- AppSession
- LightingDocument
- EffectLayer
- LightingWindow
- LightingWindow::LightingWindow
- ModuleRegistry
- Lighting editor
- OutputPanel
- LightingWindow.h
- ConflictProcesses.cpp
- EffectRenderer.cpp
- QJsonObject
- NativeEffect
- ConflictDialog
- GeometryView
- SignalForge editor reference
- Record
- test_output_ui.cpp
- ColorListEditor
- Hardware modules
- Lighting Studio — Design Doc (v2)
- Record
- ComponentPlacement
- .latestDuringSlowWrite
- .load
- Transport
- StudioWidgets.cpp
- PaletteStrip
- test_editor.cpp
- BoardIdentity
- DriverInstance
- OutputScheduler
- LightingState
- .start
- ModulesTest
- Status
- LayerBlend
- OutputFrame
- authorizeHardwareOutput
- .edit
- RecordingTransport
- EffectKind
- LightingDocument.h
- EffectRenderer.h
- OutputConnection
- ConflictProcess
- LightingWindow.cpp
- HidDescriptor
- Tool
- .connect
- StartupTest
- FakeProcesses
- .rebuild
- closeWindow
- IConflictProcesses
- MsiModule
- QDialog
- Studio interaction and motion review
- DocumentEdit
- Connection
- Self-Review
- validateChannelAssignments
- LightingCanvas.h
- chooseEffect

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 129 edges
2. `LightingCanvas` - 87 edges
3. `EffectLayer` - 49 edges
4. `LightingDocument` - 44 edges
5. `Record` - 34 edges
6. `IHidDevice` - 33 edges
7. `EditorTest` - 33 edges
8. `Implementation status` - 33 edges
9. `ColorListEditor` - 32 edges
10. `OutputPanel` - 31 edges

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

## Communities (108 total, 29 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.10
Nodes (20): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+12 more)

### Community 1 - "ChannelAssignment"
Cohesion: 0.18
Nodes (10): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+2 more)

### Community 2 - "File Structure"
Cohesion: 0.14
Nodes (15): File Structure, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness, Task 3: HID transport interface and hidapi backend, Task 4: Component data model, Task 5: Channel model, Task 7: Effect interface and built-in effects (+7 more)

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

### Community 7 - "test_hid_backend.cpp"
Cohesion: 0.07
Nodes (13): hid_device_, HidApiDevice, handle_, initialized_, HidRuntime, ready, initializeHid(), HID_API_CALL hid_close() (+5 more)

### Community 8 - "OutputSession"
Cohesion: 0.12
Nodes (11): OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_, State (+3 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "StudioWidgets.h"
Cohesion: 0.15
Nodes (10): ColorEditor, hex_, picker_, public, QDoubleSpinBox, QLabel, QLineEdit, QPushButton (+2 more)

### Community 14 - "MsiDriver"
Cohesion: 0.24
Nodes (4): MsiDriver, armed_, channels_, state_

### Community 15 - "IDriverModule"
Cohesion: 0.10
Nodes (5): Adding support, IDriverModule, IncompatibleModule, MissingInterface, VirtualModule

### Community 16 - "HardwarePicker"
Cohesion: 0.09
Nodes (18): generic(), HardwarePicker, HardwarePicker::HardwarePicker(), headers_, leds_, products_, productsList_, HeaderChoice (+10 more)

### Community 17 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 18 - "MaskTool"
Cohesion: 0.50
Nodes (3): MaskTool, Brush, Gradient

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "IHidDevice"
Cohesion: 0.11
Nodes (9): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, Task 9: SteelSeries Apex 3 driver (T-Zone protocol), IHidDevice, FakeHidDevice, opened_, openSucceeds, sendSucceeds (+1 more)

### Community 21 - "Implementation status"
Cohesion: 0.06
Nodes (31): Accepted effect authoring follow-ups (2026-10-23), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23) (+23 more)

### Community 23 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 24 - "LightingCanvas"
Cohesion: 0.04
Nodes (34): LightingCanvas, autoFit_, cropTool_, dragging_, drawColor_, drawing_, drawingBase_, drawingTool_ (+26 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 27 - "`Mystic Light Online Data.dat` — decryption and content"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "MSI Mystic Light - Lighting Studio"
Cohesion: 0.40
Nodes (5): Contents, MSI Mystic Light - Lighting Studio, Related repos, Research documents, Standalone app implementation

### Community 31 - "AppSession"
Cohesion: 0.18
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 32 - "LightingDocument"
Cohesion: 0.10
Nodes (5): LightingDocument, initialCatalog_, LightingDocument::LightingDocument(), profiles_, public

### Community 33 - "EffectLayer"
Cohesion: 0.05
Nodes (37): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+29 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (50): LightingWindow, advanced_, allChannels_, animation_, animationControls_, applicationPages_, blend_, catalog_ (+42 more)

### Community 35 - "LightingWindow::LightingWindow"
Cohesion: 0.16
Nodes (4): spin, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem

### Community 36 - "ModuleRegistry"
Cohesion: 0.19
Nodes (3): fail(), ModuleRegistry, entries_

### Community 37 - "Lighting editor"
Cohesion: 0.12
Nodes (17): Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Layers and masks, Layout (+9 more)

### Community 38 - "OutputPanel"
Cohesion: 0.09
Nodes (14): OutputPanel, authorize_, authorizing_, connections_, endpoints_, message_, OutputPanel::OutputPanel(), public (+6 more)

### Community 39 - "LightingWindow.h"
Cohesion: 0.11
Nodes (15): ColorEditor, ColorListEditor, QCheckBox, QComboBox, QDoubleSpinBox, QLabel, QLineEdit, QListWidget (+7 more)

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.29
Nodes (7): conflictReason(), creationTime(), matchesProcessIdentity(), normalized(), processPath(), requiresManualStop(), winError()

### Community 42 - "EffectRenderer.cpp"
Cohesion: 0.22
Nodes (14): effectTransform(), interpolate(), isGradient(), isParticle(), periodic(), polygon(), renderAtTime(), renderBaseEffect() (+6 more)

### Community 44 - "QJsonObject"
Cohesion: 0.10
Nodes (13): channelMapping(), ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), loadComponentDefinition(), requiredText(), DeviceProfile::fromJson() (+5 more)

### Community 45 - "NativeEffect"
Cohesion: 0.12
Nodes (12): Responsibilities, NativeEffect, brightness, colors, id, inward, speed, RgbColor (+4 more)

### Community 46 - "ConflictDialog"
Cohesion: 0.16
Nodes (11): ConflictDialog, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_, table_, terminateAll_ (+3 more)

### Community 47 - "GeometryView"
Cohesion: 0.25
Nodes (3): GeometryView, commit, scale

### Community 48 - "SignalForge editor reference"
Cohesion: 0.29
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "test_output_ui.cpp"
Cohesion: 0.15
Nodes (6): configure(), connection(), identity(), OutputUiTest, profile(), state()

### Community 51 - "ColorListEditor"
Cohesion: 0.10
Nodes (13): Component catalog schema 1, ColorListEditor, add_, gradient_, maximum_, minimum_, public, remove_ (+5 more)

### Community 52 - "Hardware modules"
Cohesion: 0.20
Nodes (9): Build and inspect, Compatibility and trust, Hardware discovery, Hardware modules, LED discovery, Output-session ownership, Profile schema 1, Startup process conflicts (+1 more)

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
Cohesion: 0.27
Nodes (6): assertClosedOnWorker(), factory(), frame(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".load"
Cohesion: 0.24
Nodes (5): frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "StudioWidgets.cpp"
Cohesion: 0.11
Nodes (9): ChannelEditor::ChannelEditor(), ComponentEditor::ComponentEditor(), applyStudioStyle(), ColorEditor::ColorEditor(), disclosure(), installParameterTooltips(), sliderField(), studioToolIcon() (+1 more)

### Community 63 - "PaletteStrip"
Cohesion: 0.15
Nodes (10): PaletteStrip, add, colors, dragging, gradient, move, positions, remove (+2 more)

### Community 64 - "test_editor.cpp"
Cohesion: 0.14
Nodes (7): document_, addEffect(), catalog(), channelProfile(), EditorTest, private, layer()

### Community 65 - "BoardIdentity"
Cohesion: 0.20
Nodes (6): BoardIdentity, manufacturer, model, ProfileCandidate, device, profileId

### Community 66 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 67 - "OutputScheduler"
Cohesion: 0.12
Nodes (8): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_, identity()

### Community 68 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 72 - "Status"
Cohesion: 0.33
Nodes (4): Status, error, generation, state

### Community 73 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

### Community 77 - "OutputFrame"
Cohesion: 0.14
Nodes (10): ChannelOutput, channelId, colors, native, software, OutputFrame, channels, endpoint (+2 more)

### Community 78 - "authorizeHardwareOutput"
Cohesion: 0.20
Nodes (8): readBoardIdentity(), main(), authorizeHardwareOutput(), confirmOutputReports(), loadModules(), present(), previewOutputReports(), sameEndpoint()

### Community 79 - ".edit"
Cohesion: 0.28
Nodes (5): channelAssignmentJson(), parseChannelAssignment(), state_, validate(), EditorTest::periodicEffectsAndGradientStops()

### Community 81 - "EffectKind"
Cohesion: 0.09
Nodes (22): EffectKind, Burst, Circle, ConicalGradient, Cross, Diamond, Fireflies, Heart (+14 more)

### Community 84 - "EffectRenderer.h"
Cohesion: 0.15
Nodes (11): EffectDefinition, id, kind, label, EffectLayer, EffectParameter, EffectLayer, id (+3 more)

### Community 85 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 86 - "ConflictProcess"
Cohesion: 0.15
Nodes (7): ConflictProcess, executable, manualStop, path, pid, reason, started

### Community 87 - "LightingWindow.cpp"
Cohesion: 0.08
Nodes (5): button(), colorIcon(), LayerDelegate, LayerTree, reordered

### Community 88 - "HidDescriptor"
Cohesion: 0.13
Nodes (11): deviceIdentity(), findProfileCandidates(), HidDescriptor, interfaceNumber, manufacturer, path, product, productId (+3 more)

### Community 89 - "Tool"
Cohesion: 0.20
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 90 - ".connect"
Cohesion: 0.17
Nodes (7): GUI authorization and presence, Output scheduler, Output session backend, Verification, nativeKey(), sameOutputScope(), validateFrame()

### Community 92 - "FakeProcesses"
Cohesion: 0.20
Nodes (7): FakeProcesses, denied, failScan, forced, graceful, restart, running

### Community 94 - ".rebuild"
Cohesion: 0.28
Nodes (3): ColorListEditor::ColorListEditor(), begin, end

### Community 98 - "QDialog"
Cohesion: 0.25
Nodes (3): ChannelEditor, initial_, ComponentEditor

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.40
Nodes (4): Data and transport boundaries, Design principles checked, Motion audit and implementation, Studio interaction and motion review

### Community 102 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 103 - "Self-Review"
Cohesion: 0.40
Nodes (4): Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan

## Knowledge Gaps
- **542 isolated node(s):** `channels_`, `armed_`, `state_`, `manufacturer`, `model` (+537 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 846 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **29 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `LightingDocument`, `BoardIdentity`, `test_editor.cpp`, `LightingWindow::LightingWindow`, `OutputPanel`, `LightingWindow.h`, `.projectFile`, `authorizeHardwareOutput`, `.edit`, `HardwarePicker`, `test_output_ui.cpp`, `LightingWindow.cpp`, `LightingCanvas`, `StudioWidgets.cpp`, `AppSession`?**
  _High betweenness centrality (0.153) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `File Structure`, `IDriverModule.h`, `test_hid_backend.cpp`, `OutputSession`, `MsiDriver`, `IDriverModule`, `BuildReport0x51`, `Recording`, `SignalRGB repository research`, `ModuleRegistry`, `NativeEffect`, `Record`, `Transport`, `RecordingTransport`, `MsiProtocol.h`, `.connect`, `MsiModule`, `Self-Review`?**
  _High betweenness centrality (0.131) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `.connect` to `LightingDocument`, `OutputScheduler`, `ModuleRegistry`, `OutputFrame`, `README.md`, `.load`?**
  _High betweenness centrality (0.103) - this node is a cross-community bridge._
- **Are the 20 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 20 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _542 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.09666666666666666 - nodes in this community are weakly interconnected._
- **Should `File Structure` be split into smaller, more focused modules?**
  _Cohesion score 0.14166666666666666 - nodes in this community are weakly interconnected._