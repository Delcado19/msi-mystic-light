# Graph Report - msi-mystic-light  (2026-10-25)

## Corpus Check
- 138 files · ~149,616 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 7 file(s) not represented in the graph (top: (none) 2, .cmake 2, .cmd 1)

## Summary
- 2155 nodes · 4561 edges · 128 communities (93 shown, 35 thin omitted)
- Extraction: 81% EXTRACTED · 19% INFERRED · 0% AMBIGUOUS · INFERRED: 875 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4286f36c`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- AuxiliaryTransport
- MsiColor
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- NativeEffect
- HidApiDevice
- MsiDriver
- MsiGen1AdvArea
- Development Tools
- LightingWindow.h
- MsiLightingMode
- test_editor.cpp
- AeroBackend
- IDriverModule
- HardwarePicker
- PawnIoRam.cpp
- Recording
- AuxiliaryDriver
- Implementation status
- README.md
- ChannelAssignment
- LightingCanvas
- Mystic Light — Overall Architecture
- LightingCanvas.cpp
- `Mystic Light Online Data.dat` — decryption and content
- Reference repository reuse review
- MSI Mystic Light - Lighting Studio
- BuildKoneColorsReport
- main.cpp
- OutputSession
- EffectLayer
- LightingWindow
- IHidDevice
- .cancelGesture
- Lighting editor
- QPainter
- OutputConnection
- ConflictProcesses.cpp
- BuildReport0x51
- HardwarePicker.cpp
- OutputPanel
- authorizeHardwareOutput
- ConflictDialog
- GeometryView
- Output session backend
- Record
- test_output_ui.cpp
- ColorListEditor
- DriverInstance
- Self-Review
- ConflictProcess
- ComponentPlacement
- .latestDuringSlowWrite
- .connect
- StartupTest
- Transport
- EneRamProtocol
- PaletteStrip
- package_release.py
- impl_
- I2cInfo
- OutputScheduler
- LightingState
- .start
- Windows RGB access investigation
- Status
- FakeProcesses
- OutputPanel::OutputPanel
- stdexcept
- LightingDocument.h
- LightingWindow.cpp
- SignalRGB repository research
- EffectKind
- LightingDocument
- ComponentsTest
- RecordingTransport
- EffectRenderer.h
- .edit
- .pushColors
- Tool
- .load
- StudioWidgets.h
- colorIcon
- .rebuild
- SignalForge editor reference
- PeripheralDriver
- PeripheralModule
- SignalRGB GitHub topic triage
- Connection
- Studio interaction and motion review
- MsiZoneSettings
- VirtualModule
- Test review: recent lighting changes (2026-10-24)
- IConflictProcesses
- Hardware modules
- ComponentEditor
- findProfileCandidates
- Record
- PawnIO AMD SMBus module
- Lighting Studio — Design Doc (v2)
- .setStops
- layer
- closeWindow
- .paint
- SnapGrid
- StudioWidgets.cpp
- QPointF
- validateChannelAssignments
- Lighting Studio 0.1.0
- DocumentEdit
- MsiModule

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 156 edges
2. `LightingCanvas` - 129 edges
3. `EditorTest` - 75 edges
4. `Implementation status` - 58 edges
5. `EffectLayer` - 52 edges
6. `LightingDocument` - 47 edges
7. `OutputPanel` - 46 edges
8. `catalog()` - 46 edges
9. `IHidDevice` - 44 edges
10. `document_` - 42 edges

## Surprising Connections (you probably didn't know these)
- `Application structure` --references--> `QPainter`  [INFERRED]
  docs/superpowers/specs/2026-10-19-lighting-studio-design.md → src/editor/LightingCanvas.h
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Task 18: Final wire-up and real-hardware verification` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → src/hid/IHidDevice.h
- `Inventory and pinned revisions` --references--> `main()`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/main.cpp
- `Output scheduler` --references--> `OutputScheduler`  [INFERRED]
  docs/OUTPUT_SESSION.md → src/output/OutputScheduler.h

## Import Cycles
- None detected.

## Communities (128 total, 35 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.10
Nodes (19): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+11 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.14
Nodes (6): AuxiliaryTransport, gpu_, ram_, ready_, openAuxiliaryTransport(), ~Impl()

### Community 2 - "MsiColor"
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

### Community 6 - "NativeEffect"
Cohesion: 0.12
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 7 - "HidApiDevice"
Cohesion: 0.05
Nodes (23): hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer, path (+15 more)

### Community 8 - "MsiDriver"
Cohesion: 0.14
Nodes (10): MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_, RgbColor, b (+2 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "LightingWindow.h"
Cohesion: 0.11
Nodes (16): ColorEditor, ColorListEditor, QCheckBox, QComboBox, QDoubleSpinBox, QLabel, QLineEdit, QListWidget (+8 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "test_editor.cpp"
Cohesion: 0.09
Nodes (8): loadComponentDefinition(), document_, addEffect(), artifactDirectory(), catalog(), channelProfile(), EditorTest, QTemporaryDir

### Community 14 - "AeroBackend"
Cohesion: 0.12
Nodes (14): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+6 more)

### Community 15 - "IDriverModule"
Cohesion: 0.09
Nodes (5): Adding support, AuxiliaryModule, IDriverModule, IncompatibleModule, MissingInterface

### Community 16 - "HardwarePicker"
Cohesion: 0.10
Nodes (15): ChannelEditor, initial_, HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice (+7 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.14
Nodes (9): version, call(), libraryPath(), locked(), onPrimary(), PawnIoRam, error, pawnIoRamDiscoveryError() (+1 more)

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 21 - "Implementation status"
Cohesion: 0.04
Nodes (56): Accepted effect authoring follow-ups (2026-10-23), Canvas scrollbars and right-hand minimap (2026-10-24), Canvas Start/Stop, fullscreen and growing Layout (2026-10-24), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23) (+48 more)

### Community 23 - "ChannelAssignment"
Cohesion: 0.17
Nodes (10): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+2 more)

### Community 24 - "LightingCanvas"
Cohesion: 0.03
Nodes (55): LightingCanvas, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, cropTool_, dragging_, drawColor_ (+47 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 27 - "`Mystic Light Online Data.dat` — decryption and content"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 28 - "Reference repository reuse review"
Cohesion: 0.20
Nodes (10): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+2 more)

### Community 29 - "MSI Mystic Light - Lighting Studio"
Cohesion: 0.29
Nodes (7): Contents, MSI Mystic Light - Lighting Studio, Per-device LED arrangement, Preparing a Windows release, Related repos, Research documents, Standalone app implementation

### Community 30 - "BuildKoneColorsReport"
Cohesion: 0.25
Nodes (3): Task 10: Roccat Kone Aimo driver, BuildKoneColorsReport(), BuildKoneInitReport()

### Community 31 - "main.cpp"
Cohesion: 0.10
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 32 - "OutputSession"
Cohesion: 0.08
Nodes (18): ChannelOutput, channelId, colors, native, software, nativeKey(), OutputFrame, channels (+10 more)

### Community 33 - "EffectLayer"
Cohesion: 0.05
Nodes (41): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+33 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (49): LightingWindow, advanced_, allChannels_, animation_, animationControls_, applicationPages_, blend_, catalog_ (+41 more)

### Community 35 - "IHidDevice"
Cohesion: 0.10
Nodes (8): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, IHidDevice, FakeHidDevice, opened_, openSucceeds, sendSucceeds, sentReports

### Community 36 - ".cancelGesture"
Cohesion: 0.12
Nodes (3): MaskTool, Brush, Gradient

### Community 37 - "Lighting editor"
Cohesion: 0.06
Nodes (31): Addressable LEDs and NV5 MKII evidence, Canvas playback, fullscreen and growing Layout, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Gradient travel and blend verification (2026-10-24) (+23 more)

### Community 38 - "QPainter"
Cohesion: 0.19
Nodes (14): effectTransform(), interpolate(), isGradient(), periodic(), polygon(), renderAtTime(), renderBaseEffect(), renderClippedBaseEffect() (+6 more)

### Community 39 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.35
Nodes (7): conflictReason(), creationTime(), matchesProcessIdentity(), normalized(), processPath(), requiresManualStop(), winError()

### Community 42 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 43 - "HardwarePicker.cpp"
Cohesion: 0.18
Nodes (8): hardwareCategory(), hardwareManufacturer(), hardwareName(), generic(), HardwarePicker::HardwarePicker(), installedProducts(), isFan(), productIcon()

### Community 44 - "OutputPanel"
Cohesion: 0.10
Nodes (15): OutputPanel, authorizing_, automatic_, configured_, connections_, endpoints_, message_, pausedChannels_ (+7 more)

### Community 45 - "authorizeHardwareOutput"
Cohesion: 0.09
Nodes (15): scanAuxiliaryHardware(), deviceIdentity(), authorizeHardwareOutput(), loadModules(), OutputProof, controlIdentity, identity, initial (+7 more)

### Community 46 - "ConflictDialog"
Cohesion: 0.16
Nodes (11): ConflictDialog, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_, table_, terminateAll_ (+3 more)

### Community 47 - "GeometryView"
Cohesion: 0.17
Nodes (5): GeometryView, commit, moved, pressPosition, scale

### Community 48 - "Output session backend"
Cohesion: 0.33
Nodes (6): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "test_output_ui.cpp"
Cohesion: 0.18
Nodes (8): previewFrame_, configure(), connection(), identity(), OutputUiTest, QTemporaryDir, profile(), state()

### Community 51 - "ColorListEditor"
Cohesion: 0.10
Nodes (13): Component catalog schema 1, ColorListEditor, add_, gradient_, maximum_, minimum_, public, remove_ (+5 more)

### Community 52 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 53 - "Self-Review"
Cohesion: 0.40
Nodes (4): Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan

### Community 54 - "ConflictProcess"
Cohesion: 0.15
Nodes (7): ConflictProcess, executable, manualStop, path, pid, reason, started

### Community 55 - "ComponentPlacement"
Cohesion: 0.22
Nodes (5): ComponentPlacement, componentId, id, origin, rotation

### Community 56 - ".latestDuringSlowWrite"
Cohesion: 0.24
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.16
Nodes (7): prepareOutput(), validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "EneRamProtocol"
Cohesion: 0.21
Nodes (4): EneRamProtocol, address_, transfer_, AuxiliaryTest

### Community 63 - "PaletteStrip"
Cohesion: 0.17
Nodes (10): PaletteStrip, add, colors, dragging, gradient, move, positions, remove (+2 more)

### Community 64 - "package_release.py"
Cohesion: 0.09
Nodes (4): package(), promote(), ReleasePackageTest, ScreenshotPromotionTest

### Community 65 - "impl_"
Cohesion: 0.33
Nodes (5): impl_, address, library, ready, sent

### Community 66 - "I2cInfo"
Cohesion: 0.20
Nodes (10): I2cInfo, address, ddc, displayMask, port, portSet, reg, regSize (+2 more)

### Community 67 - "OutputScheduler"
Cohesion: 0.13
Nodes (7): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "Status"
Cohesion: 0.20
Nodes (8): Status, error, generation, state, State, Connected, Disconnected, Faulted

### Community 73 - "FakeProcesses"
Cohesion: 0.20
Nodes (7): FakeProcesses, denied, failScan, forced, graceful, restart, running

### Community 74 - "OutputPanel::OutputPanel"
Cohesion: 0.20
Nodes (4): channelKey(), authorize_, OutputPanel::OutputPanel(), stateName()

### Community 78 - "LightingDocument.h"
Cohesion: 0.22
Nodes (6): LightingState, LayerBlend, Multiply, Normal, Overlay, Screen

### Community 79 - "LightingWindow.cpp"
Cohesion: 0.09
Nodes (5): button(), LayerTree, reordered, LightingWindow::LightingWindow(), scan_

### Community 80 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 81 - "EffectKind"
Cohesion: 0.09
Nodes (22): EffectKind, Burst, Circle, ConicalGradient, Cross, Diamond, Fireflies, Heart (+14 more)

### Community 83 - "LightingDocument"
Cohesion: 0.11
Nodes (7): LightingDocument, initialCatalog_, LightingDocument::LightingDocument(), profiles_, public, placedPoints(), validate()

### Community 84 - "ComponentsTest"
Cohesion: 0.10
Nodes (12): channelMapping(), ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), requiredText(), DeviceProfile::fromJson(), DeviceProfile::load() (+4 more)

### Community 85 - "RecordingTransport"
Cohesion: 0.14
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 86 - "EffectRenderer.h"
Cohesion: 0.12
Nodes (11): EffectDefinition, id, kind, label, EffectLayer, EffectParameter, EffectLayer, id (+3 more)

### Community 87 - ".edit"
Cohesion: 0.19
Nodes (3): channelAssignmentJson(), parseChannelAssignment(), state_

### Community 88 - ".pushColors"
Cohesion: 0.11
Nodes (10): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), feature(), output(), BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport(), CalculateCrc(), BuildVulcanColorPackets(), BuildVulcanEnableDirectReport() (+2 more)

### Community 89 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 90 - ".load"
Cohesion: 0.12
Nodes (5): main(), fail(), ModuleRegistry, entries_, ModulesTest

### Community 92 - "StudioWidgets.h"
Cohesion: 0.15
Nodes (10): ColorEditor, hex_, picker_, public, QDoubleSpinBox, QLabel, QLineEdit, QPushButton (+2 more)

### Community 94 - ".rebuild"
Cohesion: 0.32
Nodes (3): ColorListEditor::ColorListEditor(), begin, end

### Community 95 - "SignalForge editor reference"
Cohesion: 0.29
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 96 - "PeripheralDriver"
Cohesion: 0.33
Nodes (3): PeripheralDriver, channel_, initialized_

### Community 98 - "SignalRGB GitHub topic triage"
Cohesion: 0.33
Nodes (4): Priorities, Scope, licensing and evidence, SignalRGB GitHub topic triage, Useful later or only for matching hardware

### Community 99 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.18
Nodes (10): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Endpoint list correction and restored backends (2026-10-24), Gradient/output feedback correction (2026-10-24), Layout feedback and hardware output controls (2026-10-24), Motion audit and implementation (+2 more)

### Community 102 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 107 - "Hardware modules"
Cohesion: 0.15
Nodes (12): Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, LED discovery, Native GPU endpoint, Output-session ownership (+4 more)

### Community 108 - "ComponentEditor"
Cohesion: 0.22
Nodes (4): ComponentEditor, category_, manufacturer_, nonPhysicalSlots_

### Community 109 - "findProfileCandidates"
Cohesion: 0.15
Nodes (8): BoardIdentity, manufacturer, model, findProfileCandidates(), ProfileCandidate, device, profileId, readBoardIdentity()

### Community 110 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 112 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 117 - "SnapGrid"
Cohesion: 0.29
Nodes (3): SnapGrid, enabled, spacing

### Community 118 - "StudioWidgets.cpp"
Cohesion: 0.10
Nodes (8): ChannelEditor::ChannelEditor(), ComponentEditor::ComponentEditor(), applyStudioStyle(), disclosure(), installParameterTooltips(), sliderField(), studioToolIcon(), syncSliderFields()

### Community 121 - "Lighting Studio 0.1.0"
Cohesion: 0.50
Nodes (3): Changes, Lighting Studio 0.1.0, Requirements and limitations

## Knowledge Gaps
- **675 isolated node(s):** `channel_`, `channels_`, `armed_`, `state_`, `shieldSent_` (+670 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1052 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **35 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `AuxiliaryTransport`, `MsiColor`, `NativeEffect`, `HidApiDevice`, `MsiDriver`, `IDriverModule`, `Recording`, `AuxiliaryDriver`, `OutputSession`, `BuildReport0x51`, `Self-Review`, `.connect`, `Transport`, `stdexcept`, `SignalRGB repository research`, `RecordingTransport`, `.pushColors`, `.load`, `AuxiliaryHardware.cpp`, `PeripheralDriver`, `PeripheralModule`, `VirtualModule`, `Record`, `MsiModule`?**
  _High betweenness centrality (0.116) - this node is a cross-community bridge._
- **Why does `LightingWindow` connect `LightingWindow` to `.load`, `OutputPanel::OutputPanel`, `LightingWindow.h`, `OutputPanel`, `findProfileCandidates`, `test_editor.cpp`, `LightingWindow.cpp`, `HardwarePicker`, `test_output_ui.cpp`, `LightingDocument`, `layer`, `.edit`, `LightingCanvas`, `.projectFile`, `.previewPlacements`, `.eventFilter`, `main.cpp`?**
  _High betweenness centrality (0.114) - this node is a cross-community bridge._
- **Why does `LightingCanvas` connect `LightingCanvas` to `OutputSession`, `LightingWindow`, `.cancelGesture`, `OutputPanel`, `LightingDocument`, `QPointF`, `StudioWidgets.cpp`, `EffectRenderer.h`, `Tool`, `LightingCanvas.cpp`?**
  _High betweenness centrality (0.093) - this node is a cross-community bridge._
- **Are the 39 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 39 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channel_`, `channels_`, `armed_` to the rest of the system?**
  _675 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.09782608695652174 - nodes in this community are weakly interconnected._
- **Should `AuxiliaryTransport` be split into smaller, more focused modules?**
  _Cohesion score 0.14166666666666666 - nodes in this community are weakly interconnected._