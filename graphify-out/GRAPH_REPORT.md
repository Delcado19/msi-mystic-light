# Graph Report - msi-mystic-light  (2026-10-25)

## Corpus Check
- 141 files · ~156,732 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 8 file(s) not represented in the graph (top: (none) 2, .cmake 2, .cmd 1)

## Summary
- 2226 nodes · 4784 edges · 133 communities (98 shown, 35 thin omitted)
- Extraction: 80% EXTRACTED · 20% INFERRED · 0% AMBIGUOUS · INFERRED: 962 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `bcb3407b`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- IDriver
- AuxiliaryTransport
- MsiColor
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- NativeEffect
- HidApiDevice.cpp
- MsiDriver
- MsiGen1AdvArea
- Development Tools
- BuildReport0x51
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
- qfile
- LightingCanvas
- Mystic Light — Overall Architecture
- `Mystic Light Online Data.dat` — decryption and content
- Reference repository reuse review
- MSI Mystic Light - Lighting Studio
- findProfileCandidates
- PreviewDivider
- LightingCanvas.cpp
- EffectLayer
- LightingWindow
- IHidDevice
- ComponentsTest
- Lighting editor
- ComponentEditor
- OutputConnection
- ConflictProcesses.cpp
- ConflictProcess
- StartupTest
- OutputPanel
- EffectRenderer.cpp
- ConflictDialog
- GeometryView
- OutputSession
- Record
- test_output_ui.cpp
- ColorListEditor
- DriverInstance
- FakeProcesses
- RecordingTransport
- ComponentPlacement
- test_scheduler.cpp
- .connect
- IConflictProcesses
- Transport
- EneRamProtocol
- PaletteStrip
- package_release.py
- PawnIoRam
- I2cInfo
- OutputScheduler
- .render
- .start
- Windows RGB access investigation
- Status
- QTimer
- OutputPanel.cpp
- OutputFrame
- LayerBlend
- LightingWindow::LightingWindow
- SignalRGB repository research
- EffectKind
- LightingDocument
- Output session backend
- Connection
- ComponentCatalog.cpp
- .edit
- .pushColors
- Tool
- .load
- ChannelAssignment
- closeWindow
- LightingWindow.cpp
- ModulesTest
- SignalForge editor reference
- MsiZoneSettings
- PeripheralModule
- LightingDocument.h
- Studio interaction and motion review
- FakeHidDevice
- VirtualModule
- Test review: recent lighting changes (2026-10-24)
- StudioWidgets.cpp
- Hardware modules
- test_hid_backend.cpp
- HidDescriptor
- Record
- PawnIO AMD SMBus module
- Lighting Studio — Design Doc (v2)
- EffectParameter
- stdexcept
- DeviceProfile.cpp
- cmath
- SnapGrid.h
- BuildBasiliskColorsReport
- QPointF
- Lighting Studio 0.1.0
- ProfileCandidate
- DocumentEdit
- .cancelGesture
- Self-Review
- HidApiDevice
- BuildApex3ColorsPacket
- AuxiliaryModule
- authorizeHardwareOutput
- HidRuntime

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 160 edges
2. `LightingCanvas` - 133 edges
3. `EditorTest` - 88 edges
4. `Implementation status` - 66 edges
5. `EffectLayer` - 52 edges
6. `catalog()` - 51 edges
7. `LightingDocument` - 48 edges
8. `OutputPanel` - 46 edges
9. `document_` - 45 edges
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

## Communities (133 total, 35 thin omitted)

### Community 0 - "IDriver"
Cohesion: 0.10
Nodes (17): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+9 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.16
Nodes (5): AuxiliaryTransport, gpu_, ram_, ready_, openAuxiliaryTransport()

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
Cohesion: 0.10
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 8 - "MsiDriver"
Cohesion: 0.13
Nodes (11): Responsibilities, MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_, RgbColor (+3 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "test_editor.cpp"
Cohesion: 0.08
Nodes (7): document_, addEffect(), artifactDirectory(), catalog(), channelProfile(), EditorTest, QTemporaryDir

### Community 14 - "AeroBackend"
Cohesion: 0.12
Nodes (14): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+6 more)

### Community 15 - "IDriverModule"
Cohesion: 0.07
Nodes (10): MsiModule, IDriverModule, OutputProof, controlIdentity, identity, initial, profile, reports (+2 more)

### Community 16 - "HardwarePicker"
Cohesion: 0.06
Nodes (27): drawHardwareArtwork(), drawHardwareComponentArtwork(), hardwareArtworkCategory(), hardwareBodyBounds(), initializeHardwareArtwork(), QPainter, hardwareCategory(), hardwareManufacturer() (+19 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.16
Nodes (8): version, call(), libraryPath(), locked(), onPrimary(), error, pawnIoRamDiscoveryError(), scanPawnIoRam()

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 21 - "Implementation status"
Cohesion: 0.03
Nodes (64): Accepted effect authoring follow-ups (2026-10-23), Aero visibility and build cleanup (2026-10-25), Artwork geometry and frame regression fixes (2026-10-25), Canvas scrollbars and right-hand minimap (2026-10-24), Canvas Start/Stop, fullscreen and growing Layout (2026-10-24), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23) (+56 more)

### Community 22 - "README.md"
Cohesion: 0.20
Nodes (3): Entwurf: Kommentar für GitLab-Issues #4935 / #5429, Hardware artwork, Third-party runtime components

### Community 24 - "LightingCanvas"
Cohesion: 0.03
Nodes (56): LightingCanvas, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, cropTool_, dragging_, drawColor_ (+48 more)

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

### Community 30 - "findProfileCandidates"
Cohesion: 0.13
Nodes (9): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), readBoardIdentity(), present(), sameEndpoint() (+1 more)

### Community 31 - "PreviewDivider"
Cohesion: 0.12
Nodes (3): LayerDelegate, PreviewDivider, PreviewSplitter

### Community 33 - "EffectLayer"
Cohesion: 0.05
Nodes (40): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+32 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (64): ColorEditor, ColorListEditor, LightingWindow, advanced_, allChannels_, animation_, animationControls_, applicationPages_ (+56 more)

### Community 35 - "IHidDevice"
Cohesion: 0.11
Nodes (5): Task 18: Final wire-up and real-hardware verification, PeripheralDriver, channel_, initialized_, IHidDevice

### Community 37 - "Lighting editor"
Cohesion: 0.05
Nodes (38): Addressable LEDs and NV5 MKII evidence, Artwork alignment and custom device geometry, Canvas playback, fullscreen and growing Layout, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Detailed hardware artwork (+30 more)

### Community 38 - "ComponentEditor"
Cohesion: 0.29
Nodes (4): ComponentEditor, category_, manufacturer_, nonPhysicalSlots_

### Community 39 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.35
Nodes (7): conflictReason(), creationTime(), matchesProcessIdentity(), normalized(), processPath(), requiresManualStop(), winError()

### Community 42 - "ConflictProcess"
Cohesion: 0.15
Nodes (7): ConflictProcess, executable, manualStop, path, pid, reason, started

### Community 44 - "OutputPanel"
Cohesion: 0.10
Nodes (16): OutputPanel, authorize_, authorizing_, automatic_, configured_, connections_, endpoints_, message_ (+8 more)

### Community 45 - "EffectRenderer.cpp"
Cohesion: 0.27
Nodes (11): effectTransform(), interpolate(), isGradient(), periodic(), polygon(), renderAtTime(), renderBaseEffect(), renderClippedBaseEffect() (+3 more)

### Community 46 - "ConflictDialog"
Cohesion: 0.19
Nodes (10): ConflictDialog, busy_, conflicts_, ignore_, status_, table_, terminateAll_, QLabel (+2 more)

### Community 47 - "GeometryView"
Cohesion: 0.14
Nodes (6): GeometryView, artwork, commit, moved, pressPosition, scale

### Community 48 - "OutputSession"
Cohesion: 0.12
Nodes (9): nativeKey(), OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_ (+1 more)

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

### Community 53 - "FakeProcesses"
Cohesion: 0.20
Nodes (7): FakeProcesses, denied, failScan, forced, graceful, restart, running

### Community 54 - "RecordingTransport"
Cohesion: 0.14
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 55 - "ComponentPlacement"
Cohesion: 0.12
Nodes (10): ComponentPlacement, componentId, id, origin, rotation, LightingState, assignments, components (+2 more)

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.26
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.23
Nodes (5): frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "EneRamProtocol"
Cohesion: 0.19
Nodes (4): EneRamProtocol, address_, transfer_, AuxiliaryTest

### Community 63 - "PaletteStrip"
Cohesion: 0.13
Nodes (11): PaletteStrip, add, colors, dragging, end, gradient, move, positions (+3 more)

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
Cohesion: 0.12
Nodes (8): Output scheduler, OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - ".render"
Cohesion: 0.17
Nodes (7): channelAssignmentJson(), parseChannelAssignment(), isParticle(), renderEffect(), validateEffect(), validate(), layer()

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "Status"
Cohesion: 0.20
Nodes (8): Status, error, generation, state, State, Connected, Disconnected, Faulted

### Community 73 - "QTimer"
Cohesion: 0.16
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 74 - "OutputPanel.cpp"
Cohesion: 0.27
Nodes (3): channelKey(), OutputPanel::OutputPanel(), stateName()

### Community 77 - "OutputFrame"
Cohesion: 0.13
Nodes (11): ChannelOutput, channelId, colors, native, software, OutputFrame, channels, endpoint (+3 more)

### Community 78 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

### Community 79 - "LightingWindow::LightingWindow"
Cohesion: 0.13
Nodes (4): spin, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem

### Community 80 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 81 - "EffectKind"
Cohesion: 0.09
Nodes (22): EffectKind, Burst, Circle, ConicalGradient, Cross, Diamond, Fireflies, Heart (+14 more)

### Community 83 - "LightingDocument"
Cohesion: 0.10
Nodes (6): LightingDocument, initialCatalog_, LightingDocument::LightingDocument(), profiles_, public, placedPoints()

### Community 84 - "Output session backend"
Cohesion: 0.40
Nodes (5): Automatic GUI output and presence, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification

### Community 85 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 86 - "ComponentCatalog.cpp"
Cohesion: 0.16
Nodes (6): channelMapping(), ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), requiredText()

### Community 87 - ".edit"
Cohesion: 0.19
Nodes (3): loadComponentDefinition(), state_, applyStudioStyle()

### Community 88 - ".pushColors"
Cohesion: 0.18
Nodes (4): feature(), output(), BuildVulcanColorPackets(), BuildVulcanEnableDirectReport()

### Community 89 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 90 - ".load"
Cohesion: 0.24
Nodes (4): main(), fail(), ModuleRegistry, entries_

### Community 91 - "ChannelAssignment"
Cohesion: 0.13
Nodes (13): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+5 more)

### Community 93 - "LightingWindow.cpp"
Cohesion: 0.07
Nodes (3): colorIcon(), LayerTree, reordered

### Community 95 - "SignalForge editor reference"
Cohesion: 0.29
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 96 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.18
Nodes (10): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Endpoint list correction and restored backends (2026-10-24), Gradient/output feedback correction (2026-10-24), Layout feedback and hardware output controls (2026-10-24), Motion audit and implementation (+2 more)

### Community 102 - "FakeHidDevice"
Cohesion: 0.15
Nodes (8): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, Task 3: Canvas-global placement and "follow global effect", FakeHidDevice, opened_, openSucceeds, sendSucceeds, sentReports

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 106 - "StudioWidgets.cpp"
Cohesion: 0.10
Nodes (10): ChannelEditor::ChannelEditor(), ComponentEditor::ComponentEditor(), ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), disclosure(), installParameterTooltips(), begin, sliderField() (+2 more)

### Community 107 - "Hardware modules"
Cohesion: 0.14
Nodes (13): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, LED discovery (+5 more)

### Community 108 - "test_hid_backend.cpp"
Cohesion: 0.21
Nodes (6): HID_API_CALL hid_close(), HID_API_CALL hid_error(), HID_API_CALL hid_free_enumeration(), HID_API_CALL hid_send_feature_report(), HID_API_CALL hid_write(), hid_device_

### Community 109 - "HidDescriptor"
Cohesion: 0.17
Nodes (9): HidDescriptor, interfaceNumber, manufacturer, path, product, productId, usage, usagePage (+1 more)

### Community 110 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 112 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 113 - "EffectParameter"
Cohesion: 0.18
Nodes (10): EffectDefinition, id, kind, label, EffectParameter, EffectLayer, id, label (+2 more)

### Community 114 - "stdexcept"
Cohesion: 0.22
Nodes (3): Task 10: Roccat Kone Aimo driver, BuildKoneColorsReport(), BuildKoneInitReport()

### Community 115 - "DeviceProfile.cpp"
Cohesion: 0.36
Nodes (6): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text(), QJsonObject

### Community 117 - "SnapGrid.h"
Cohesion: 0.24
Nodes (3): SnapGrid, enabled, spacing

### Community 118 - "BuildBasiliskColorsReport"
Cohesion: 0.57
Nodes (3): BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport(), CalculateCrc()

### Community 121 - "Lighting Studio 0.1.0"
Cohesion: 0.50
Nodes (3): Changes, Lighting Studio 0.1.0, Requirements and limitations

### Community 122 - "ProfileCandidate"
Cohesion: 0.18
Nodes (5): scanAuxiliaryHardware(), ProfileCandidate, device, profileId, importLegacyHardware()

### Community 124 - ".cancelGesture"
Cohesion: 0.12
Nodes (3): MaskTool, Brush, Gradient

### Community 125 - "Self-Review"
Cohesion: 0.40
Nodes (4): Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan

### Community 127 - "HidApiDevice"
Cohesion: 0.29
Nodes (4): hid_device_, HidApiDevice, handle_, initialized_

### Community 129 - "BuildApex3ColorsPacket"
Cohesion: 0.36
Nodes (3): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), BuildApex3BrightnessPacket(), BuildApex3ColorsPacket()

### Community 131 - "authorizeHardwareOutput"
Cohesion: 0.32
Nodes (3): authorizeHardwareOutput(), loadModules(), previewOutputReports()

## Knowledge Gaps
- **695 isolated node(s):** `channel_`, `channels_`, `armed_`, `state_`, `shieldSent_` (+690 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1078 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **35 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `IHidDevice` connect `IHidDevice` to `IDriver`, `BuildApex3ColorsPacket`, `MsiColor`, `AuxiliaryModule`, `AuxiliaryTransport`, `NativeEffect`, `MsiDriver`, `BuildReport0x51`, `Recording`, `AuxiliaryDriver`, `qfile`, `OutputSession`, `RecordingTransport`, `.connect`, `Transport`, `SignalRGB repository research`, `.load`, `PeripheralModule`, `FakeHidDevice`, `VirtualModule`, `Hardware modules`, `Record`, `Self-Review`, `AuxiliaryHardware.cpp`, `HidApiDevice`?**
  _High betweenness centrality (0.125) - this node is a cross-community bridge._
- **Why does `LightingWindow` connect `LightingWindow` to `.eventFilter`, `.render`, `.load`, `QTimer`, `OutputPanel.cpp`, `OutputPanel`, `test_editor.cpp`, `LightingWindow::LightingWindow`, `HardwarePicker`, `test_output_ui.cpp`, `LightingDocument`, `.edit`, `LightingCanvas`, `ProfileCandidate`, `LightingWindow.cpp`?**
  _High betweenness centrality (0.100) - this node is a cross-community bridge._
- **Why does `LightingCanvas` connect `LightingCanvas` to `LightingCanvas.cpp`, `LightingDocument.h`, `LightingWindow`, `NativeEffect`, `HardwarePicker`, `LightingDocument`, `QPointF`, `.select`, `Tool`, `.paintEvent`, `.cancelGesture`?**
  _High betweenness centrality (0.066) - this node is a cross-community bridge._
- **Are the 43 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 43 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channel_`, `channels_`, `armed_` to the rest of the system?**
  _695 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `IDriver` be split into smaller, more focused modules?**
  _Cohesion score 0.1038961038961039 - nodes in this community are weakly interconnected._
- **Should `MsiColor` be split into smaller, more focused modules?**
  _Cohesion score 0.14166666666666666 - nodes in this community are weakly interconnected._