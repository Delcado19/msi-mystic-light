# Graph Report - msi-mystic-light  (2026-10-27)

## Corpus Check
- 147 files · ~325,068 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 9 file(s) not represented in the graph (top: (none) 2, .qrc 2, .cmake 2)

## Summary
- 2397 nodes · 5206 edges · 135 communities (93 shown, 42 thin omitted)
- Extraction: 79% EXTRACTED · 21% INFERRED · 0% AMBIGUOUS · INFERRED: 1074 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `ae98e9e3`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- IDriver
- AuxiliaryTransport
- BuildReport0x51
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
- test_editor.cpp
- AeroBackend
- OutputPanel
- ColorListEditor
- PawnIoRam.cpp
- ComponentEditor.cpp
- Recording
- HardwarePicker
- Implementation status
- README.md
- IDriverModule
- LightingCanvas
- Mystic Light — Overall Architecture
- `Mystic Light Online Data.dat` — decryption and content
- Reference repository reuse review
- MSI Mystic Light - Lighting Studio
- LightingWindow.h
- OutputPanel.cpp
- LightingCanvas.cpp
- EffectLayer
- LightingWindow
- IHidDevice
- Lighting editor
- Record
- OutputConnection
- ConflictProcess
- MsiColor
- SignalRGB repository research
- test_output_ui.cpp
- Effect editor interaction research
- ConflictDialog
- GeometryView
- OutputSession
- Record
- Lighting Studio — Design Doc (v2)
- ComponentCatalog.cpp
- DriverInstance
- test_hid_backend.cpp
- RecordingTransport
- ComponentPlacement
- test_scheduler.cpp
- .connect
- .render
- Transport
- EneRamProtocol
- PaletteStrip
- package_release.py
- impl_
- I2cInfo
- OutputScheduler
- FakeProcesses
- .start
- Windows RGB access investigation
- State
- main.cpp
- FakeHidDevice
- Tool
- QSet
- LightingWindow.cpp
- LightingState
- EffectKind
- ScreenSampler
- LightingDocument
- MsiZoneSettings
- ConflictProcesses.cpp
- DeviceProfile.cpp
- .edit
- .pushColors
- .cancelGesture
- .load
- BuildKoneColorsReport
- ComponentsTest
- AuxiliaryDriver
- algorithm
- Lighting Studio 0.1.0
- PeripheralModule
- Studio interaction and motion review
- Lighting Studio GUI and interaction audit
- VirtualModule
- Test review: recent lighting changes (2026-10-24)
- StudioWidgets.cpp
- Hardware modules
- QPainter
- Connection
- BuildApex3ColorsPacket
- PawnIO AMD SMBus module
- ChannelAssignment
- findProfileCandidates
- CONTINUE.md
- SignalForge editor reference
- IConflictProcesses
- StartupTest
- validateChannelAssignments
- .mouseMoveEvent
- Editor interaction implementation
- authorizeHardwareOutput
- DocumentEdit
- main
- Self-Review
- PawnIoRam
- LightingDocument.h
- THIRD_PARTY_NOTICES.md
- closeWindow
- branding/README.md

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 179 edges
2. `LightingCanvas` - 145 edges
3. `EditorTest` - 100 edges
4. `Implementation status` - 66 edges
5. `LightingDocument` - 65 edges
6. `catalog()` - 61 edges
7. `EffectLayer` - 54 edges
8. `document_` - 52 edges
9. `OutputPanel` - 47 edges
10. `IHidDevice` - 44 edges

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

## Communities (135 total, 42 thin omitted)

### Community 0 - "IDriver"
Cohesion: 0.10
Nodes (17): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+9 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.14
Nodes (5): AuxiliaryTransport, gpu_, ram_, ready_, openAuxiliaryTransport()

### Community 2 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

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
Cohesion: 0.07
Nodes (17): hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer, path (+9 more)

### Community 8 - "MsiDriver"
Cohesion: 0.13
Nodes (11): Responsibilities, MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_, RgbColor (+3 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "LightingWindow::LightingWindow"
Cohesion: 0.14
Nodes (4): spin, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "test_editor.cpp"
Cohesion: 0.07
Nodes (8): loadComponentDefinition(), document_, addEffect(), artifactDirectory(), catalog(), channelProfile(), EditorTest, QTemporaryDir

### Community 14 - "AeroBackend"
Cohesion: 0.15
Nodes (11): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+3 more)

### Community 15 - "OutputPanel"
Cohesion: 0.09
Nodes (17): OutputPanel, authorize_, authorizing_, automatic_, channelDetails_, configured_, connections_, endpoints_ (+9 more)

### Community 16 - "ColorListEditor"
Cohesion: 0.07
Nodes (23): Component catalog schema 1, ColorEditor, hex_, picker_, public, ColorListEditor, add_, gradient_ (+15 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.16
Nodes (8): version, call(), libraryPath(), locked(), onPrimary(), error, pawnIoRamDiscoveryError(), scanPawnIoRam()

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "HardwarePicker"
Cohesion: 0.05
Nodes (27): drawHardwareArtwork(), drawHardwareComponentArtwork(), hardwareArtworkCategory(), hardwareBodyBounds(), hardwarePreviewLedRadius(), initializeHardwareArtwork(), hardwareCategory(), hardwareManufacturer() (+19 more)

### Community 21 - "Implementation status"
Cohesion: 0.03
Nodes (64): Accepted effect authoring follow-ups (2026-10-23), Aero visibility and build cleanup (2026-10-25), Artwork geometry and frame regression fixes (2026-10-25), Canvas scrollbars and right-hand minimap (2026-10-24), Canvas Start/Stop, fullscreen and growing Layout (2026-10-24), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23) (+56 more)

### Community 23 - "IDriverModule"
Cohesion: 0.10
Nodes (5): Adding support, AuxiliaryModule, IDriverModule, IncompatibleModule, MissingInterface

### Community 24 - "LightingCanvas"
Cohesion: 0.03
Nodes (60): LightingCanvas, activeLayer_, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, createdObject_, cropTool_ (+52 more)

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
Cohesion: 0.29
Nodes (7): Contents, MSI Mystic Light - Lighting Studio, Per-device LED arrangement, Preparing a Windows release, Related repos, Research documents, Standalone app implementation

### Community 30 - "LightingWindow.h"
Cohesion: 0.12
Nodes (15): ColorEditor, ColorListEditor, QCheckBox, QComboBox, QDoubleSpinBox, QLabel, QLineEdit, QListWidget (+7 more)

### Community 31 - "OutputPanel.cpp"
Cohesion: 0.27
Nodes (3): channelKey(), OutputPanel::OutputPanel(), stateName()

### Community 33 - "EffectLayer"
Cohesion: 0.04
Nodes (42): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+34 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (57): LightingWindow, activeLayer_, adjustingPanels_, advanced_, allChannels_, animation_, animationControls_, applicationPages_ (+49 more)

### Community 37 - "Lighting editor"
Cohesion: 0.05
Nodes (41): Addressable LEDs and NV5 MKII evidence, Artwork alignment and custom device geometry, Canvas playback, fullscreen and growing Layout, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Detailed hardware artwork (+33 more)

### Community 38 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 39 - "OutputConnection"
Cohesion: 0.40
Nodes (3): OutputConnection, initial, present

### Community 41 - "ConflictProcess"
Cohesion: 0.20
Nodes (7): ConflictProcess, executable, manualStop, path, pid, reason, started

### Community 42 - "MsiColor"
Cohesion: 0.14
Nodes (15): File Structure, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness, Task 3: HID transport interface and hidapi backend, Task 4: Component data model, Task 5: Channel model, Task 7: Effect interface and built-in effects (+7 more)

### Community 43 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 44 - "test_output_ui.cpp"
Cohesion: 0.21
Nodes (8): previewFrame_, configure(), connection(), identity(), OutputUiTest, QTemporaryDir, profile(), state()

### Community 45 - "Effect editor interaction research"
Cohesion: 0.25
Nodes (8): Acceptance and implementation gates, Effect editor interaction research, Findings in current source, Interaction contract, Interface and identity, Recommended content model, Reference patterns, Scope and evidence

### Community 46 - "ConflictDialog"
Cohesion: 0.16
Nodes (11): ConflictDialog, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_, table_, terminateAll_ (+3 more)

### Community 47 - "GeometryView"
Cohesion: 0.09
Nodes (10): ComponentEditor, category_, manufacturer_, nonPhysicalSlots_, GeometryView, artwork, commit, moved (+2 more)

### Community 48 - "OutputSession"
Cohesion: 0.08
Nodes (18): ChannelOutput, channelId, colors, native, software, nativeKey(), OutputFrame, channels (+10 more)

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 51 - "ComponentCatalog.cpp"
Cohesion: 0.17
Nodes (6): channelMapping(), ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), requiredText()

### Community 52 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 53 - "test_hid_backend.cpp"
Cohesion: 0.23
Nodes (6): HID_API_CALL hid_close(), HID_API_CALL hid_error(), HID_API_CALL hid_free_enumeration(), HID_API_CALL hid_send_feature_report(), HID_API_CALL hid_write(), hid_device_

### Community 54 - "RecordingTransport"
Cohesion: 0.14
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 55 - "ComponentPlacement"
Cohesion: 0.22
Nodes (5): ComponentPlacement, componentId, id, origin, rotation

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.26
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.17
Nodes (7): prepareOutput(), validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 58 - ".render"
Cohesion: 0.18
Nodes (3): channelAssignmentJson(), parseChannelAssignment(), layer()

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "EneRamProtocol"
Cohesion: 0.21
Nodes (4): EneRamProtocol, address_, transfer_, AuxiliaryTest

### Community 63 - "PaletteStrip"
Cohesion: 0.15
Nodes (11): PaletteStrip, add, colors, dragging, end, gradient, move, positions (+3 more)

### Community 64 - "package_release.py"
Cohesion: 0.09
Nodes (4): package(), promote(), ReleasePackageTest, ScreenshotPromotionTest

### Community 65 - "impl_"
Cohesion: 0.33
Nodes (5): impl_, address, library, ready, sent

### Community 66 - "I2cInfo"
Cohesion: 0.14
Nodes (13): aeroColorPacket(), I2cInfo, address, data, ddc, displayMask, port, portSet (+5 more)

### Community 67 - "OutputScheduler"
Cohesion: 0.10
Nodes (13): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification, OutputScheduler, mutex_ (+5 more)

### Community 68 - "FakeProcesses"
Cohesion: 0.20
Nodes (7): FakeProcesses, denied, failScan, forced, graceful, restart, running

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "State"
Cohesion: 0.20
Nodes (8): Status, error, generation, state, State, Connected, Disconnected, Faulted

### Community 73 - "main.cpp"
Cohesion: 0.10
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 74 - "FakeHidDevice"
Cohesion: 0.15
Nodes (8): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, Task 3: Canvas-global placement and "follow global effect", FakeHidDevice, opened_, openSucceeds, sendSucceeds, sentReports

### Community 77 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 79 - "LightingWindow.cpp"
Cohesion: 0.05
Nodes (7): button(), colorIcon(), LayerDelegate, LayerTree, reordered, PreviewDivider, PreviewSplitter

### Community 80 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 81 - "EffectKind"
Cohesion: 0.05
Nodes (49): EffectDefinition, id, kind, label, EffectKind, Burst, Circle, ConicalGradient (+41 more)

### Community 82 - "ScreenSampler"
Cohesion: 0.07
Nodes (12): ColorPopup, finished, gestureBegun, sampling, SaturationValueField, brightness, changed, hue (+4 more)

### Community 83 - "LightingDocument"
Cohesion: 0.08
Nodes (13): LightingDocument, colorPreviewBefore_, componentNames_, frameCache_, frameRevision_, frameSeconds_, initialCatalog_, LightingDocument::LightingDocument() (+5 more)

### Community 84 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 85 - "ConflictProcesses.cpp"
Cohesion: 0.29
Nodes (7): conflictReason(), creationTime(), matchesProcessIdentity(), normalized(), processPath(), requiresManualStop(), winError()

### Community 86 - "DeviceProfile.cpp"
Cohesion: 0.36
Nodes (6): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text(), QJsonObject

### Community 88 - ".pushColors"
Cohesion: 0.11
Nodes (10): feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport(), CalculateCrc() (+2 more)

### Community 89 - ".cancelGesture"
Cohesion: 0.12
Nodes (3): MaskTool, Brush, Gradient

### Community 90 - ".load"
Cohesion: 0.14
Nodes (4): fail(), ModuleRegistry, entries_, ModulesTest

### Community 91 - "BuildKoneColorsReport"
Cohesion: 0.25
Nodes (3): Task 10: Roccat Kone Aimo driver, BuildKoneColorsReport(), BuildKoneInitReport()

### Community 96 - "Lighting Studio 0.1.0"
Cohesion: 0.50
Nodes (3): Changes, Lighting Studio 0.1.0, Requirements and limitations

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.14
Nodes (14): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Endpoint list correction and restored backends (2026-10-24), Fixed optional component captions (2026-10-26), Gaming lighting workspace review implementation (2026-10-26), Gradient/output feedback correction (2026-10-24) (+6 more)

### Community 102 - "Lighting Studio GUI and interaction audit"
Cohesion: 0.18
Nodes (10): Color picker, Consolidation, Evidence, Findings, ordered by impact, Identity and direction, Implementation follow-up, Implementation order and acceptance, Integer scales (+2 more)

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 106 - "StudioWidgets.cpp"
Cohesion: 0.07
Nodes (15): ChannelEditor::ChannelEditor(), openStudioColorPicker(), ComponentEditor::ComponentEditor(), ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), disclosure(), installParameterTooltips(), begin (+7 more)

### Community 107 - "Hardware modules"
Cohesion: 0.15
Nodes (12): Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, LED discovery, Native GPU endpoint (+4 more)

### Community 108 - "QPainter"
Cohesion: 0.16
Nodes (3): SnapGrid, enabled, spacing

### Community 109 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 110 - "BuildApex3ColorsPacket"
Cohesion: 0.36
Nodes (3): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), BuildApex3BrightnessPacket(), BuildApex3ColorsPacket()

### Community 112 - "ChannelAssignment"
Cohesion: 0.13
Nodes (13): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+5 more)

### Community 113 - "findProfileCandidates"
Cohesion: 0.12
Nodes (9): BoardIdentity, manufacturer, model, findProfileCandidates(), ProfileCandidate, device, profileId, readBoardIdentity() (+1 more)

### Community 114 - "CONTINUE.md"
Cohesion: 0.33
Nodes (5): Dense strip SVG visibility regression (2026-10-26), Fixed optional component captions (2026-10-26), Gaming lighting workspace implementation (2026-10-26), GUI audit implementation and selected branding (2026-10-26), Layout presentation and playback fixes (2026-10-26)

### Community 115 - "SignalForge editor reference"
Cohesion: 0.29
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 121 - "Editor interaction implementation"
Cohesion: 0.33
Nodes (6): Editor interaction implementation, Identity and color sampling, Persistence and limits, Pointer behavior, Rendering and output status, Result

### Community 122 - "authorizeHardwareOutput"
Cohesion: 0.09
Nodes (15): scanAuxiliaryHardware(), deviceIdentity(), authorizeHardwareOutput(), loadModules(), OutputProof, controlIdentity, identity, initial (+7 more)

### Community 125 - "Self-Review"
Cohesion: 0.40
Nodes (4): Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan

### Community 130 - "LightingDocument.h"
Cohesion: 0.25
Nodes (6): LightingState, LayerBlend, Multiply, Normal, Overlay, Screen

## Knowledge Gaps
- **757 isolated node(s):** `channel_`, `channels_`, `armed_`, `state_`, `shieldSent_` (+752 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1157 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **42 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `.previewPlacements`, `LightingWindow::LightingWindow`, `test_editor.cpp`, `OutputPanel`, `HardwarePicker`, `LightingCanvas`, `LightingWindow.h`, `OutputPanel.cpp`, `test_output_ui.cpp`, `.render`, `main.cpp`, `LightingWindow.cpp`, `EffectKind`, `LightingDocument`, `.edit`, `findProfileCandidates`, `QString`, `main`, `.confirmDiscard`?**
  _High betweenness centrality (0.142) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `IDriver`, `AuxiliaryTransport`, `BuildReport0x51`, `NativeEffect`, `HidApiDevice`, `MsiDriver`, `Recording`, `IDriverModule`, `Record`, `MsiColor`, `SignalRGB repository research`, `OutputSession`, `RecordingTransport`, `.connect`, `Transport`, `FakeHidDevice`, `.pushColors`, `.load`, `stdexcept`, `AuxiliaryDriver`, `PeripheralModule`, `VirtualModule`, `BuildApex3ColorsPacket`, `Self-Review`?**
  _High betweenness centrality (0.115) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `OutputScheduler` to `OutputSession`, `LightingDocument`, `README.md`, `.connect`, `.load`?**
  _High betweenness centrality (0.080) - this node is a cross-community bridge._
- **Are the 50 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 50 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channel_`, `channels_`, `armed_` to the rest of the system?**
  _757 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `IDriver` be split into smaller, more focused modules?**
  _Cohesion score 0.1038961038961039 - nodes in this community are weakly interconnected._
- **Should `AuxiliaryTransport` be split into smaller, more focused modules?**
  _Cohesion score 0.13725490196078433 - nodes in this community are weakly interconnected._