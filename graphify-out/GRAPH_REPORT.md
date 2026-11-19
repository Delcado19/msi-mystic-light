# Graph Report - msi-mystic-light  (2026-11-19)

## Corpus Check
- 205 files · ~1,710,270 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 21 file(s) not represented in the graph (top: .csv 6, .cmd 3, .bin 3)

## Summary
- 3623 nodes · 8254 edges · 191 communities (147 shown, 44 thin omitted)
- Extraction: 78% EXTRACTED · 22% INFERRED · 0% AMBIGUOUS · INFERRED: 1776 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `a0b2bf92`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- IDriver
- AuxiliaryTransport
- Implementation status
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- ServicePanel
- ChannelEditor::ChannelEditor
- MsiDriver
- MsiGen1AdvArea
- Development Tools
- PreviewDivider
- QMap
- test_editor.cpp
- MsiColor
- BuildReport0x51
- LenovoMotherboardMode
- PawnIoRam.cpp
- .load
- Recording
- LightingWindow.cpp
- CONTINUE.md history archive
- Unified Editor implementation
- IDriverModule
- LightingCanvas
- state_
- Transport
- Mystic Light — Overall Architecture
- Reference repository reuse review
- Lighting Studio
- ServicePanel.cpp
- AsrockPolychromeMode
- HardwareArtwork.cpp
- EffectLayer
- LightingWindow
- OutputSession
- EffectRenderer.cpp
- Lighting editor
- LenovoMotherboardModule.cpp
- ServiceRuntime
- AsrockPolychromeZone
- ComponentEditor.cpp
- IHidDevice
- test_output_ui.cpp
- Lighting Studio — Design Doc (v2)
- ConflictDialog
- GeometryView
- ConflictProcess
- Record
- MsiZoneSettings
- Lighting Studio: whole-interface and unified-editor review
- DriverInstance
- MsiLightingMode
- RecordingTransport
- LightingState
- test_scheduler.cpp
- .connect
- SignalRGB repository research
- Transport
- EneRamProtocol
- LightingCanvas.cpp
- inventory_hardware.py
- ServiceRuntime.cpp
- StartupTest
- OutputScheduler
- FakeHidDevice
- OutputScheduler.cpp
- Windows RGB access investigation
- Status
- src/main.cpp
- ColorListEditor
- OutputFrame
- ComponentPlacement
- test_service.cpp
- PaletteStrip
- EffectKind
- SaturationValueField
- .edit
- ChannelAssignment
- ConflictProcesses.cpp
- PawnIO integration and service feasibility
- OutputConnection
- .pushColors
- OutputPanel
- Layer visibility, brush size and Delete
- AuxiliaryDriver
- AeroBackend
- .cancelGesture
- FakeProcesses
- Layer model proposal
- .start
- HardwarePicker
- NativeEffect
- LightingDocument
- Lighting Studio GUI and interaction audit
- OutputPanel.cpp
- Test review: recent lighting changes (2026-10-24)
- VirtualModule
- Gradient travel and blend verification (2026-10-24)
- Lightning and Fireworks visual review (2026-10-29)
- ServiceProtocol.cpp
- authorizeHardwareOutput
- PawnIO AMD SMBus module
- LIGHTING_EDITOR.md
- service/main.cpp
- ServiceServer.cpp
- AsrockPolychromeDriver
- LightingDocument.h
- StudioWidgets.cpp
- Record
- .fromJson
- main
- DeviceConflicts.cpp
- Studio interaction and motion review
- DocumentEdit
- hw_inventory_linux.sh
- QString
- .mousePressEvent
- ComponentDefinition
- closeWindow
- Tool
- ModulesTest
- Connection
- test_hid_backend.cpp
- CONTINUE.md
- Lighting Studio editor UX review
- closeWindow
- QJsonObject
- branding/README.md
- JARGB2 physical response tests - 2026-11-13 / 2026-11-14
- DeviceConflictMonitor
- Hardware migration inventory and priorities
- OutputProof
- HidDescriptor
- UsbWrite
- Capture
- NativeServices
- README.md
- ServiceServer
- ParameterSpinBox
- Hardware support strategy
- Uninstall-LightingStudio.ps1
- ChannelEditor
- impl_
- SignalForge editor reference
- LightingCanvas.h
- PeripheralModule
- Workspace navigation and catalog identity
- Artwork, continuous gestures and spatial controls (GS-06)
- ScCloser
- Distinct source capabilities and particle authoring
- Project files and boundaries
- ComponentEditor
- SnapGrid
- ScCloser
- HidApiDevice
- EffectParameter
- Hardware modules
- eventProperty
- RgbColor
- OutputOnlyHid
- Review Focus
- EffectRenderer.h
- Effect implementation checklist
- ScanMailbox
- ConflictService

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 228 edges
2. `LightingCanvas` - 198 edges
3. `EditorTest` - 149 edges
4. `catalog()` - 96 edges
5. `ServiceRuntime` - 85 edges
6. `document_` - 84 edges
7. `LightingDocument` - 78 edges
8. `EffectLayer` - 71 edges
9. `Implementation status` - 66 edges
10. `IHidDevice` - 59 edges

## Surprising Connections (you probably didn't know these)
- `Project compatibility` --references--> `locked()`  [INFERRED]
  docs/design/UNIFIED_EDITOR_IMPLEMENTATION_2026-10-27.md → src/backends/PawnIoRam.cpp
- `Problem confirmed in the current code` --references--> `LightingWindow`  [INFERRED]
  docs/LAYER_MODEL_PLAN.md → src/editor/LightingWindow.h
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Task 18: Final wire-up and real-hardware verification` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → src/hid/IHidDevice.h
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp

## Import Cycles
- None detected.

## Communities (191 total, 44 thin omitted)

### Community 0 - "IDriver"
Cohesion: 0.08
Nodes (21): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+13 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.12
Nodes (5): AuxiliaryTransport, gpu_, ram_, ready_, openAuxiliaryTransport()

### Community 2 - "Implementation status"
Cohesion: 0.03
Nodes (64): Accepted effect authoring follow-ups (2026-10-23), Aero visibility and build cleanup (2026-10-25), Artwork geometry and frame regression fixes (2026-10-25), Canvas scrollbars and right-hand minimap (2026-10-24), Canvas Start/Stop, fullscreen and growing Layout (2026-10-24), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23) (+56 more)

### Community 3 - "Mystic Light — local network interfaces"
Cohesion: 0.25
Nodes (8): 1. TCP 26820 — local HTTP "Web API" (`MLModule.dll:CWebAPI.cs`), 2. UDP 26822 — mobile app pairing (`CBaseServer.cs`/`CBaseFN.cs`), 3. UDP 26823 — "party sync" between multiple PCs (`CSyncServer.cs`/`CSyncFN.cs`), 4. UDP 61337 — SignalRGB bridge ("LEDKeeper 2" service), Live verification (2026-10-17): the whole MLModule network layer is dead code, Mystic Light — local network interfaces, Protocol (confirmed via code, extends the `Msi.js` reconstruction), Still open

### Community 4 - "Standalone RGB Lighting App — Design"
Cohesion: 0.15
Nodes (13): Accepted effect authoring scope (2026-10-23), Anti-goals, Constraints, Goals, Hardware module architecture (2026-10-23), How it gets made (decided), Intent and why, Non-goals (v1) (+5 more)

### Community 5 - "MSI Mainboard LED Protocol"
Cohesion: 0.22
Nodes (9): MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`), Record layout (16 bytes, per zone/"area"), Report `0x50` ("Gen1"), 290 bytes — status and default configuration, Report `0x51` ("Gen1_Adv"), 727 bytes — per-LED addressing of individual segments, Report `0x90`+port / `0xB0`+port ("Gen2"/"Gen2_Adv") — newer strip model, ⚠️ Safety-relevant finding: the `Store` flag is set on every write, Upstream OpenRGB bug found while cross-checking: `FeaturePacket_PerLED_761` is 727 bytes, not 761, Verification against the existing capture template (+1 more)

### Community 6 - "ServicePanel"
Cohesion: 0.09
Nodes (17): ServicePanel, active_, clockDirty_, commands_, dirty_, enabled_, nextPoll_, outputs_ (+9 more)

### Community 7 - "ChannelEditor::ChannelEditor"
Cohesion: 0.15
Nodes (11): ChannelEditor::ChannelEditor(), NumericFieldStyle, QAbstractSpinBox, sliderField(), sliderPosition(), sliderRange(), SliderWheelStep, remainder_ (+3 more)

### Community 8 - "MsiDriver"
Cohesion: 0.19
Nodes (6): MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "PreviewDivider"
Cohesion: 0.10
Nodes (3): LayerDelegate, PreviewDivider, PreviewSplitter

### Community 12 - "QMap"
Cohesion: 0.25
Nodes (4): SharedRun, firstLed, lastLed, source

### Community 13 - "test_editor.cpp"
Cohesion: 0.06
Nodes (8): Task 1: Gradient stop colors survive Apply, document_, addEffect(), artifactDirectory(), catalog(), channelProfile(), EditorTest, QTemporaryDir

### Community 14 - "MsiColor"
Cohesion: 0.12
Nodes (18): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness, Task 3: HID transport interface and hidapi backend (+10 more)

### Community 15 - "BuildReport0x51"
Cohesion: 0.16
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 16 - "LenovoMotherboardMode"
Cohesion: 0.12
Nodes (12): BuildLenovoMotherboardReports(), LenovoMotherboardMode, Breathing, RainbowWave, Random, Sparkle, Specter, Static (+4 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.16
Nodes (10): call(), ~Impl(), libraryPath(), locked(), onPrimary(), PawnIoRam, error, pawnIoRamDiscoveryError() (+2 more)

### Community 18 - ".load"
Cohesion: 0.10
Nodes (9): fail(), ModuleRegistry, entries_, AsrockPolychromeTest, identity(), profile(), identity(), LenovoMotherboardTest (+1 more)

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "LightingWindow.cpp"
Cohesion: 0.05
Nodes (13): Task 4: Menus are available where they act, once, hardwareManufacturer(), spin, button(), colorIcon(), LayerTree, reordered, LightingWindow::LightingWindow() (+5 more)

### Community 21 - "CONTINUE.md history archive"
Cohesion: 0.07
Nodes (27): Active-layer picking, canvas labels and transport controls (2026-10-28), Authoring controls, pan and two patterns (2026-10-28), Components fullscreen and edit-mode visibility fix (2026-10-28), CONTINUE.md history archive, Dense strip SVG visibility regression (2026-10-26), Editor interaction and channel preview follow-up (2026-10-27), Editor UX, Pan crash and four sources (2026-10-28), Fixed optional component captions (2026-10-26) (+19 more)

### Community 22 - "Unified Editor implementation"
Cohesion: 0.33
Nodes (5): Project compatibility, Rendering, color and numeric behavior, Unified Editor implementation, Verification, Workspace and interaction

### Community 23 - "IDriverModule"
Cohesion: 0.07
Nodes (5): AuxiliaryModule, MsiModule, IDriverModule, IncompatibleModule, MissingInterface

### Community 24 - "LightingCanvas"
Cohesion: 0.02
Nodes (76): LightingCanvas, activeLayer_, additiveMarquee_, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, componentSeconds_ (+68 more)

### Community 25 - "state_"
Cohesion: 0.15
Nodes (6): loadComponentCatalog(), loadComponentDefinition(), hardwareCategory(), hardwareName(), state_, applyStudioStyle()

### Community 26 - "Transport"
Cohesion: 0.13
Nodes (7): Recording, opens, present, writes, Transport, opened, record

### Community 27 - "Mystic Light — Overall Architecture"
Cohesion: 0.14
Nodes (12): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll, Content (+4 more)

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "Lighting Studio"
Cohesion: 0.25
Nodes (8): Building from source, Documentation, Features, Getting started, License, Lighting Studio, Related repos, Supported hardware

### Community 30 - "ServicePanel.cpp"
Cohesion: 0.15
Nodes (9): command(), host(), ServiceConflictProcesses, elevate_, rpc_, ServiceConflictProcesses::ServiceConflictProcesses(), serviceInventory(), ServicePanel::ServicePanel() (+1 more)

### Community 31 - "AsrockPolychromeMode"
Cohesion: 0.13
Nodes (15): AsrockPolychromeMode, Breathing, Cram, Music, Neon, Off, Rainbow, Random (+7 more)

### Community 32 - "HardwareArtwork.cpp"
Cohesion: 0.18
Nodes (7): drawHardwareArtwork(), drawHardwareComponentArtwork(), hardwareArtworkCategory(), hardwareBodyBounds(), hardwarePreviewLedRadius(), initializeHardwareArtwork(), productIcon()

### Community 33 - "EffectLayer"
Cohesion: 0.03
Nodes (59): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+51 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (71): ColorEditor, ColorListEditor, LightingWindow, activeLayer_, adjustingPanels_, advanced_, allChannels_, animation_ (+63 more)

### Community 35 - "OutputSession"
Cohesion: 0.09
Nodes (12): nativeKey(), OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_ (+4 more)

### Community 36 - "EffectRenderer.cpp"
Cohesion: 0.18
Nodes (14): interpolate(), isGradient(), isParticle(), isShape(), noise2d(), periodic(), polygon(), renderAtTime() (+6 more)

### Community 37 - "Lighting editor"
Cohesion: 0.09
Nodes (23): Authoring controls follow-up, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Integer controls, live color and identity (2026-10-26) (+15 more)

### Community 38 - "LenovoMotherboardModule.cpp"
Cohesion: 0.14
Nodes (6): LenovoMotherboardDriver, channels_, LenovoMotherboardModule, mapBrightness(), mapSpeed(), zoneFromAddress()

### Community 39 - "ServiceRuntime"
Cohesion: 0.04
Nodes (42): Retired, endpoint, scheduler, ScanResult, blocked, error, guarded, inventory (+34 more)

### Community 41 - "AsrockPolychromeZone"
Cohesion: 0.13
Nodes (10): AsrockPolychromeZone, Argb1, Argb2, Audio, IoCover, Pcb, Pch, Rgb1 (+2 more)

### Community 42 - "ComponentEditor.cpp"
Cohesion: 0.07
Nodes (5): CoordinateDelegate, QPainter, importLegacyHardware(), updatePlaybackButton(), PeripheralTest

### Community 43 - "IHidDevice"
Cohesion: 0.08
Nodes (15): 162-byte MSI family — corroborated, not conflicting, ASRock Polychrome USB (`26CE:01A2`) — mostly confirmed, one open discrepancy, ASRock Polychrome USB — implemented (2026-10-29, same session), ENE SMBus (RAM) — register map confirmed 3-way, detection method rejected, Lenovo motherboard — implemented (2026-10-29, same session), Not yet cross-checked this pass, Open questions, Our board (`0DB0:0076`) — triple-confirmed, including the shared bug (+7 more)

### Community 44 - "test_output_ui.cpp"
Cohesion: 0.22
Nodes (8): previewFrame_, configure(), connection(), identity(), OutputUiTest, QTemporaryDir, profile(), state()

### Community 45 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 46 - "ConflictDialog"
Cohesion: 0.12
Nodes (13): ConflictDialog, adminServicePid_, adminStop_, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_ (+5 more)

### Community 47 - "GeometryView"
Cohesion: 0.13
Nodes (6): GeometryView, artwork, commit, moved, pressPosition, scale

### Community 48 - "ConflictProcess"
Cohesion: 0.16
Nodes (14): ConflictProcess, elevatedStop, executable, manualStop, path, pid, provenHolder, reason (+6 more)

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 51 - "Lighting Studio: whole-interface and unified-editor review"
Cohesion: 0.18
Nodes (10): Editor interaction contract, Elevation and stable hardware inventory, Evidence and limits, Findings and priorities, Implementation order and acceptance, Layers, colors, numeric controls and appearance, Lighting Studio: whole-interface and unified-editor review, Main decision (+2 more)

### Community 52 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 53 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 54 - "RecordingTransport"
Cohesion: 0.14
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 55 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.27
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.18
Nodes (7): previewOutputReports(), validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 58 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "EneRamProtocol"
Cohesion: 0.19
Nodes (4): EneRamProtocol, address_, transfer_, AuxiliaryTest

### Community 64 - "inventory_hardware.py"
Cohesion: 0.05
Nodes (13): arguments(), build(), git(), integer(), js_return(), priority(), registrations(), snapshot() (+5 more)

### Community 65 - "ServiceRuntime.cpp"
Cohesion: 0.11
Nodes (9): Hung device open and project-only conflict check - 2026-11-17, Task 2: Project edits update running hardware without reopening it, prepareOutput(), sameOutputScope(), Scan, key(), connect_, ServiceRuntime::ServiceRuntime() (+1 more)

### Community 67 - "OutputScheduler"
Cohesion: 0.10
Nodes (13): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification, OutputScheduler, mutex_ (+5 more)

### Community 68 - "FakeHidDevice"
Cohesion: 0.15
Nodes (11): Completed: HID transport (2026-10-23), FakeHidDevice, featureReportResponse, getFeatureReportCalls, inputReportResponse, opened_, openSucceeds, readInputReportCalls (+3 more)

### Community 70 - "OutputScheduler.cpp"
Cohesion: 0.15
Nodes (3): onHidInterface(), OutputScheduler::OutputScheduler(), watchDevices()

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "Status"
Cohesion: 0.29
Nodes (4): Status, error, generation, state

### Community 73 - "src/main.cpp"
Cohesion: 0.08
Nodes (7): AppSession, AppSession::AppSession(), recoveryFile_, timer_, applicationIcon(), PowerResumeFilter, resumed_

### Community 74 - "ColorListEditor"
Cohesion: 0.06
Nodes (23): Component catalog schema 1, Task 5: `BreathingEffect`, Task 7: `CometEffect` and `MarqueeEffect`, ColorEditor, hex_, picker_, public, ColorListEditor (+15 more)

### Community 77 - "OutputFrame"
Cohesion: 0.15
Nodes (11): buildFrames(), ChannelOutput, channelId, colors, native, software, OutputFrame, channels (+3 more)

### Community 78 - "ComponentPlacement"
Cohesion: 0.18
Nodes (6): ComponentPlacement, componentId, id, locked, origin, rotation

### Community 79 - "test_service.cpp"
Cohesion: 0.13
Nodes (8): Child, process, cmd(), identity(), project(), ServiceTest, private, ServiceTest::protocolBounds()

### Community 80 - "PaletteStrip"
Cohesion: 0.17
Nodes (12): PaletteStrip, add, begin, colors, dragging, end, gradient, move (+4 more)

### Community 81 - "EffectKind"
Cohesion: 0.05
Nodes (41): EffectKind, Aurora, Burst, Checkerboard, Circle, ConicalGradient, Cross, Diamond (+33 more)

### Community 82 - "SaturationValueField"
Cohesion: 0.07
Nodes (12): ColorPopup, finished, gestureBegun, sampling, SaturationValueField, brightness, changed, hue (+4 more)

### Community 83 - ".edit"
Cohesion: 0.12
Nodes (7): canCreateEffect(), isPattern(), renderEffect(), validateEffect(), makeSource(), validate(), layer()

### Community 84 - "ChannelAssignment"
Cohesion: 0.10
Nodes (16): chainStarts(), ChannelAssignment, channelId, endpoint, id, native, placements, profileId (+8 more)

### Community 85 - "ConflictProcesses.cpp"
Cohesion: 0.15
Nodes (18): belongsToMysticLightService(), conflictReason(), creationTime(), isSignalRgbService(), matchesProcessIdentity(), mysticLightServiceStopped(), normalized(), processPath() (+10 more)

### Community 86 - "PawnIO integration and service feasibility"
Cohesion: 0.11
Nodes (16): Access control and process boundary, Conclusion, Coverage and next gate, Exact evidence, Installation and licensing boundary, PawnIO integration and service feasibility, Performance and recovery, Primary sources (+8 more)

### Community 87 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 88 - ".pushColors"
Cohesion: 0.09
Nodes (13): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport() (+5 more)

### Community 89 - "OutputPanel"
Cohesion: 0.08
Nodes (17): OutputPanel, authorize_, authorizing_, automatic_, channelDetails_, configured_, connections_, endpoints_ (+9 more)

### Community 90 - "Layer visibility, brush size and Delete"
Cohesion: 0.22
Nodes (9): Artwork alignment and custom device geometry, Detailed hardware artwork, Fixed-device settings versus attachable headers, Gaming workspace and responsive panels, Layer visibility, brush size and Delete, Mask playback, navigation and cosmetic hardware variants, Model labels, fullsize keyboards and zero-angle defaults, Scrollbar contrast and Effects preview divider (+1 more)

### Community 92 - "AeroBackend"
Cohesion: 0.08
Nodes (25): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+17 more)

### Community 93 - ".cancelGesture"
Cohesion: 0.11
Nodes (3): MaskTool, Brush, Gradient

### Community 94 - "FakeProcesses"
Cohesion: 0.05
Nodes (27): FakeProcesses, adminActive, adminNeverStops, adminPendingPolls, adminPollError, adminQueryError, adminRequestError, adminRequests (+19 more)

### Community 95 - "Layer model proposal"
Cohesion: 0.20
Nodes (10): Acceptance matrix, Creation and selection contract, Delivery in reviewable increments, Explain the blends with a live result, Explicitly outside the first implementation, Feasibility evidence and resolved review points, Layer model proposal, Problem confirmed in the current code (+2 more)

### Community 96 - ".start"
Cohesion: 0.19
Nodes (4): DeviceChanges, generation, mutex, wake

### Community 97 - "HardwarePicker"
Cohesion: 0.11
Nodes (14): TODO: custom hardware lives only inside the project - 2026-11-17, HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity (+6 more)

### Community 99 - "NativeEffect"
Cohesion: 0.11
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 100 - "LightingDocument"
Cohesion: 0.08
Nodes (14): sharedColorNote(), LightingDocument, colorPreviewBefore_, componentNames_, frameCache_, frameRevision_, frameSeconds_, identify_ (+6 more)

### Community 102 - "Lighting Studio GUI and interaction audit"
Cohesion: 0.18
Nodes (10): Color picker, Consolidation, Evidence, Findings, ordered by impact, Identity and direction, Implementation follow-up, Implementation order and acceptance, Integer scales (+2 more)

### Community 104 - "OutputPanel.cpp"
Cohesion: 0.25
Nodes (3): channelKey(), OutputPanel::OutputPanel(), stateName()

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 107 - "Gradient travel and blend verification (2026-10-24)"
Cohesion: 0.25
Nodes (8): Canvas playback, fullscreen and growing Layout, Gradient travel and blend verification (2026-10-24), Identify flash (2026-11-15), Live layout movement and output selection, Per-device LED arrangement, Physical LEDs and reserved protocol slots, RAM component geometry correction, Restored integrated lighting

### Community 108 - "Lightning and Fireworks visual review (2026-10-29)"
Cohesion: 0.50
Nodes (3): Before and after, Lightning and Fireworks visual review (2026-10-29), Sources and observation limits

### Community 109 - "ServiceProtocol.cpp"
Cohesion: 0.22
Nodes (13): Installed editor could not reach the service - 2026-11-17, authorizedClient(), currentUserSid(), decode(), encode(), installedHost(), isSystem(), pipeName() (+5 more)

### Community 110 - "authorizeHardwareOutput"
Cohesion: 0.10
Nodes (15): scanAuxiliaryHardware(), BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), ProfileCandidate, device (+7 more)

### Community 112 - "LIGHTING_EDITOR.md"
Cohesion: 0.10
Nodes (14): Collaboration contract, Current status, Decisions and validation boundaries, Final software validation, Package and scope limits, Reliability and layer-design work (2026-11-11), Evidence, Ice workspace - GS-06-R2 (+6 more)

### Community 113 - "service/main.cpp"
Cohesion: 0.16
Nodes (9): control(), knownFolder(), logWarning(), main(), readJson(), report(), run(), serviceMain() (+1 more)

### Community 114 - "ServiceServer.cpp"
Cohesion: 0.14
Nodes (3): ServiceServer::connected(), handler_, ServiceServer::ServiceServer()

### Community 115 - "AsrockPolychromeDriver"
Cohesion: 0.09
Nodes (10): AsrockPolychromeDriver, channels_, state_, AsrockPolychromeModule, mapSpeed(), ZoneState, applied, color (+2 more)

### Community 116 - "LightingDocument.h"
Cohesion: 0.22
Nodes (6): LightingState, LayerBlend, Multiply, Normal, Overlay, Screen

### Community 117 - "StudioWidgets.cpp"
Cohesion: 0.07
Nodes (16): openStudioColorPicker(), ComponentEditor::ComponentEditor(), QToolButton, studioThemeColor(), ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), disclosure(), installParameterTooltips() (+8 more)

### Community 118 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 119 - ".fromJson"
Cohesion: 0.18
Nodes (6): chainOffsets(), chainSharing(), channelMapping(), integer(), requiredText(), ComponentsTest

### Community 121 - "DeviceConflicts.cpp"
Cohesion: 0.17
Nodes (18): Responsive service and installed conflict dialog - 2026-11-16, captureUsbWrites(), deviceInstanceFromPath(), endpointBusy(), endpointLabel(), findWriters(), folderOf(), hidSetReport() (+10 more)

### Community 122 - "Studio interaction and motion review"
Cohesion: 0.06
Nodes (28): Editor interaction implementation, Identity and color sampling, Persistence and limits, Pointer behavior, Rendering and output status, Result, Acceptance and implementation gates, Effect editor interaction research (+20 more)

### Community 125 - "QString"
Cohesion: 0.30
Nodes (10): HandleCloser, hostedService(), imagePath(), openVerified(), processInfo(), requestClose(), stopApprovedProcess(), stopHolder() (+2 more)

### Community 129 - "ComponentDefinition"
Cohesion: 0.14
Nodes (12): ComponentDefinition, category, id, manufacturer, mapping, name, nonPhysicalSlots, positions (+4 more)

### Community 131 - "Tool"
Cohesion: 0.18
Nodes (10): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pan (+2 more)

### Community 134 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 135 - "test_hid_backend.cpp"
Cohesion: 0.19
Nodes (8): HID_API_CALL hid_close(), HID_API_CALL hid_error(), HID_API_CALL hid_free_enumeration(), HID_API_CALL hid_get_feature_report(), HID_API_CALL hid_read_timeout(), HID_API_CALL hid_send_feature_report(), HID_API_CALL hid_write(), hid_device_

### Community 137 - "CONTINUE.md"
Cohesion: 0.11
Nodes (16): Branching and releases - 2026-11-15, CodeRabbit round 2 fixes, R12 - 2026-11-17, Editor fixes and menu cleanup, R16 - 2026-11-17, Editor fixes and small audit fixes - 2026-11-14 (GS-06-R7, build/r4), Event-driven presence check - 2026-11-18, GS-06-R2 UI correction - 2026-11-13, GS-06-R9 - 2026-11-15, Header LED count - 2026-11-15 (+8 more)

### Community 138 - "Lighting Studio editor UX review"
Cohesion: 0.15
Nodes (13): Additional procedural sources, Captures and measured checks, Crash evidence and validation boundaries, Creative-software research, Direction and scope, Findings and implementation, Gaming-Pro-Studio refinement, GS-06 finish (+5 more)

### Community 140 - "QJsonObject"
Cohesion: 0.20
Nodes (7): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text(), QLocalSocket, QJsonObject

### Community 143 - "JARGB2 physical response tests - 2026-11-13 / 2026-11-14"
Cohesion: 0.22
Nodes (8): Approved tests and observations, Follow-up observations - 2026-11-14, JARGB2 physical response tests - 2026-11-13 / 2026-11-14, Pattern 6 - sequential fan-prefix qualification, Sleep/resume event, Software implications, Software status - 2026-11-14, Supported interpretation

### Community 146 - "DeviceConflictMonitor"
Cohesion: 0.09
Nodes (15): DetectedConflicts, list_, owner_, DeviceConflictMonitor, busy_, capture_, lastBusy_, lastError_ (+7 more)

### Community 150 - "Hardware migration inventory and priorities"
Cohesion: 0.33
Nodes (6): Browse the full list, Hardware migration inventory and priorities, Prioritized migration queue, Reproduction and next decisions, Scale and counting units, Source provenance and limitations

### Community 151 - "OutputProof"
Cohesion: 0.20
Nodes (6): OutputProof, controlIdentity, identity, initial, profile, reports

### Community 153 - "HidDescriptor"
Cohesion: 0.15
Nodes (10): hid_device_, HidDescriptor, interfaceNumber, manufacturer, path, product, productId, usage (+2 more)

### Community 154 - "UsbWrite"
Cohesion: 0.15
Nodes (12): ProcessEntry, parent, path, pid, started, UsbWrite, count, device (+4 more)

### Community 155 - "Capture"
Cohesion: 0.20
Nodes (7): Capture, devices, instances, writes, Seen, count, last

### Community 156 - "NativeServices"
Cohesion: 0.22
Nodes (4): IConflictServices, NativeServices, manager_, verifiedPid_

### Community 157 - "README.md"
Cohesion: 0.10
Nodes (12): OpenRGB controller-family worklist, HP Omen 30L migration: first independent-module increment, Integration and qualification, Pinned reference comparison, Selected protocol and scope, Entwurf: Kommentar für GitLab-Issues #4935 / #5429, Changes, Lighting Studio 0.1.0 (+4 more)

### Community 158 - "ServiceServer"
Cohesion: 0.17
Nodes (8): Service pipe access list - 2026-11-14 (Windows only), QLocalServer, QLocalSocket, QWinEventNotifier, ServiceServer, clients_, error_, owner_

### Community 159 - "ParameterSpinBox"
Cohesion: 0.20
Nodes (6): maximum, minimum, ParameterSpinBox, scale_, typed_, typedAt_

### Community 160 - "Hardware support strategy"
Cohesion: 0.33
Nodes (6): Evidence and qualification, Execution boundary, Hardware support strategy, Independent hardware modules, Installed manufacturer software and peripheral settings, Mandatory comparison of reference implementations

### Community 161 - "Uninstall-LightingStudio.ps1"
Cohesion: 0.36
Nodes (4): Get-JsonFiles(), Remove-Tree(), Remove-TreeItem(), Remove-UserData()

### Community 162 - "ChannelEditor"
Cohesion: 0.33
Nodes (4): TODO: shared LED colors are hard to find - 2026-11-17, ChannelEditor, fixedComponent_, initial_

### Community 163 - "impl_"
Cohesion: 0.29
Nodes (6): impl_, address, library, ready, sent, temporaryFailure

### Community 164 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 165 - "LightingCanvas.h"
Cohesion: 0.29
Nodes (7): Task 10: Qt tray app shell, QAction, QMenu, QPainter, QPushButton, QScrollBar, QTimer

### Community 167 - "Workspace navigation and catalog identity"
Cohesion: 0.67
Nodes (3): Addressable LEDs and NV5 MKII evidence, MSI board configuration versus spatial pixel frames, Workspace navigation and catalog identity

### Community 170 - "Artwork, continuous gestures and spatial controls (GS-06)"
Cohesion: 0.67
Nodes (3): Artwork, continuous gestures and spatial controls (GS-06), Image-editor crop and visible preview scope (2026-11-13), Startup compatibility hotfix (2026-10-30)

### Community 180 - "ComponentEditor"
Cohesion: 0.25
Nodes (4): ComponentEditor, category_, manufacturer_, nonPhysicalSlots_

### Community 181 - "SnapGrid"
Cohesion: 0.29
Nodes (3): SnapGrid, enabled, spacing

### Community 183 - "HidApiDevice"
Cohesion: 0.12
Nodes (8): Resolved: `IHidDevice` read capability added (API 3), HidApiDevice, handle_, initialized_, HidRuntime, ready, initializeHid(), HidBackendTest

### Community 190 - "EffectParameter"
Cohesion: 0.25
Nodes (6): EffectParameter, EffectLayer, id, label, schemaVersion, practicalEffectRange()

### Community 201 - "Hardware modules"
Cohesion: 0.07
Nodes (25): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, LED discovery (+17 more)

### Community 203 - "eventProperty"
Cohesion: 0.48
Nodes (3): eventProperty(), onUsbEvent(), pointerProperty()

### Community 215 - "RgbColor"
Cohesion: 0.12
Nodes (8): Responsibilities, HpOmenDriver, channels_, HpOmenModule, RgbColor, b, g, r

### Community 216 - "OutputOnlyHid"
Cohesion: 0.21
Nodes (5): HpOmenTest, identity(), OutputOnlyHid, featureWrites, profile()

### Community 218 - "Review Focus"
Cohesion: 0.29
Nodes (6): Editor Fixes and Menu Cleanup Implementation Plan, Global Constraints, Review Focus, Task 3: Portable start and its documentation, Task 5: Effect library with sorted categories and a clear add button, Task 6: Release R16

### Community 221 - "EffectRenderer.h"
Cohesion: 0.18
Nodes (9): EffectDefinition, description, id, kind, label, particleCount, patternSites, patternSpeed (+1 more)

### Community 222 - "Effect implementation checklist"
Cohesion: 0.20
Nodes (9): Additional native patterns, Additional native source batch, Capability gate and particle follow-up, Editing follow-up, Effect implementation checklist, First implementation batch, Second batch and color picker, Second batch and remaining candidates (+1 more)

### Community 223 - "ScanMailbox"
Cohesion: 0.33
Nodes (4): ScanMailbox, finished, mutex, owner

### Community 228 - "ConflictService"
Cohesion: 0.11
Nodes (13): ConflictService, acceptsStop, commandLine, name, ownProcess, pid, state, IConflictProcesses (+5 more)

## Knowledge Gaps
- **1163 isolated node(s):** `mode`, `speed`, `color`, `applied`, `channels_` (+1158 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1742 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **44 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `ServicePanel`, `PreviewDivider`, `test_editor.cpp`, `LightingWindow.cpp`, `LightingCanvas`, `state_`, `ServicePanel.cpp`, `EffectRenderer.cpp`, `test_output_ui.cpp`, `src/main.cpp`, `.edit`, `OutputPanel`, `Review Focus`, `Layer model proposal`, `HardwarePicker`, `LightingDocument`, `OutputPanel.cpp`, `authorizeHardwareOutput`, `main`?**
  _High betweenness centrality (0.129) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `IDriver`, `AuxiliaryTransport`, `MsiDriver`, `MsiColor`, `BuildReport0x51`, `.load`, `Recording`, `IDriverModule`, `Transport`, `OutputSession`, `LenovoMotherboardModule.cpp`, `PeripheralModule`, `ComponentEditor.cpp`, `RecordingTransport`, `HidApiDevice`, `.connect`, `SignalRGB repository research`, `Transport`, `FakeHidDevice`, `Hardware modules`, `RgbColor`, `.pushColors`, `AuxiliaryDriver`, `array`, `NativeEffect`, `VirtualModule`, `AsrockPolychromeDriver`, `Record`?**
  _High betweenness centrality (0.103) - this node is a cross-community bridge._
- **Why does `LightingCanvas` connect `LightingCanvas` to `Tool`, `test_editor.cpp`, `DeviceConflictMonitor`, `IDriverModule`, `ServicePanel.cpp`, `HardwareArtwork.cpp`, `LightingWindow`, `OutputSession`, `LightingCanvas.h`, `.workingState`, `ComponentEditor.cpp`, `LightingCanvas.cpp`, `SaturationValueField`, `OutputPanel`, `.cancelGesture`, `EffectRenderer.h`, `LightingDocument`, `StudioWidgets.cpp`, `.paintComponents`, `.mousePressEvent`?**
  _High betweenness centrality (0.077) - this node is a cross-community bridge._
- **Are the 90 inferred relationships involving `LightingWindow` (e.g. with `Problem confirmed in the current code` and `Editor Fixes and Menu Cleanup Implementation Plan`) actually correct?**
  _`LightingWindow` has 90 INFERRED edges - model-reasoned connections that need verification._
- **Are the 8 inferred relationships involving `catalog()` (e.g. with `.componentLedOffsets()` and `.componentName()`) actually correct?**
  _`catalog()` has 8 INFERRED edges - model-reasoned connections that need verification._
- **What connects `mode`, `speed`, `color` to the rest of the system?**
  _1163 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `IDriver` be split into smaller, more focused modules?**
  _Cohesion score 0.07526881720430108 - nodes in this community are weakly interconnected._