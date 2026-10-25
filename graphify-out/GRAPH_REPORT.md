# Graph Report - msi-mystic-light  (2026-10-25)

## Corpus Check
- 138 files · ~150,225 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 7 file(s) not represented in the graph (top: (none) 2, .cmake 2, .cmd 1)

## Summary
- 2171 nodes · 4597 edges · 121 communities (86 shown, 35 thin omitted)
- Extraction: 81% EXTRACTED · 19% INFERRED · 0% AMBIGUOUS · INFERRED: 880 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `fce35249`
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
- ComponentEditor.cpp
- Recording
- AuxiliaryDriver
- Implementation status
- README.md
- ChannelAssignment
- LightingCanvas
- Mystic Light — Overall Architecture
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
- stdexcept
- HardwarePicker.cpp
- OutputPanel
- authorizeHardwareOutput
- ConflictDialog
- GeometryView
- OutputProof
- Record
- test_output_ui.cpp
- ColorListEditor
- DriverInstance
- ModulesTest
- AuxiliaryModule
- ComponentPlacement
- test_scheduler.cpp
- .connect
- LayerTree
- Transport
- EneRamProtocol
- PaletteStrip
- package_release.py
- PawnIoRam
- I2cInfo
- OutputScheduler
- LightingState
- .start
- Windows RGB access investigation
- Status
- deviceIdentity
- OutputPanel.cpp
- AuxiliaryTest
- LightingDocument.h
- LightingWindow.cpp
- SignalRGB repository research
- EffectKind
- LightingDocument
- ComponentCatalog.cpp
- RecordingTransport
- EffectRenderer.h
- state_
- .pushColors
- Tool
- .load
- LightingDocument::LightingDocument
- colorIcon
- SignalForge editor reference
- PeripheralModule
- SignalRGB GitHub topic triage
- Studio interaction and motion review
- MsiZoneSettings
- VirtualModule
- Test review: recent lighting changes (2026-10-24)
- Hardware modules
- ComponentEditor
- findProfileCandidates
- Record
- PawnIO AMD SMBus module
- Lighting Studio — Design Doc (v2)
- StudioWidgets.cpp
- PreviewDivider
- SnapGrid.h
- ChannelEditor::ChannelEditor
- QPointF
- Lighting Studio 0.1.0
- DocumentEdit
- MsiModule

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 158 edges
2. `LightingCanvas` - 129 edges
3. `EditorTest` - 77 edges
4. `Implementation status` - 59 edges
5. `EffectLayer` - 52 edges
6. `LightingDocument` - 47 edges
7. `catalog()` - 47 edges
8. `OutputPanel` - 46 edges
9. `IHidDevice` - 44 edges
10. `document_` - 43 edges

## Surprising Connections (you probably didn't know these)
- `Application structure` --references--> `QPainter`  [INFERRED]
  docs/superpowers/specs/2026-10-19-lighting-studio-design.md → src/editor/LightingCanvas.h
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Task 18: Final wire-up and real-hardware verification` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → src/hid/IHidDevice.h
- `Inventory and pinned revisions` --references--> `main()`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/main.cpp
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp

## Import Cycles
- None detected.

## Communities (121 total, 35 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.10
Nodes (19): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+11 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.16
Nodes (5): AuxiliaryTransport, gpu_, ram_, ready_, openAuxiliaryTransport()

### Community 2 - "MsiColor"
Cohesion: 0.12
Nodes (17): File Structure, Global Constraints, Review Focus, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness, Task 4: Component data model (+9 more)

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
Cohesion: 0.10
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 7 - "HidApiDevice"
Cohesion: 0.05
Nodes (23): hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer, path (+15 more)

### Community 8 - "MsiDriver"
Cohesion: 0.13
Nodes (11): Responsibilities, MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_, RgbColor (+3 more)

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
Cohesion: 0.08
Nodes (8): document_, addEffect(), artifactDirectory(), catalog(), channelProfile(), EditorTest, QTemporaryDir, layer()

### Community 14 - "AeroBackend"
Cohesion: 0.12
Nodes (14): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+6 more)

### Community 15 - "IDriverModule"
Cohesion: 0.14
Nodes (3): IDriverModule, IncompatibleModule, MissingInterface

### Community 16 - "HardwarePicker"
Cohesion: 0.09
Nodes (15): ChannelEditor, initial_, HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice (+7 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.16
Nodes (8): version, call(), libraryPath(), locked(), onPrimary(), error, pawnIoRamDiscoveryError(), scanPawnIoRam()

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 21 - "Implementation status"
Cohesion: 0.04
Nodes (57): Accepted effect authoring follow-ups (2026-10-23), Canvas scrollbars and right-hand minimap (2026-10-24), Canvas Start/Stop, fullscreen and growing Layout (2026-10-24), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23) (+49 more)

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
Cohesion: 0.06
Nodes (23): Connection, approved, generation, scheduler, ChannelOutput, channelId, colors, native (+15 more)

### Community 33 - "EffectLayer"
Cohesion: 0.05
Nodes (41): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+33 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (49): LightingWindow, advanced_, allChannels_, animation_, animationControls_, applicationPages_, blend_, catalog_ (+41 more)

### Community 35 - "IHidDevice"
Cohesion: 0.09
Nodes (10): Completed: HID transport (2026-10-23), Self-Review, Task 3: HID transport interface and hidapi backend, Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, IHidDevice, FakeHidDevice, opened_, openSucceeds (+2 more)

### Community 36 - ".cancelGesture"
Cohesion: 0.10
Nodes (4): LightingCanvas::LightingCanvas(), MaskTool, Brush, Gradient

### Community 37 - "Lighting editor"
Cohesion: 0.06
Nodes (32): Addressable LEDs and NV5 MKII evidence, Canvas playback, fullscreen and growing Layout, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Gradient travel and blend verification (2026-10-24) (+24 more)

### Community 38 - "QPainter"
Cohesion: 0.24
Nodes (15): effectTransform(), interpolate(), isGradient(), isParticle(), periodic(), polygon(), renderAtTime(), renderBaseEffect() (+7 more)

### Community 39 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.05
Nodes (25): closeWindow(), ConflictProcess, executable, manualStop, path, pid, reason, started (+17 more)

### Community 42 - "stdexcept"
Cohesion: 0.15
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 43 - "HardwarePicker.cpp"
Cohesion: 0.18
Nodes (8): hardwareCategory(), hardwareManufacturer(), hardwareName(), generic(), HardwarePicker::HardwarePicker(), installedProducts(), isFan(), productIcon()

### Community 44 - "OutputPanel"
Cohesion: 0.09
Nodes (16): OutputPanel, authorize_, authorizing_, automatic_, configured_, connections_, endpoints_, message_ (+8 more)

### Community 45 - "authorizeHardwareOutput"
Cohesion: 0.20
Nodes (8): scanAuxiliaryHardware(), readBoardIdentity(), main(), authorizeHardwareOutput(), loadModules(), present(), sameEndpoint(), vulcanControlIdentity()

### Community 46 - "ConflictDialog"
Cohesion: 0.16
Nodes (11): ConflictDialog, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_, table_, terminateAll_ (+3 more)

### Community 47 - "GeometryView"
Cohesion: 0.14
Nodes (5): GeometryView, commit, moved, pressPosition, scale

### Community 48 - "OutputProof"
Cohesion: 0.22
Nodes (6): OutputProof, controlIdentity, identity, initial, profile, reports

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "test_output_ui.cpp"
Cohesion: 0.20
Nodes (8): previewFrame_, configure(), connection(), identity(), OutputUiTest, QTemporaryDir, profile(), state()

### Community 51 - "ColorListEditor"
Cohesion: 0.07
Nodes (23): Component catalog schema 1, ColorEditor, hex_, picker_, public, ColorListEditor, add_, gradient_ (+15 more)

### Community 52 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 55 - "ComponentPlacement"
Cohesion: 0.22
Nodes (5): ComponentPlacement, componentId, id, origin, rotation

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.26
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.18
Nodes (7): previewOutputReports(), prepareOutput(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "EneRamProtocol"
Cohesion: 0.31
Nodes (3): EneRamProtocol, address_, transfer_

### Community 63 - "PaletteStrip"
Cohesion: 0.14
Nodes (12): PaletteStrip, add, begin, colors, dragging, end, gradient, move (+4 more)

### Community 64 - "package_release.py"
Cohesion: 0.09
Nodes (4): package(), promote(), ReleasePackageTest, ScreenshotPromotionTest

### Community 65 - "PawnIoRam"
Cohesion: 0.18
Nodes (7): ~Impl(), PawnIoRam, impl_, address, library, ready, sent

### Community 66 - "I2cInfo"
Cohesion: 0.20
Nodes (10): I2cInfo, address, ddc, displayMask, port, portSet, reg, regSize (+2 more)

### Community 67 - "OutputScheduler"
Cohesion: 0.10
Nodes (13): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification, OutputScheduler, mutex_ (+5 more)

### Community 68 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "Status"
Cohesion: 0.20
Nodes (8): Status, error, generation, state, State, Connected, Disconnected, Faulted

### Community 74 - "OutputPanel.cpp"
Cohesion: 0.27
Nodes (3): channelKey(), OutputPanel::OutputPanel(), stateName()

### Community 78 - "LightingDocument.h"
Cohesion: 0.24
Nodes (6): LightingState, LayerBlend, Multiply, Normal, Overlay, Screen

### Community 80 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 81 - "EffectKind"
Cohesion: 0.09
Nodes (22): EffectKind, Burst, Circle, ConicalGradient, Cross, Diamond, Fireflies, Heart (+14 more)

### Community 83 - "LightingDocument"
Cohesion: 0.10
Nodes (7): validateChannelAssignments(), LightingDocument, initialCatalog_, profiles_, public, placedPoints(), validate()

### Community 84 - "ComponentCatalog.cpp"
Cohesion: 0.07
Nodes (14): channelMapping(), ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), loadComponentDefinition(), requiredText(), DeviceProfile::fromJson() (+6 more)

### Community 85 - "RecordingTransport"
Cohesion: 0.14
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 86 - "EffectRenderer.h"
Cohesion: 0.15
Nodes (11): EffectDefinition, id, kind, label, EffectLayer, EffectParameter, EffectLayer, id (+3 more)

### Community 87 - "state_"
Cohesion: 0.22
Nodes (3): channelAssignmentJson(), parseChannelAssignment(), state_

### Community 88 - ".pushColors"
Cohesion: 0.09
Nodes (13): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport() (+5 more)

### Community 89 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 90 - ".load"
Cohesion: 0.20
Nodes (3): fail(), ModuleRegistry, entries_

### Community 95 - "SignalForge editor reference"
Cohesion: 0.29
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 98 - "SignalRGB GitHub topic triage"
Cohesion: 0.33
Nodes (4): Priorities, Scope, licensing and evidence, SignalRGB GitHub topic triage, Useful later or only for matching hardware

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
Nodes (12): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, LED discovery, Native GPU endpoint (+4 more)

### Community 108 - "ComponentEditor"
Cohesion: 0.22
Nodes (4): ComponentEditor, category_, manufacturer_, nonPhysicalSlots_

### Community 109 - "findProfileCandidates"
Cohesion: 0.12
Nodes (8): BoardIdentity, manufacturer, model, findProfileCandidates(), ProfileCandidate, device, profileId, importLegacyHardware()

### Community 110 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 112 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 116 - "PreviewDivider"
Cohesion: 0.12
Nodes (3): LayerDelegate, PreviewDivider, PreviewSplitter

### Community 117 - "SnapGrid.h"
Cohesion: 0.25
Nodes (3): SnapGrid, enabled, spacing

### Community 118 - "ChannelEditor::ChannelEditor"
Cohesion: 0.15
Nodes (8): ChannelEditor::ChannelEditor(), ComponentEditor::ComponentEditor(), applyStudioStyle(), disclosure(), installParameterTooltips(), sliderField(), studioToolIcon(), syncSliderFields()

### Community 121 - "Lighting Studio 0.1.0"
Cohesion: 0.50
Nodes (3): Changes, Lighting Studio 0.1.0, Requirements and limitations

## Knowledge Gaps
- **676 isolated node(s):** `channel_`, `channels_`, `armed_`, `state_`, `shieldSent_` (+671 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1052 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **35 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `OutputPanel.cpp`, `LightingWindow.h`, `OutputPanel`, `findProfileCandidates`, `test_editor.cpp`, `LightingWindow.cpp`, `HardwarePicker`, `authorizeHardwareOutput`, `test_output_ui.cpp`, `LightingDocument`, `state_`, `LightingCanvas`, `.projectFile`, `.previewPlacements`, `main.cpp`?**
  _High betweenness centrality (0.115) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `AuxiliaryTransport`, `NativeEffect`, `HidApiDevice`, `MsiDriver`, `Recording`, `AuxiliaryDriver`, `OutputSession`, `stdexcept`, `AuxiliaryModule`, `.connect`, `Transport`, `SignalRGB repository research`, `msiprotocol`, `ComponentCatalog.cpp`, `RecordingTransport`, `.pushColors`, `.load`, `AuxiliaryHardware.cpp`, `PeripheralModule`, `VirtualModule`, `Hardware modules`, `Record`?**
  _High betweenness centrality (0.114) - this node is a cross-community bridge._
- **Why does `LightingCanvas` connect `LightingCanvas` to `OutputSession`, `LightingWindow`, `.cancelGesture`, `NativeEffect`, `OutputPanel`, `LightingDocument`, `EffectRenderer.h`, `QPointF`, `Tool`, `LightingCanvas.cpp`?**
  _High betweenness centrality (0.092) - this node is a cross-community bridge._
- **Are the 41 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 41 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channel_`, `channels_`, `armed_` to the rest of the system?**
  _676 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.09782608695652174 - nodes in this community are weakly interconnected._
- **Should `MsiColor` be split into smaller, more focused modules?**
  _Cohesion score 0.11695906432748537 - nodes in this community are weakly interconnected._