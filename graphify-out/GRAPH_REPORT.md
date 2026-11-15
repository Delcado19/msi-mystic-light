# Graph Report - msi-mystic-light  (2026-11-15)

## Corpus Check
- 203 files · ~1,695,649 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 18 file(s) not represented in the graph (top: .csv 6, .bin 3, (none) 2)

## Summary
- 3349 nodes · 7506 edges · 186 communities (137 shown, 49 thin omitted)
- Extraction: 78% EXTRACTED · 22% INFERRED · 0% AMBIGUOUS · INFERRED: 1628 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `57ee1cba`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- IDriver
- AuxiliaryTransport
- Implementation status
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- VirtualModule
- HpOmenDriver
- RgbColor
- MsiGen1AdvArea
- Development Tools
- LightingWindow.cpp
- MsiColor
- test_editor.cpp
- AeroBackend
- StudioWidgets.h
- HardwareArtwork.cpp
- PawnIoRam.cpp
- .load
- Recording
- AsrockPolychromeMode
- CONTINUE.md history archive
- LIGHTING_EDITOR.md
- IDriverModule
- LightingCanvas
- .render
- Transport
- Mystic Light — Overall Architecture
- Reference repository reuse review
- MSI Mystic Light – Lighting Studio
- ServicePanel
- BuildReport0x51
- OutputPanel
- EffectLayer
- LightingWindow
- OutputSession
- EffectRenderer.cpp
- Lighting editor
- LenovoMotherboardModule.cpp
- ServiceRuntime
- OpenRGB SDK client — cross-platform (esp. Linux) hardware access
- ComponentEditor.cpp
- IHidDevice
- test_output_ui.cpp
- Lighting Studio — Design Doc (v2)
- ConflictDialog
- GeometryView
- test_service.cpp
- Record
- MsiZoneSettings
- Lighting Studio: whole-interface and unified-editor review
- DriverInstance
- MsiLightingMode
- RecordingTransport
- ComponentPlacement
- test_scheduler.cpp
- .connect
- .edit
- Transport
- EneRamProtocol
- .paintEvent
- inventory_hardware.py
- Hardware modules
- StartupTest
- OutputScheduler
- FakeHidDevice
- .start
- Windows RGB access investigation
- Status
- src/main.cpp
- ColorListEditor
- authorizeHardwareOutput
- Effect implementation checklist
- PaletteStrip
- LightingWindow::LightingWindow
- EffectKind
- SaturationValueField
- StudioWidgets.cpp
- ChannelAssignment
- ConflictProcesses.cpp
- ModulesTest
- HidApiDevice
- .pushColors
- SignalRGB repository research
- Layer visibility, brush size and Delete
- test_hp_omen.cpp
- .cancelGesture
- FakeProcesses
- Layer model proposal
- Output session backend
- HardwarePicker
- NativeEffect
- LightingDocument
- Lighting Studio GUI and interaction audit
- scanAuxiliaryHardware
- Test review: recent lighting changes (2026-10-24)
- .fromJson
- Gradient travel and blend verification (2026-10-24)
- Lightning and Fireworks visual review (2026-10-29)
- OutputPanel.cpp
- ProfileCandidate
- PawnIO AMD SMBus module
- AuxiliaryDriver
- service/main.cpp
- .mousePressEvent
- AsrockPolychromeDriver
- LenovoMotherboardMode
- ConflictService
- QJsonObject
- I2cInfo
- Record
- PreviewDivider
- Studio interaction and motion review
- DocumentEdit
- hw_inventory_linux.sh
- EffectRenderer.h
- QWidget
- LayerTree
- test_hid_backend.cpp
- closeWindow
- Tool
- ModuleRegistry
- Connection
- PawnIO integration and service feasibility
- Reliability and layer-design work (2026-11-11)
- Lighting Studio editor UX review
- HidDescriptor
- QPainter
- branding/README.md
- JARGB2 physical response tests - 2026-11-13 / 2026-11-14
- ConflictProcess
- Hardware migration inventory and priorities
- OutputFrame
- AsrockPolychromeZone
- button
- QPushButton
- .resize
- README.md
- stdexcept
- ParameterSpinBox
- EffectDefinition
- impl_
- Hardware support strategy
- ComponentDefinition
- LightingCanvas.h
- PeripheralModule
- PowerResumeFilter
- ServiceServer.cpp
- Artwork, continuous gestures and spatial controls (GS-06)
- Unified Editor implementation
- OutputConnection
- Three-way cross-check: SignalRGB plugin tree added as a third source
- colorIcon
- Workspace navigation and catalog identity
- Distinct source capabilities and particle authoring
- Project files and boundaries
- LightingState
- EffectParameter
- LayerBlend
- SignalForge editor reference
- ScCloser
- State
- HidBackendTest
- HidRuntime

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 223 edges
2. `LightingCanvas` - 194 edges
3. `EditorTest` - 144 edges
4. `catalog()` - 94 edges
5. `document_` - 81 edges
6. `LightingDocument` - 78 edges
7. `EffectLayer` - 71 edges
8. `Implementation status` - 66 edges
9. `IHidDevice` - 59 edges
10. `EffectKind` - 53 edges

## Surprising Connections (you probably didn't know these)
- `Project compatibility` --references--> `locked()`  [INFERRED]
  docs/design/UNIFIED_EDITOR_IMPLEMENTATION_2026-10-27.md → src/backends/PawnIoRam.cpp
- `Problem confirmed in the current code` --references--> `LightingWindow`  [INFERRED]
  docs/LAYER_MODEL_PLAN.md → src/editor/LightingWindow.h
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Output scheduler` --references--> `OutputScheduler`  [INFERRED]
  docs/OUTPUT_SESSION.md → src/output/OutputScheduler.h
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp

## Import Cycles
- None detected.

## Communities (186 total, 49 thin omitted)

### Community 0 - "IDriver"
Cohesion: 0.11
Nodes (17): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+9 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.16
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

### Community 7 - "HpOmenDriver"
Cohesion: 0.16
Nodes (3): HpOmenDriver, channels_, HpOmenModule

### Community 8 - "RgbColor"
Cohesion: 0.13
Nodes (11): Responsibilities, MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_, RgbColor (+3 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 12 - "MsiColor"
Cohesion: 0.12
Nodes (18): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness, Task 3: HID transport interface and hidapi backend (+10 more)

### Community 13 - "test_editor.cpp"
Cohesion: 0.07
Nodes (6): document_, addEffect(), artifactDirectory(), catalog(), EditorTest, QTemporaryDir

### Community 14 - "AeroBackend"
Cohesion: 0.12
Nodes (14): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+6 more)

### Community 15 - "StudioWidgets.h"
Cohesion: 0.11
Nodes (12): Task 5: `BreathingEffect`, Task 7: `CometEffect` and `MarqueeEffect`, ColorEditor, hex_, picker_, public, QDoubleSpinBox, QLabel (+4 more)

### Community 16 - "HardwareArtwork.cpp"
Cohesion: 0.18
Nodes (7): drawHardwareArtwork(), drawHardwareComponentArtwork(), hardwareArtworkCategory(), hardwareBodyBounds(), hardwarePreviewLedRadius(), initializeHardwareArtwork(), productIcon()

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.14
Nodes (10): call(), ~Impl(), libraryPath(), locked(), onPrimary(), PawnIoRam, error, pawnIoRamDiscoveryError() (+2 more)

### Community 18 - ".load"
Cohesion: 0.12
Nodes (6): AsrockPolychromeTest, identity(), profile(), identity(), LenovoMotherboardTest, profile()

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "AsrockPolychromeMode"
Cohesion: 0.13
Nodes (15): AsrockPolychromeMode, Breathing, Cram, Music, Neon, Off, Rainbow, Random (+7 more)

### Community 21 - "CONTINUE.md history archive"
Cohesion: 0.07
Nodes (27): Active-layer picking, canvas labels and transport controls (2026-10-28), Authoring controls, pan and two patterns (2026-10-28), Components fullscreen and edit-mode visibility fix (2026-10-28), CONTINUE.md history archive, Dense strip SVG visibility regression (2026-10-26), Editor interaction and channel preview follow-up (2026-10-27), Editor UX, Pan crash and four sources (2026-10-28), Fixed optional component captions (2026-10-26) (+19 more)

### Community 22 - "LIGHTING_EDITOR.md"
Cohesion: 0.15
Nodes (8): Evidence, Ice workspace - GS-06-R2, Behavioral evidence, GS-06-R2 editor, background and mask validation, Investigation and limits, Resulting interface, Design and collaboration, Editor interaction audit and resolution - 2026-11-13

### Community 23 - "IDriverModule"
Cohesion: 0.08
Nodes (7): AuxiliaryModule, IDriverModule, QLocalServer, QLocalSocket, QWinEventNotifier, IncompatibleModule, MissingInterface

### Community 24 - "LightingCanvas"
Cohesion: 0.02
Nodes (76): LightingCanvas, activeLayer_, additiveMarquee_, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, componentSeconds_ (+68 more)

### Community 25 - ".render"
Cohesion: 0.19
Nodes (4): canCreateEffect(), renderEffect(), validateEffect(), layer()

### Community 26 - "Transport"
Cohesion: 0.13
Nodes (7): Recording, opens, present, writes, Transport, opened, record

### Community 27 - "Mystic Light — Overall Architecture"
Cohesion: 0.14
Nodes (12): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll, Content (+4 more)

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "MSI Mystic Light – Lighting Studio"
Cohesion: 0.25
Nodes (8): Building from source, Documentation, Features, Getting started, License, MSI Mystic Light – Lighting Studio, Related repos, Supported hardware

### Community 30 - "ServicePanel"
Cohesion: 0.08
Nodes (22): main(), command(), host(), serviceInventory(), ServicePanel, active_, changedAt_, clockDirty_ (+14 more)

### Community 31 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 32 - "OutputPanel"
Cohesion: 0.08
Nodes (17): OutputPanel, authorize_, authorizing_, automatic_, channelDetails_, configured_, connections_, endpoints_ (+9 more)

### Community 33 - "EffectLayer"
Cohesion: 0.03
Nodes (58): EffectLayer, allChannels, breathe, center, channelTargets, color, colorCycle, colorCyclePositions (+50 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (71): ColorEditor, ColorListEditor, LightingWindow, activeLayer_, adjustingPanels_, advanced_, allChannels_, animation_ (+63 more)

### Community 35 - "OutputSession"
Cohesion: 0.12
Nodes (8): nativeKey(), OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_

### Community 36 - "EffectRenderer.cpp"
Cohesion: 0.19
Nodes (14): interpolate(), isGradient(), isParticle(), isPattern(), noise2d(), periodic(), polygon(), renderAtTime() (+6 more)

### Community 37 - "Lighting editor"
Cohesion: 0.09
Nodes (23): Authoring controls follow-up, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Integer controls, live color and identity (2026-10-26) (+15 more)

### Community 38 - "LenovoMotherboardModule.cpp"
Cohesion: 0.14
Nodes (6): LenovoMotherboardDriver, channels_, LenovoMotherboardModule, mapBrightness(), mapSpeed(), zoneFromAddress()

### Community 39 - "ServiceRuntime"
Cohesion: 0.06
Nodes (28): Scan, key(), ServiceRuntime, catalog_, clock_, connect_, connections_, directory_ (+20 more)

### Community 41 - "OpenRGB SDK client — cross-platform (esp. Linux) hardware access"
Cohesion: 0.13
Nodes (13): LED discovery, Data model (`RGBControllerInterface.h`) — what the server actually exposes, Deeper pass (2026-10-29, same day): SignalRgb.exe native binary + OpenRGB current-master re-check, Full decompile, targeted (2026-10-29, same session), Handshake (`NetworkClient::ConnectionThreadFunction`, `NetworkClient.cpp:1707`), License note (not a legal determination), Mapping onto this app's architecture, OpenRGB current-master re-check (`build/research/openrgb`, `aac7e551`) (+5 more)

### Community 43 - "IHidDevice"
Cohesion: 0.08
Nodes (13): ASRock Polychrome USB — implemented (2026-10-29, same session), Lenovo motherboard — implemented (2026-10-29, same session), Open questions, Overlap deep-dive 1: our board (`0DB0:0076`) — resolved, no conflict, Overlap deep-dive 2: ENE SMBus — not yet completed, Porting OpenRGB/SignalRGB hardware support into this app — motherboard tier, Resolved: `IHidDevice` read capability added (API 3), Transport classification (21 real controllers + 1 debug, excluded) (+5 more)

### Community 44 - "test_output_ui.cpp"
Cohesion: 0.22
Nodes (8): previewFrame_, configure(), connection(), identity(), OutputUiTest, QTemporaryDir, profile(), state()

### Community 45 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 46 - "ConflictDialog"
Cohesion: 0.12
Nodes (14): ConflictDialog, adminServicePid_, adminStop_, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_ (+6 more)

### Community 47 - "GeometryView"
Cohesion: 0.13
Nodes (6): GeometryView, artwork, commit, moved, pressPosition, scale

### Community 48 - "test_service.cpp"
Cohesion: 0.13
Nodes (8): Child, process, cmd(), identity(), project(), ServiceTest, private, ServiceTest::protocolBounds()

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
Cohesion: 0.15
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 55 - "ComponentPlacement"
Cohesion: 0.22
Nodes (6): ComponentPlacement, componentId, id, locked, origin, rotation

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.26
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.20
Nodes (6): validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 58 - ".edit"
Cohesion: 0.13
Nodes (8): loadComponentCatalog(), loadComponentDefinition(), channelAssignmentJson(), parseChannelAssignment(), hardwareName(), state_, applyStudioStyle(), channelProfile()

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "EneRamProtocol"
Cohesion: 0.23
Nodes (4): EneRamProtocol, address_, transfer_, AuxiliaryTest

### Community 64 - "inventory_hardware.py"
Cohesion: 0.05
Nodes (13): arguments(), build(), git(), integer(), js_return(), priority(), registrations(), snapshot() (+5 more)

### Community 65 - "Hardware modules"
Cohesion: 0.17
Nodes (12): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, Native GPU endpoint (+4 more)

### Community 67 - "OutputScheduler"
Cohesion: 0.14
Nodes (7): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "FakeHidDevice"
Cohesion: 0.15
Nodes (11): Completed: HID transport (2026-10-23), FakeHidDevice, featureReportResponse, getFeatureReportCalls, inputReportResponse, opened_, openSucceeds, readInputReportCalls (+3 more)

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "Status"
Cohesion: 0.29
Nodes (4): Status, error, generation, state

### Community 73 - "src/main.cpp"
Cohesion: 0.12
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 74 - "ColorListEditor"
Cohesion: 0.10
Nodes (12): Component catalog schema 1, ColorListEditor, add_, ColorListEditor::ColorListEditor(), gradient_, maximum_, minimum_, public (+4 more)

### Community 77 - "authorizeHardwareOutput"
Cohesion: 0.14
Nodes (8): authorizeHardwareOutput(), OutputProof, controlIdentity, identity, initial, profile, reports, previewOutputReports()

### Community 78 - "Effect implementation checklist"
Cohesion: 0.20
Nodes (9): Additional native patterns, Additional native source batch, Capability gate and particle follow-up, Editing follow-up, Effect implementation checklist, First implementation batch, Second batch and color picker, Second batch and remaining candidates (+1 more)

### Community 79 - "PaletteStrip"
Cohesion: 0.15
Nodes (12): PaletteStrip, add, begin, colors, dragging, end, gradient, move (+4 more)

### Community 80 - "LightingWindow::LightingWindow"
Cohesion: 0.11
Nodes (5): spin, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem, studioLogo()

### Community 81 - "EffectKind"
Cohesion: 0.05
Nodes (41): EffectKind, Aurora, Burst, Checkerboard, Circle, ConicalGradient, Cross, Diamond (+33 more)

### Community 82 - "SaturationValueField"
Cohesion: 0.08
Nodes (12): ColorPopup, finished, gestureBegun, sampling, SaturationValueField, brightness, changed, hue (+4 more)

### Community 83 - "StudioWidgets.cpp"
Cohesion: 0.14
Nodes (8): openStudioColorPicker(), ColorEditor::ColorEditor(), disclosure(), sliderField(), sliderPosition(), sliderRange(), studioMotionEnabled(), syncSliderFields()

### Community 84 - "ChannelAssignment"
Cohesion: 0.07
Nodes (19): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+11 more)

### Community 85 - "ConflictProcesses.cpp"
Cohesion: 0.15
Nodes (18): conflictReason(), creationTime(), matchesProcessIdentity(), mysticLightServiceStopped(), NativeServices, manager_, verifiedPid_, normalized() (+10 more)

### Community 87 - "HidApiDevice"
Cohesion: 0.23
Nodes (4): HidApiDevice, handle_, initialized_, initializeHid()

### Community 88 - ".pushColors"
Cohesion: 0.07
Nodes (15): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport() (+7 more)

### Community 89 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 90 - "Layer visibility, brush size and Delete"
Cohesion: 0.22
Nodes (9): Artwork alignment and custom device geometry, Detailed hardware artwork, Fixed-device settings versus attachable headers, Gaming workspace and responsive panels, Layer visibility, brush size and Delete, Mask playback, navigation and cosmetic hardware variants, Model labels, fullsize keyboards and zero-angle defaults, Scrollbar contrast and Effects preview divider (+1 more)

### Community 91 - "test_hp_omen.cpp"
Cohesion: 0.19
Nodes (5): HpOmenTest, identity(), OutputOnlyHid, featureWrites, profile()

### Community 93 - ".cancelGesture"
Cohesion: 0.08
Nodes (3): MaskTool, Brush, Gradient

### Community 94 - "FakeProcesses"
Cohesion: 0.06
Nodes (26): IConflictServices, FakeProcesses, adminActive, adminNeverStops, adminPendingPolls, adminPollError, adminQueryError, adminRequestError (+18 more)

### Community 95 - "Layer model proposal"
Cohesion: 0.20
Nodes (10): Acceptance matrix, Creation and selection contract, Delivery in reviewable increments, Explain the blends with a live result, Explicitly outside the first implementation, Feasibility evidence and resolved review points, Layer model proposal, Problem confirmed in the current code (+2 more)

### Community 96 - "Output session backend"
Cohesion: 0.33
Nodes (6): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification

### Community 97 - "HardwarePicker"
Cohesion: 0.12
Nodes (13): HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity, name (+5 more)

### Community 99 - "NativeEffect"
Cohesion: 0.11
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 100 - "LightingDocument"
Cohesion: 0.07
Nodes (16): chainStarts(), LightingDocument, colorPreviewBefore_, componentNames_, frameCache_, frameRevision_, frameSeconds_, identify_ (+8 more)

### Community 102 - "Lighting Studio GUI and interaction audit"
Cohesion: 0.18
Nodes (10): Color picker, Consolidation, Evidence, Findings, ordered by impact, Identity and direction, Implementation follow-up, Implementation order and acceptance, Integer scales (+2 more)

### Community 104 - "scanAuxiliaryHardware"
Cohesion: 0.18
Nodes (6): scanAuxiliaryHardware(), deviceIdentity(), present(), sameEndpoint(), vulcanControlIdentity(), PeripheralTest

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 106 - ".fromJson"
Cohesion: 0.09
Nodes (14): chainOffsets(), chainSharing(), channelMapping(), integer(), requiredText(), SharedRun, firstLed, lastLed (+6 more)

### Community 107 - "Gradient travel and blend verification (2026-10-24)"
Cohesion: 0.25
Nodes (8): Canvas playback, fullscreen and growing Layout, Gradient travel and blend verification (2026-10-24), Identify flash (2026-11-15), Live layout movement and output selection, Per-device LED arrangement, Physical LEDs and reserved protocol slots, RAM component geometry correction, Restored integrated lighting

### Community 108 - "Lightning and Fireworks visual review (2026-10-29)"
Cohesion: 0.50
Nodes (3): Before and after, Lightning and Fireworks visual review (2026-10-29), Sources and observation limits

### Community 109 - "OutputPanel.cpp"
Cohesion: 0.24
Nodes (4): channelKey(), OutputPanel::OutputPanel(), stateName(), sameOutputScope()

### Community 110 - "ProfileCandidate"
Cohesion: 0.12
Nodes (9): BoardIdentity, manufacturer, model, findProfileCandidates(), ProfileCandidate, device, profileId, readBoardIdentity() (+1 more)

### Community 113 - "service/main.cpp"
Cohesion: 0.14
Nodes (8): control(), knownFolder(), main(), readJson(), report(), run(), serviceMain(), verifyInstall()

### Community 114 - ".mousePressEvent"
Cohesion: 0.19
Nodes (4): effectTransform(), isShape(), centeredGradient(), gradientEndpoints()

### Community 115 - "AsrockPolychromeDriver"
Cohesion: 0.08
Nodes (10): AsrockPolychromeDriver, channels_, state_, AsrockPolychromeModule, mapSpeed(), ZoneState, applied, color (+2 more)

### Community 116 - "LenovoMotherboardMode"
Cohesion: 0.12
Nodes (12): BuildLenovoMotherboardReports(), LenovoMotherboardMode, Breathing, RainbowWave, Random, Sparkle, Specter, Static (+4 more)

### Community 117 - "ConflictService"
Cohesion: 0.11
Nodes (13): ConflictService, acceptsStop, commandLine, name, ownProcess, pid, state, IConflictProcesses (+5 more)

### Community 118 - "QJsonObject"
Cohesion: 0.13
Nodes (17): DeviceProfile::fromJson(), DeviceProfile::load(), integer(), text(), authorizedClient(), currentUserSid(), decode(), encode() (+9 more)

### Community 119 - "I2cInfo"
Cohesion: 0.18
Nodes (11): I2cInfo, address, ddc, displayMask, port, portSet, reg, regSize (+3 more)

### Community 120 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 122 - "Studio interaction and motion review"
Cohesion: 0.06
Nodes (28): Editor interaction implementation, Identity and color sampling, Persistence and limits, Pointer behavior, Rendering and output status, Result, Acceptance and implementation gates, Effect editor interaction research (+20 more)

### Community 126 - "QWidget"
Cohesion: 0.13
Nodes (8): QToolButton, studioThemeColor(), QToolBar, StudioBanner, artwork_, StudioHeaderToolbar, followHeaderRow_, StudioSectionButton

### Community 129 - "test_hid_backend.cpp"
Cohesion: 0.19
Nodes (8): HID_API_CALL hid_close(), HID_API_CALL hid_error(), HID_API_CALL hid_free_enumeration(), HID_API_CALL hid_get_feature_report(), HID_API_CALL hid_read_timeout(), HID_API_CALL hid_send_feature_report(), HID_API_CALL hid_write(), hid_device_

### Community 131 - "Tool"
Cohesion: 0.18
Nodes (10): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pan (+2 more)

### Community 133 - "ModuleRegistry"
Cohesion: 0.27
Nodes (4): fail(), ModuleRegistry, entries_, loadModules()

### Community 134 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 135 - "PawnIO integration and service feasibility"
Cohesion: 0.12
Nodes (15): Access control and process boundary, Conclusion, Coverage and next gate, Exact evidence, Installation and licensing boundary, PawnIO integration and service feasibility, Performance and recovery, Primary sources (+7 more)

### Community 137 - "Reliability and layer-design work (2026-11-11)"
Cohesion: 0.33
Nodes (6): Collaboration contract, Current status, Decisions and validation boundaries, Final software validation, Package and scope limits, Reliability and layer-design work (2026-11-11)

### Community 138 - "Lighting Studio editor UX review"
Cohesion: 0.15
Nodes (13): Additional procedural sources, Captures and measured checks, Crash evidence and validation boundaries, Creative-software research, Direction and scope, Findings and implementation, Gaming-Pro-Studio refinement, GS-06 finish (+5 more)

### Community 139 - "HidDescriptor"
Cohesion: 0.15
Nodes (10): hid_device_, HidDescriptor, interfaceNumber, manufacturer, path, product, productId, usage (+2 more)

### Community 140 - "QPainter"
Cohesion: 0.22
Nodes (3): SnapGrid, enabled, spacing

### Community 143 - "JARGB2 physical response tests - 2026-11-13 / 2026-11-14"
Cohesion: 0.22
Nodes (8): Approved tests and observations, Follow-up observations - 2026-11-14, JARGB2 physical response tests - 2026-11-13 / 2026-11-14, Pattern 6 - sequential fan-prefix qualification, Sleep/resume event, Software implications, Software status - 2026-11-14, Supported interpretation

### Community 146 - "ConflictProcess"
Cohesion: 0.17
Nodes (7): ConflictProcess, executable, manualStop, path, pid, reason, started

### Community 150 - "Hardware migration inventory and priorities"
Cohesion: 0.33
Nodes (6): Browse the full list, Hardware migration inventory and priorities, Prioritized migration queue, Reproduction and next decisions, Scale and counting units, Source provenance and limitations

### Community 151 - "OutputFrame"
Cohesion: 0.14
Nodes (12): buildFrames(), ChannelOutput, channelId, colors, native, software, OutputFrame, channels (+4 more)

### Community 153 - "AsrockPolychromeZone"
Cohesion: 0.15
Nodes (10): AsrockPolychromeZone, Argb1, Argb2, Audio, IoCover, Pcb, Pch, Rgb1 (+2 more)

### Community 155 - "QPushButton"
Cohesion: 0.22
Nodes (3): LayerDelegate, updatePlaybackButton(), studioToolIcon()

### Community 156 - ".resize"
Cohesion: 0.23
Nodes (6): NumericFieldStyle, QAbstractSpinBox, SliderWheelStep, remainder_, value_, styleStudioNumericField()

### Community 157 - "README.md"
Cohesion: 0.10
Nodes (12): OpenRGB controller-family worklist, HP Omen 30L migration: first independent-module increment, Integration and qualification, Pinned reference comparison, Selected protocol and scope, Entwurf: Kommentar für GitLab-Issues #4935 / #5429, Changes, Lighting Studio 0.1.0 (+4 more)

### Community 159 - "ParameterSpinBox"
Cohesion: 0.21
Nodes (4): maximum, minimum, ParameterSpinBox, scale_

### Community 160 - "EffectDefinition"
Cohesion: 0.25
Nodes (8): EffectDefinition, description, id, kind, label, particleCount, patternSites, patternSpeed

### Community 162 - "impl_"
Cohesion: 0.29
Nodes (6): impl_, address, library, ready, sent, temporaryFailure

### Community 163 - "Hardware support strategy"
Cohesion: 0.33
Nodes (6): Evidence and qualification, Execution boundary, Hardware support strategy, Independent hardware modules, Installed manufacturer software and peripheral settings, Mandatory comparison of reference implementations

### Community 164 - "ComponentDefinition"
Cohesion: 0.11
Nodes (16): ComponentDefinition, category, id, manufacturer, mapping, name, nonPhysicalSlots, positions (+8 more)

### Community 165 - "LightingCanvas.h"
Cohesion: 0.25
Nodes (7): Task 10: Qt tray app shell, QAction, QMenu, QPainter, QPushButton, QScrollBar, QTimer

### Community 168 - "ServiceServer.cpp"
Cohesion: 0.07
Nodes (16): Editor fixes and small audit fixes - 2026-11-14 (GS-06-R7, build/r4), GS-06-R2 UI correction - 2026-11-13, GS-06-R9 - 2026-11-15, Header LED count - 2026-11-15, Identify flash - 2026-11-15, JARGB2 shared routing - 2026-11-14, JARGB2 topology follow-up - updated 2026-11-14, LEDKeeper2 stop - 2026-11-14 (+8 more)

### Community 170 - "Artwork, continuous gestures and spatial controls (GS-06)"
Cohesion: 0.67
Nodes (3): Artwork, continuous gestures and spatial controls (GS-06), Image-editor crop and visible preview scope (2026-11-13), Startup compatibility hotfix (2026-10-30)

### Community 171 - "Unified Editor implementation"
Cohesion: 0.33
Nodes (5): Project compatibility, Rendering, color and numeric behavior, Unified Editor implementation, Verification, Workspace and interaction

### Community 172 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 173 - "Three-way cross-check: SignalRGB plugin tree added as a third source"
Cohesion: 0.33
Nodes (6): 162-byte MSI family — corroborated, not conflicting, ASRock Polychrome USB (`26CE:01A2`) — mostly confirmed, one open discrepancy, ENE SMBus (RAM) — register map confirmed 3-way, detection method rejected, Not yet cross-checked this pass, Our board (`0DB0:0076`) — triple-confirmed, including the shared bug, Three-way cross-check: SignalRGB plugin tree added as a third source

### Community 175 - "Workspace navigation and catalog identity"
Cohesion: 0.67
Nodes (3): Addressable LEDs and NV5 MKII evidence, MSI board configuration versus spatial pixel frames, Workspace navigation and catalog identity

### Community 178 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 179 - "EffectParameter"
Cohesion: 0.25
Nodes (6): EffectParameter, EffectLayer, id, label, schemaVersion, practicalEffectRange()

### Community 180 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

### Community 181 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 183 - "State"
Cohesion: 0.50
Nodes (4): State, Connected, Disconnected, Faulted

## Knowledge Gaps
- **1092 isolated node(s):** `mode`, `speed`, `color`, `applied`, `channels_` (+1087 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1640 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **49 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `IHidDevice` connect `IHidDevice` to `IDriver`, `AuxiliaryTransport`, `VirtualModule`, `HpOmenDriver`, `RgbColor`, `MsiColor`, `.load`, `Recording`, `IDriverModule`, `Transport`, `stdexcept`, `BuildReport0x51`, `memory`, `OutputSession`, `LenovoMotherboardModule.cpp`, `PeripheralModule`, `RecordingTransport`, `.connect`, `Transport`, `Hardware modules`, `FakeHidDevice`, `HidApiDevice`, `.pushColors`, `SignalRGB repository research`, `msiprotocol`, `NativeEffect`, `AuxiliaryDriver`, `AsrockPolychromeDriver`, `Record`?**
  _High betweenness centrality (0.130) - this node is a cross-community bridge._
- **Why does `LightingWindow` connect `LightingWindow` to `LightingWindow.cpp`, `test_editor.cpp`, `LightingCanvas`, `.render`, `button`, `ServicePanel`, `OutputPanel`, `EffectRenderer.cpp`, `.eventFilter`, `test_output_ui.cpp`, `test_service.cpp`, `.edit`, `src/main.cpp`, `LightingWindow::LightingWindow`, `Layer model proposal`, `HardwarePicker`, `LightingDocument`, `OutputPanel.cpp`, `ProfileCandidate`, `PreviewDivider`?**
  _High betweenness centrality (0.123) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `Output session backend` to `OutputScheduler`, `LightingDocument`, `ModuleRegistry`, `OutputFrame`, `.connect`, `README.md`?**
  _High betweenness centrality (0.089) - this node is a cross-community bridge._
- **Are the 85 inferred relationships involving `LightingWindow` (e.g. with `Problem confirmed in the current code` and `main()`) actually correct?**
  _`LightingWindow` has 85 INFERRED edges - model-reasoned connections that need verification._
- **Are the 8 inferred relationships involving `catalog()` (e.g. with `.componentLedOffsets()` and `.componentName()`) actually correct?**
  _`catalog()` has 8 INFERRED edges - model-reasoned connections that need verification._
- **Are the 80 inferred relationships involving `document_` (e.g. with `main()` and `addEffect()`) actually correct?**
  _`document_` has 80 INFERRED edges - model-reasoned connections that need verification._
- **What connects `mode`, `speed`, `color` to the rest of the system?**
  _1092 weakly-connected nodes found - possible documentation gaps or missing edges._