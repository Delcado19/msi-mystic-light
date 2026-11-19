# Graph Report - msi-mystic-light  (2026-11-19)

## Corpus Check
- 205 files · ~1,710,000 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 21 file(s) not represented in the graph (top: .csv 6, .cmd 3, .bin 3)

## Summary
- 3619 nodes · 8244 edges · 190 communities (140 shown, 50 thin omitted)
- Extraction: 78% EXTRACTED · 22% INFERRED · 0% AMBIGUOUS · INFERRED: 1775 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `e2682278`
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
- .resize
- RgbColor
- MsiGen1AdvArea
- Development Tools
- LightingWindow.cpp
- .fromJson
- test_editor.cpp
- MsiColor
- BuildReport0x51
- LenovoMotherboardMode
- PawnIoRam.cpp
- .load
- Recording
- LightingWindow::LightingWindow
- CONTINUE.md history archive
- LIGHTING_EDITOR.md
- IDriverModule
- LightingCanvas
- .edit
- Transport
- Mystic Light — Overall Architecture
- Reference repository reuse review
- Lighting Studio
- ServicePanel.cpp
- AsrockPolychromeMode
- .denseHardwareArtwork
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
- OutputPanel
- Lighting Studio — Design Doc (v2)
- ConflictDialog
- GeometryView
- ConflictProcess
- Record
- MsiZoneSettings
- Lighting Studio: whole-interface and unified-editor review
- DriverInstance
- MsiLightingMode
- previewOutputReports
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
- Editor interaction implementation
- ChannelAssignment
- ConflictProcesses.cpp
- PawnIO integration and service feasibility
- .start
- .pushColors
- .setNativeMode_rejectsInvalidEffectsAndClosedDevice
- Layer visibility, brush size and Delete
- AuxiliaryDriver
- AeroBackend
- .cancelGesture
- FakeProcesses
- Layer model proposal
- PowerResumeFilter
- HardwarePicker
- NativeEffect
- LightingDocument
- Lighting Studio GUI and interaction audit
- Test review: recent lighting changes (2026-10-24)
- VirtualModule
- Gradient travel and blend verification (2026-10-24)
- Lightning and Fireworks visual review (2026-10-29)
- ServiceProtocol.cpp
- main
- PawnIO AMD SMBus module
- design/README.md
- service/main.cpp
- ServiceServer.cpp
- AsrockPolychromeDriver
- LayerTree
- StudioWidgets.cpp
- .addHeaderComponents
- .reports
- studioThemeColor
- GS-06-R2 editor, background and mask validation
- Studio interaction and motion review
- .componentAuthoring
- hw_inventory_linux.sh
- DeviceConflicts.cpp
- .mousePressEvent
- ComponentDefinition
- closeWindow
- Tool
- .create
- Connection
- Windows service operation
- CONTINUE.md
- Lighting Studio editor UX review
- closeWindow
- State
- branding/README.md
- JARGB2 physical response tests - 2026-11-13 / 2026-11-14
- DeviceConflictMonitor
- Hardware migration inventory and priorities
- stdexcept
- README.md
- ParameterSpinBox
- Uninstall-LightingStudio.ps1
- LightingCanvas.h
- PeripheralModule
- Workspace navigation and catalog identity
- Output session backend
- Artwork, continuous gestures and spatial controls (GS-06)
- ModulesTest
- Distinct source capabilities and particle authoring
- Project files and boundaries
- ChannelEditor
- QPainter
- ScCloser
- HidApiDevice
- Effect editor interaction research
- EffectParameter
- Hardware modules
- eventProperty
- Reliability and layer-design work (2026-11-11)
- `Mystic Light Online Data.dat` — decryption and content
- HpOmenDriver
- qttest
- QString
- EffectRenderer.h
- Effect implementation checklist
- ScanMailbox
- SignalForge editor reference
- LayerBlend
- .paint
- IConflictServices
- Lighting Studio 0.1.0
- QJsonObject
- Third-party runtime components
- ScCloser

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 228 edges
2. `LightingCanvas` - 198 edges
3. `EditorTest` - 148 edges
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
- `Output scheduler` --references--> `OutputScheduler`  [INFERRED]
  docs/OUTPUT_SESSION.md → src/output/OutputScheduler.h

## Import Cycles
- None detected.

## Communities (190 total, 50 thin omitted)

### Community 0 - "IDriver"
Cohesion: 0.08
Nodes (21): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+13 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.11
Nodes (6): AuxiliaryTransport, gpu_, ram_, ready_, openAuxiliaryTransport(), scanAuxiliaryHardware()

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

### Community 7 - ".resize"
Cohesion: 0.15
Nodes (10): NumericFieldStyle, QAbstractSpinBox, sliderField(), sliderPosition(), sliderRange(), SliderWheelStep, remainder_, value_ (+2 more)

### Community 8 - "RgbColor"
Cohesion: 0.13
Nodes (11): Responsibilities, MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_, RgbColor (+3 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "LightingWindow.cpp"
Cohesion: 0.06
Nodes (3): colorIcon(), PreviewDivider, PreviewSplitter

### Community 12 - ".fromJson"
Cohesion: 0.08
Nodes (14): chainOffsets(), chainSharing(), channelMapping(), integer(), requiredText(), SharedRun, firstLed, lastLed (+6 more)

### Community 13 - "test_editor.cpp"
Cohesion: 0.06
Nodes (10): Task 1: Gradient stop colors survive Apply, document_, applyStudioStyle(), studioToolIcon(), addEffect(), artifactDirectory(), catalog(), channelProfile() (+2 more)

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
Cohesion: 0.11
Nodes (16): call(), ~Impl(), libraryPath(), locked(), onPrimary(), PawnIoRam, impl_, address (+8 more)

### Community 18 - ".load"
Cohesion: 0.24
Nodes (3): fail(), ModuleRegistry, entries_

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "LightingWindow::LightingWindow"
Cohesion: 0.09
Nodes (9): isShape(), button(), LightingWindow::LightingWindow(), scan_, practicalEffectRange(), QTreeWidgetItem, setEffectControlRange(), studioLogo() (+1 more)

### Community 21 - "CONTINUE.md history archive"
Cohesion: 0.07
Nodes (27): Active-layer picking, canvas labels and transport controls (2026-10-28), Authoring controls, pan and two patterns (2026-10-28), Components fullscreen and edit-mode visibility fix (2026-10-28), CONTINUE.md history archive, Dense strip SVG visibility regression (2026-10-26), Editor interaction and channel preview follow-up (2026-10-27), Editor UX, Pan crash and four sources (2026-10-28), Fixed optional component captions (2026-10-26) (+19 more)

### Community 22 - "LIGHTING_EDITOR.md"
Cohesion: 0.20
Nodes (5): Project compatibility, Rendering, color and numeric behavior, Unified Editor implementation, Verification, Workspace and interaction

### Community 23 - "IDriverModule"
Cohesion: 0.06
Nodes (6): AuxiliaryModule, MsiModule, LightingState, IDriverModule, IncompatibleModule, MissingInterface

### Community 24 - "LightingCanvas"
Cohesion: 0.02
Nodes (76): LightingCanvas, activeLayer_, additiveMarquee_, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, componentSeconds_ (+68 more)

### Community 25 - ".edit"
Cohesion: 0.12
Nodes (6): canCreateEffect(), renderEffect(), validateEffect(), state_, validate(), layer()

### Community 26 - "Transport"
Cohesion: 0.13
Nodes (7): Recording, opens, present, writes, Transport, opened, record

### Community 27 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "Lighting Studio"
Cohesion: 0.25
Nodes (8): Building from source, Documentation, Features, Getting started, License, Lighting Studio, Related repos, Supported hardware

### Community 30 - "ServicePanel.cpp"
Cohesion: 0.13
Nodes (9): command(), host(), ServiceConflictProcesses, elevate_, rpc_, ServiceConflictProcesses::ServiceConflictProcesses(), serviceInventory(), ServicePanel::ServicePanel() (+1 more)

### Community 31 - "AsrockPolychromeMode"
Cohesion: 0.13
Nodes (15): AsrockPolychromeMode, Breathing, Cram, Music, Neon, Off, Rainbow, Random (+7 more)

### Community 32 - ".denseHardwareArtwork"
Cohesion: 0.24
Nodes (5): loadComponentCatalog(), loadComponentDefinition(), hardwareCategory(), hardwareManufacturer(), hardwareName()

### Community 33 - "EffectLayer"
Cohesion: 0.03
Nodes (59): EffectLayer, allChannels, breathe, center, channelTargets, color, colorCycle, colorCyclePositions (+51 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (71): ColorEditor, ColorListEditor, LightingWindow, activeLayer_, adjustingPanels_, advanced_, allChannels_, animation_ (+63 more)

### Community 35 - "OutputSession"
Cohesion: 0.12
Nodes (8): nativeKey(), OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_

### Community 36 - "EffectRenderer.cpp"
Cohesion: 0.19
Nodes (15): effectTransform(), interpolate(), isGradient(), isParticle(), isPattern(), noise2d(), periodic(), polygon() (+7 more)

### Community 37 - "Lighting editor"
Cohesion: 0.09
Nodes (23): Authoring controls follow-up, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Integer controls, live color and identity (2026-10-26) (+15 more)

### Community 38 - "LenovoMotherboardModule.cpp"
Cohesion: 0.14
Nodes (6): LenovoMotherboardDriver, channels_, LenovoMotherboardModule, mapBrightness(), mapSpeed(), zoneFromAddress()

### Community 39 - "ServiceRuntime"
Cohesion: 0.04
Nodes (43): Retired, endpoint, scheduler, ScanResult, blocked, error, guarded, inventory (+35 more)

### Community 41 - "AsrockPolychromeZone"
Cohesion: 0.13
Nodes (10): AsrockPolychromeZone, Argb1, Argb2, Audio, IoCover, Pcb, Pch, Rgb1 (+2 more)

### Community 42 - "ComponentEditor.cpp"
Cohesion: 0.07
Nodes (4): CoordinateDelegate, QPainter, QToolButton, StudioSectionButton

### Community 43 - "IHidDevice"
Cohesion: 0.07
Nodes (16): 162-byte MSI family — corroborated, not conflicting, ASRock Polychrome USB (`26CE:01A2`) — mostly confirmed, one open discrepancy, ASRock Polychrome USB — implemented (2026-10-29, same session), ENE SMBus (RAM) — register map confirmed 3-way, detection method rejected, Lenovo motherboard — implemented (2026-10-29, same session), Not yet cross-checked this pass, Open questions, Our board (`0DB0:0076`) — triple-confirmed, including the shared bug (+8 more)

### Community 44 - "OutputPanel"
Cohesion: 0.05
Nodes (34): previewFrame_, channelKey(), OutputPanel, authorize_, authorizing_, automatic_, channelDetails_, configured_ (+26 more)

### Community 45 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 46 - "ConflictDialog"
Cohesion: 0.10
Nodes (13): ConflictDialog, adminServicePid_, adminStop_, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_ (+5 more)

### Community 47 - "GeometryView"
Cohesion: 0.13
Nodes (6): GeometryView, artwork, commit, moved, pressPosition, scale

### Community 48 - "ConflictProcess"
Cohesion: 0.10
Nodes (12): ConflictProcess, elevatedStop, executable, manualStop, path, pid, provenHolder, reason (+4 more)

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

### Community 54 - "previewOutputReports"
Cohesion: 0.14
Nodes (6): PairedTransport, control, leds, previewOutputReports(), RecordingTransport, reports_

### Community 55 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.27
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.20
Nodes (6): validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 58 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "EneRamProtocol"
Cohesion: 0.23
Nodes (4): EneRamProtocol, address_, transfer_, AuxiliaryTest

### Community 64 - "inventory_hardware.py"
Cohesion: 0.05
Nodes (13): arguments(), build(), git(), integer(), js_return(), priority(), registrations(), snapshot() (+5 more)

### Community 65 - "ServiceRuntime.cpp"
Cohesion: 0.13
Nodes (8): Hung device open and project-only conflict check - 2026-11-17, Task 2: Project edits update running hardware without reopening it, prepareOutput(), sameOutputScope(), Scan, key(), ServiceRuntime::ServiceRuntime(), writeFile()

### Community 67 - "OutputScheduler"
Cohesion: 0.13
Nodes (7): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "FakeHidDevice"
Cohesion: 0.15
Nodes (11): Completed: HID transport (2026-10-23), FakeHidDevice, featureReportResponse, getFeatureReportCalls, inputReportResponse, opened_, openSucceeds, readInputReportCalls (+3 more)

### Community 70 - "OutputScheduler.cpp"
Cohesion: 0.11
Nodes (7): DeviceChanges, generation, mutex, wake, onHidInterface(), OutputScheduler::OutputScheduler(), watchDevices()

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "Status"
Cohesion: 0.29
Nodes (4): Status, error, generation, state

### Community 73 - "src/main.cpp"
Cohesion: 0.11
Nodes (5): AppSession, AppSession::AppSession(), recoveryFile_, timer_, applicationIcon()

### Community 74 - "ColorListEditor"
Cohesion: 0.06
Nodes (23): Component catalog schema 1, Task 5: `BreathingEffect`, Task 7: `CometEffect` and `MarqueeEffect`, ColorEditor, hex_, picker_, public, ColorListEditor (+15 more)

### Community 77 - "OutputFrame"
Cohesion: 0.13
Nodes (11): buildFrames(), ChannelOutput, channelId, colors, native, software, OutputFrame, channels (+3 more)

### Community 78 - "ComponentPlacement"
Cohesion: 0.18
Nodes (6): ComponentPlacement, componentId, id, locked, origin, rotation

### Community 79 - "test_service.cpp"
Cohesion: 0.14
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

### Community 83 - "Editor interaction implementation"
Cohesion: 0.22
Nodes (6): Editor interaction implementation, Identity and color sampling, Persistence and limits, Pointer behavior, Rendering and output status, Result

### Community 84 - "ChannelAssignment"
Cohesion: 0.12
Nodes (13): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+5 more)

### Community 85 - "ConflictProcesses.cpp"
Cohesion: 0.15
Nodes (21): belongsToMysticLightService(), conflictReason(), creationTime(), isSignalRgbService(), matchesProcessIdentity(), mysticLightServiceStopped(), NativeServices, manager_ (+13 more)

### Community 86 - "PawnIO integration and service feasibility"
Cohesion: 0.22
Nodes (9): Access control and process boundary, Conclusion, Coverage and next gate, Exact evidence, Installation and licensing boundary, PawnIO integration and service feasibility, Performance and recovery, Primary sources (+1 more)

### Community 88 - ".pushColors"
Cohesion: 0.09
Nodes (13): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport() (+5 more)

### Community 89 - ".setNativeMode_rejectsInvalidEffectsAndClosedDevice"
Cohesion: 0.33
Nodes (3): identity(), LenovoMotherboardTest, profile()

### Community 90 - "Layer visibility, brush size and Delete"
Cohesion: 0.22
Nodes (9): Artwork alignment and custom device geometry, Detailed hardware artwork, Fixed-device settings versus attachable headers, Gaming workspace and responsive panels, Layer visibility, brush size and Delete, Mask playback, navigation and cosmetic hardware variants, Model labels, fullsize keyboards and zero-angle defaults, Scrollbar contrast and Effects preview divider (+1 more)

### Community 92 - "AeroBackend"
Cohesion: 0.07
Nodes (25): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+17 more)

### Community 93 - ".cancelGesture"
Cohesion: 0.11
Nodes (3): MaskTool, Brush, Gradient

### Community 94 - "FakeProcesses"
Cohesion: 0.05
Nodes (39): ConflictService, acceptsStop, commandLine, name, ownProcess, pid, state, State (+31 more)

### Community 95 - "Layer model proposal"
Cohesion: 0.20
Nodes (10): Acceptance matrix, Creation and selection contract, Delivery in reviewable increments, Explain the blends with a live result, Explicitly outside the first implementation, Feasibility evidence and resolved review points, Layer model proposal, Problem confirmed in the current code (+2 more)

### Community 97 - "HardwarePicker"
Cohesion: 0.11
Nodes (14): TODO: custom hardware lives only inside the project - 2026-11-17, HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity (+6 more)

### Community 99 - "NativeEffect"
Cohesion: 0.11
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 100 - "LightingDocument"
Cohesion: 0.08
Nodes (15): chainStarts(), LightingDocument, colorPreviewBefore_, componentNames_, frameCache_, frameRevision_, frameSeconds_, identify_ (+7 more)

### Community 102 - "Lighting Studio GUI and interaction audit"
Cohesion: 0.18
Nodes (10): Color picker, Consolidation, Evidence, Findings, ordered by impact, Identity and direction, Implementation follow-up, Implementation order and acceptance, Integer scales (+2 more)

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
Cohesion: 0.25
Nodes (12): authorizedClient(), currentUserSid(), decode(), encode(), installedHost(), isSystem(), pipeName(), processSid() (+4 more)

### Community 110 - "main"
Cohesion: 0.13
Nodes (10): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), ProfileCandidate, device, profileId (+2 more)

### Community 112 - "design/README.md"
Cohesion: 0.29
Nodes (4): Evidence, Ice workspace - GS-06-R2, Design and collaboration, Editor interaction audit and resolution - 2026-11-13

### Community 113 - "service/main.cpp"
Cohesion: 0.12
Nodes (9): control(), knownFolder(), logWarning(), main(), readJson(), report(), run(), serviceMain() (+1 more)

### Community 114 - "ServiceServer.cpp"
Cohesion: 0.08
Nodes (11): Service pipe access list - 2026-11-14 (Windows only), QLocalServer, QLocalSocket, QWinEventNotifier, ServiceServer, clients_, ServiceServer::connected(), error_ (+3 more)

### Community 115 - "AsrockPolychromeDriver"
Cohesion: 0.09
Nodes (10): AsrockPolychromeDriver, channels_, state_, AsrockPolychromeModule, mapSpeed(), ZoneState, applied, color (+2 more)

### Community 117 - "StudioWidgets.cpp"
Cohesion: 0.07
Nodes (12): openStudioColorPicker(), ComponentEditor::ComponentEditor(), ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), disclosure(), installParameterTooltips(), QToolBar, StudioBanner (+4 more)

### Community 121 - "GS-06-R2 editor, background and mask validation"
Cohesion: 0.50
Nodes (4): Behavioral evidence, GS-06-R2 editor, background and mask validation, Investigation and limits, Resulting interface

### Community 122 - "Studio interaction and motion review"
Cohesion: 0.14
Nodes (14): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Endpoint list correction and restored backends (2026-10-24), Fixed optional component captions (2026-10-26), Gaming lighting workspace review implementation (2026-10-26), Gradient/output feedback correction (2026-10-24) (+6 more)

### Community 125 - "DeviceConflicts.cpp"
Cohesion: 0.09
Nodes (38): Responsive service and installed conflict dialog - 2026-11-16, alive(), Capture, devices, instances, writes, captureUsbWrites(), creationTime() (+30 more)

### Community 129 - "ComponentDefinition"
Cohesion: 0.09
Nodes (19): ComponentDefinition, category, id, manufacturer, mapping, name, nonPhysicalSlots, positions (+11 more)

### Community 131 - "Tool"
Cohesion: 0.18
Nodes (10): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pan (+2 more)

### Community 133 - ".create"
Cohesion: 0.16
Nodes (3): AsrockPolychromeTest, identity(), profile()

### Community 134 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 135 - "Windows service operation"
Cohesion: 0.22
Nodes (7): Diagnostics and removal, Install and update, Playback and editing, Removal, Security boundary, Verification boundaries, Windows service operation

### Community 137 - "CONTINUE.md"
Cohesion: 0.11
Nodes (17): Branching and releases - 2026-11-15, CodeRabbit round 2 fixes, R12 - 2026-11-17, Editor fixes and menu cleanup, R16 - 2026-11-17, Editor fixes and small audit fixes - 2026-11-14 (GS-06-R7, build/r4), Event-driven presence check - 2026-11-18, GS-06-R2 UI correction - 2026-11-13, GS-06-R9 - 2026-11-15, Header LED count - 2026-11-15 (+9 more)

### Community 138 - "Lighting Studio editor UX review"
Cohesion: 0.15
Nodes (13): Additional procedural sources, Captures and measured checks, Crash evidence and validation boundaries, Creative-software research, Direction and scope, Findings and implementation, Gaming-Pro-Studio refinement, GS-06 finish (+5 more)

### Community 140 - "State"
Cohesion: 0.50
Nodes (4): State, Connected, Disconnected, Faulted

### Community 143 - "JARGB2 physical response tests - 2026-11-13 / 2026-11-14"
Cohesion: 0.22
Nodes (8): Approved tests and observations, Follow-up observations - 2026-11-14, JARGB2 physical response tests - 2026-11-13 / 2026-11-14, Pattern 6 - sequential fan-prefix qualification, Sleep/resume event, Software implications, Software status - 2026-11-14, Supported interpretation

### Community 146 - "DeviceConflictMonitor"
Cohesion: 0.06
Nodes (27): DetectedConflicts, list_, owner_, DeviceConflictMonitor, busy_, capture_, lastBusy_, lastError_ (+19 more)

### Community 150 - "Hardware migration inventory and priorities"
Cohesion: 0.12
Nodes (13): OpenRGB controller-family worklist, Browse the full list, Hardware migration inventory and priorities, Prioritized migration queue, Reproduction and next decisions, Scale and counting units, Source provenance and limitations, Evidence and qualification (+5 more)

### Community 151 - "stdexcept"
Cohesion: 0.09
Nodes (14): authorizeHardwareOutput(), loadModules(), OutputProof, controlIdentity, identity, initial, profile, reports (+6 more)

### Community 157 - "README.md"
Cohesion: 0.20
Nodes (5): HP Omen 30L migration: first independent-module increment, Integration and qualification, Pinned reference comparison, Selected protocol and scope, Entwurf: Kommentar für GitLab-Issues #4935 / #5429

### Community 159 - "ParameterSpinBox"
Cohesion: 0.39
Nodes (4): ParameterSpinBox, scale_, typed_, typedAt_

### Community 165 - "LightingCanvas.h"
Cohesion: 0.29
Nodes (7): Task 10: Qt tray app shell, QAction, QMenu, QPainter, QPushButton, QScrollBar, QTimer

### Community 167 - "Workspace navigation and catalog identity"
Cohesion: 0.67
Nodes (3): Addressable LEDs and NV5 MKII evidence, MSI board configuration versus spatial pixel frames, Workspace navigation and catalog identity

### Community 169 - "Output session backend"
Cohesion: 0.33
Nodes (6): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification

### Community 170 - "Artwork, continuous gestures and spatial controls (GS-06)"
Cohesion: 0.67
Nodes (3): Artwork, continuous gestures and spatial controls (GS-06), Image-editor crop and visible preview scope (2026-11-13), Startup compatibility hotfix (2026-10-30)

### Community 180 - "ChannelEditor"
Cohesion: 0.13
Nodes (8): TODO: shared LED colors are hard to find - 2026-11-17, ChannelEditor, fixedComponent_, initial_, ComponentEditor, category_, manufacturer_, nonPhysicalSlots_

### Community 181 - "QPainter"
Cohesion: 0.22
Nodes (3): SnapGrid, enabled, spacing

### Community 183 - "HidApiDevice"
Cohesion: 0.06
Nodes (26): Resolved: `IHidDevice` read capability added (API 3), hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer (+18 more)

### Community 184 - "Effect editor interaction research"
Cohesion: 0.25
Nodes (8): Acceptance and implementation gates, Effect editor interaction research, Findings in current source, Interaction contract, Interface and identity, Recommended content model, Reference patterns, Scope and evidence

### Community 190 - "EffectParameter"
Cohesion: 0.22
Nodes (7): EffectParameter, EffectLayer, id, label, maximum, minimum, schemaVersion

### Community 201 - "Hardware modules"
Cohesion: 0.07
Nodes (25): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, LED discovery (+17 more)

### Community 203 - "eventProperty"
Cohesion: 0.48
Nodes (3): eventProperty(), onUsbEvent(), pointerProperty()

### Community 210 - "Reliability and layer-design work (2026-11-11)"
Cohesion: 0.33
Nodes (6): Collaboration contract, Current status, Decisions and validation boundaries, Final software validation, Package and scope limits, Reliability and layer-design work (2026-11-11)

### Community 213 - "`Mystic Light Online Data.dat` — decryption and content"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 215 - "HpOmenDriver"
Cohesion: 0.16
Nodes (3): HpOmenDriver, channels_, HpOmenModule

### Community 216 - "qttest"
Cohesion: 0.16
Nodes (5): HpOmenTest, identity(), OutputOnlyHid, featureWrites, profile()

### Community 218 - "QString"
Cohesion: 0.16
Nodes (8): Editor Fixes and Menu Cleanup Implementation Plan, Global Constraints, Review Focus, Task 3: Portable start and its documentation, Task 4: Menus are available where they act, once, Task 5: Effect library with sorted categories and a clear add button, Task 6: Release R16, outputTrafficLight()

### Community 221 - "EffectRenderer.h"
Cohesion: 0.18
Nodes (9): EffectDefinition, description, id, kind, label, particleCount, patternSites, patternSpeed (+1 more)

### Community 222 - "Effect implementation checklist"
Cohesion: 0.22
Nodes (9): Additional native patterns, Additional native source batch, Capability gate and particle follow-up, Editing follow-up, Effect implementation checklist, First implementation batch, Second batch and color picker, Second batch and remaining candidates (+1 more)

### Community 223 - "ScanMailbox"
Cohesion: 0.33
Nodes (4): ScanMailbox, finished, mutex, owner

### Community 224 - "SignalForge editor reference"
Cohesion: 0.29
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 225 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

### Community 229 - "Lighting Studio 0.1.0"
Cohesion: 0.50
Nodes (3): Changes, Lighting Studio 0.1.0, Requirements and limitations

### Community 230 - "QJsonObject"
Cohesion: 0.18
Nodes (7): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text(), QLocalSocket, QJsonObject

### Community 232 - "Third-party runtime components"
Cohesion: 0.50
Nodes (3): Hardware artwork, HP Omen 30L protocol module, Third-party runtime components

## Knowledge Gaps
- **1163 isolated node(s):** `mode`, `speed`, `color`, `applied`, `channels_` (+1158 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1742 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **50 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `.denseHardwareArtwork`, `LightingDocument`, `ServicePanel`, `.activate`, `src/main.cpp`, `LightingWindow.cpp`, `OutputPanel`, `test_editor.cpp`, `main`, `LightingWindow::LightingWindow`, `.addHeaderComponents`, `LightingCanvas`, `.edit`, `QString`, `ServicePanel.cpp`, `Layer model proposal`?**
  _High betweenness centrality (0.145) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `IDriver`, `AuxiliaryTransport`, `.create`, `RgbColor`, `MsiColor`, `BuildReport0x51`, `Recording`, `IDriverModule`, `Transport`, `OutputSession`, `LenovoMotherboardModule.cpp`, `PeripheralModule`, `ComponentEditor.cpp`, `OutputPanel`, `previewOutputReports`, `HidApiDevice`, `.connect`, `SignalRGB repository research`, `Transport`, `FakeHidDevice`, `Hardware modules`, `HpOmenDriver`, `.pushColors`, `AuxiliaryDriver`, `array`, `NativeEffect`, `VirtualModule`, `AsrockPolychromeDriver`?**
  _High betweenness centrality (0.115) - this node is a cross-community bridge._
- **Why does `LightingCanvas` connect `LightingCanvas` to `Tool`, `.fromJson`, `test_editor.cpp`, `DeviceConflictMonitor`, `IDriverModule`, `ServicePanel.cpp`, `LightingWindow`, `EffectRenderer.cpp`, `LightingCanvas.h`, `ComponentEditor.cpp`, `OutputPanel`, `LightingCanvas.cpp`, `OutputFrame`, `SaturationValueField`, `.cancelGesture`, `EffectRenderer.h`, `LightingDocument`, `.paintComponents`, `.mousePressEvent`?**
  _High betweenness centrality (0.088) - this node is a cross-community bridge._
- **Are the 90 inferred relationships involving `LightingWindow` (e.g. with `Problem confirmed in the current code` and `Editor Fixes and Menu Cleanup Implementation Plan`) actually correct?**
  _`LightingWindow` has 90 INFERRED edges - model-reasoned connections that need verification._
- **Are the 8 inferred relationships involving `catalog()` (e.g. with `.componentLedOffsets()` and `.componentName()`) actually correct?**
  _`catalog()` has 8 INFERRED edges - model-reasoned connections that need verification._
- **What connects `mode`, `speed`, `color` to the rest of the system?**
  _1163 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `IDriver` be split into smaller, more focused modules?**
  _Cohesion score 0.07526881720430108 - nodes in this community are weakly interconnected._