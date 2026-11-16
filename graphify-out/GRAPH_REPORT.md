# Graph Report - msi-mystic-light  (2026-11-16)

## Corpus Check
- 206 files · ~1,704,198 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 21 file(s) not represented in the graph (top: .csv 6, .cmd 3, .bin 3)

## Summary
- 3492 nodes · 7862 edges · 187 communities (140 shown, 47 thin omitted)
- Extraction: 79% EXTRACTED · 21% INFERRED · 0% AMBIGUOUS · INFERRED: 1670 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `8f166cc8`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- IDriver
- AuxiliaryTransport
- Implementation status
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- test_service.cpp
- RgbColor
- MsiDriver
- MsiGen1AdvArea
- Development Tools
- LightingWindow.cpp
- ServicePanel.cpp
- test_editor.cpp
- AeroBackend
- authorizeHardwareOutput
- ComponentDefinition
- PawnIoRam.cpp
- .open
- Recording
- AsrockPolychromeMode
- CONTINUE.md history archive
- LIGHTING_EDITOR.md
- IDriverModule
- LightingCanvas
- .edit
- Transport
- Mystic Light — Overall Architecture
- Reference repository reuse review
- MSI Mystic Light – Lighting Studio
- ServicePanel
- stdexcept
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
- Porting OpenRGB/SignalRGB hardware support into this app — motherboard tier
- test_output_ui.cpp
- Lighting Studio — Design Doc (v2)
- ConflictDialog
- GeometryView
- ConflictService
- Record
- MsiZoneSettings
- Lighting Studio: whole-interface and unified-editor review
- DriverInstance
- MsiLightingMode
- PairedTransport
- ComponentPlacement
- test_scheduler.cpp
- .connect
- .addComponent
- Transport
- EneRamProtocol
- LightingCanvas.cpp
- inventory_hardware.py
- ServiceRuntime.cpp
- StartupTest
- OutputScheduler
- FakeHidDevice
- .start
- Windows RGB access investigation
- Status
- src/main.cpp
- ColorListEditor
- StudioWidgets.cpp
- Effect implementation checklist
- .setNativeMode_rejectsInvalidEffectsAndClosedDevice
- PaletteStrip
- EffectKind
- SaturationValueField
- ChannelAssignment
- ConflictProcesses.cpp
- ModulesTest
- VirtualModule
- .pushColors
- SignalRGB repository research
- Layer visibility, brush size and Delete
- test_hp_omen.cpp
- HandleEntry
- .cancelGesture
- FakeProcesses
- Layer model proposal
- HardwarePicker::HardwarePicker
- HardwarePicker
- NativeEffect
- LightingDocument
- Lighting Studio GUI and interaction audit
- OutputProof
- Test review: recent lighting changes (2026-10-24)
- .fromJson
- Gradient travel and blend verification (2026-10-24)
- Lightning and Fireworks visual review (2026-10-29)
- OutputPanel.cpp
- qfile
- PawnIO AMD SMBus module
- IHidDevice
- service/main.cpp
- ServiceProtocol.cpp
- AsrockPolychromeDriver
- MsiColor
- StudioWidgets.h
- ScanResult
- BuildApex3ColorsPacket
- Record
- QToolButton
- Studio interaction and motion review
- DocumentEdit
- hw_inventory_linux.sh
- DeviceConflicts.cpp
- .mousePressEvent
- algorithm
- ComponentEditor
- closeWindow
- Tool
- .load
- Connection
- PawnIO integration and service feasibility
- CONTINUE.md
- Lighting Studio editor UX review
- closeWindow
- cmath
- branding/README.md
- JARGB2 physical response tests - 2026-11-13 / 2026-11-14
- ConflictProcess
- Hardware migration inventory and priorities
- OutputFrame
- .resize
- BuildBasiliskColorsReport
- QJsonObject
- Hardware modules
- README.md
- EffectParameter
- ParameterSpinBox
- EffectRenderer.h
- Uninstall-LightingStudio.ps1
- IConflictServices
- Hardware support strategy
- QMap
- LightingCanvas.h
- PeripheralModule
- Workspace navigation and catalog identity
- ServiceServer.cpp
- Artwork, continuous gestures and spatial controls (GS-06)
- Unified Editor implementation
- OutputConnection
- Distinct source capabilities and particle authoring
- ChannelEditor::ChannelEditor
- Project files and boundaries
- PowerResumeFilter
- .color
- LightingDocument.h
- SignalForge editor reference
- ScCloser
- State
- HidApiDevice
- LightingState
- LayerBlend

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 223 edges
2. `LightingCanvas` - 194 edges
3. `EditorTest` - 144 edges
4. `catalog()` - 94 edges
5. `document_` - 82 edges
6. `LightingDocument` - 78 edges
7. `EffectLayer` - 71 edges
8. `ServiceRuntime` - 66 edges
9. `Implementation status` - 66 edges
10. `IHidDevice` - 59 edges

## Surprising Connections (you probably didn't know these)
- `Project compatibility` --references--> `locked()`  [INFERRED]
  docs/design/UNIFIED_EDITOR_IMPLEMENTATION_2026-10-27.md → src/backends/PawnIoRam.cpp
- `Problem confirmed in the current code` --references--> `LightingWindow`  [INFERRED]
  docs/LAYER_MODEL_PLAN.md → src/editor/LightingWindow.h
- `Open questions` --references--> `IHidDevice`  [INFERRED]
  docs/OPENRGB_HARDWARE_PORT_PLAN.md → src/hid/IHidDevice.h
- `What's portable right now` --references--> `IHidDevice`  [INFERRED]
  docs/OPENRGB_HARDWARE_PORT_PLAN.md → src/hid/IHidDevice.h
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h

## Import Cycles
- None detected.

## Communities (187 total, 47 thin omitted)

### Community 0 - "IDriver"
Cohesion: 0.10
Nodes (17): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+9 more)

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

### Community 6 - "test_service.cpp"
Cohesion: 0.13
Nodes (6): Child, process, cmd(), ServiceTest, private, ServiceTest::protocolBounds()

### Community 7 - "RgbColor"
Cohesion: 0.12
Nodes (8): Responsibilities, HpOmenDriver, channels_, HpOmenModule, RgbColor, b, g, r

### Community 8 - "MsiDriver"
Cohesion: 0.14
Nodes (6): MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "LightingWindow.cpp"
Cohesion: 0.06
Nodes (8): spin, colorIcon(), LayerTree, reordered, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem, studioLogo()

### Community 12 - "ServicePanel.cpp"
Cohesion: 0.17
Nodes (6): main(), command(), host(), ServiceConflictProcesses::ServiceConflictProcesses(), serviceInventory(), ServicePanel::ServicePanel()

### Community 13 - "test_editor.cpp"
Cohesion: 0.07
Nodes (6): document_, addEffect(), artifactDirectory(), catalog(), EditorTest, QTemporaryDir

### Community 14 - "AeroBackend"
Cohesion: 0.08
Nodes (25): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+17 more)

### Community 15 - "authorizeHardwareOutput"
Cohesion: 0.22
Nodes (5): authorizeHardwareOutput(), loadModules(), previewOutputReports(), RecordingTransport, reports_

### Community 16 - "ComponentDefinition"
Cohesion: 0.08
Nodes (21): ComponentDefinition, category, id, manufacturer, mapping, name, nonPhysicalSlots, positions (+13 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.12
Nodes (16): call(), ~Impl(), libraryPath(), locked(), onPrimary(), PawnIoRam, impl_, address (+8 more)

### Community 18 - ".open"
Cohesion: 0.26
Nodes (3): AsrockPolychromeTest, identity(), profile()

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "AsrockPolychromeMode"
Cohesion: 0.07
Nodes (25): AsrockPolychromeMode, Breathing, Cram, Music, Neon, Off, Rainbow, Random (+17 more)

### Community 21 - "CONTINUE.md history archive"
Cohesion: 0.07
Nodes (27): Active-layer picking, canvas labels and transport controls (2026-10-28), Authoring controls, pan and two patterns (2026-10-28), Components fullscreen and edit-mode visibility fix (2026-10-28), CONTINUE.md history archive, Dense strip SVG visibility regression (2026-10-26), Editor interaction and channel preview follow-up (2026-10-27), Editor UX, Pan crash and four sources (2026-10-28), Fixed optional component captions (2026-10-26) (+19 more)

### Community 22 - "LIGHTING_EDITOR.md"
Cohesion: 0.10
Nodes (14): Collaboration contract, Current status, Decisions and validation boundaries, Final software validation, Package and scope limits, Reliability and layer-design work (2026-11-11), Evidence, Ice workspace - GS-06-R2 (+6 more)

### Community 23 - "IDriverModule"
Cohesion: 0.07
Nodes (8): AuxiliaryModule, MsiModule, IDriverModule, QLocalServer, QLocalSocket, QWinEventNotifier, IncompatibleModule, MissingInterface

### Community 24 - "LightingCanvas"
Cohesion: 0.02
Nodes (76): LightingCanvas, activeLayer_, additiveMarquee_, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, componentSeconds_ (+68 more)

### Community 25 - ".edit"
Cohesion: 0.16
Nodes (7): canCreateEffect(), isPattern(), renderEffect(), validateEffect(), state_, validate(), layer()

### Community 26 - "Transport"
Cohesion: 0.12
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
Nodes (19): ServiceConflictProcesses, rpc_, ServicePanel, active_, changedAt_, clockDirty_, commands_, dirty_ (+11 more)

### Community 31 - "stdexcept"
Cohesion: 0.16
Nodes (6): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 32 - "OutputPanel"
Cohesion: 0.08
Nodes (17): OutputPanel, authorize_, authorizing_, automatic_, channelDetails_, configured_, connections_, endpoints_ (+9 more)

### Community 33 - "EffectLayer"
Cohesion: 0.03
Nodes (59): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+51 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (71): ColorEditor, ColorListEditor, LightingWindow, activeLayer_, adjustingPanels_, advanced_, allChannels_, animation_ (+63 more)

### Community 35 - "OutputSession"
Cohesion: 0.12
Nodes (8): nativeKey(), OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_

### Community 36 - "EffectRenderer.cpp"
Cohesion: 0.15
Nodes (15): interpolate(), isGradient(), isParticle(), isShape(), noise2d(), periodic(), polygon(), renderAtTime() (+7 more)

### Community 37 - "Lighting editor"
Cohesion: 0.09
Nodes (23): Authoring controls follow-up, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Integer controls, live color and identity (2026-10-26) (+15 more)

### Community 38 - "LenovoMotherboardModule.cpp"
Cohesion: 0.07
Nodes (18): LenovoMotherboardDriver, channels_, LenovoMotherboardModule, mapBrightness(), mapSpeed(), zoneFromAddress(), BuildLenovoMotherboardReports(), LenovoMotherboardMode (+10 more)

### Community 39 - "ServiceRuntime"
Cohesion: 0.05
Nodes (30): ProfileCandidate, device, profileId, ServiceRuntime, background_, catalog_, clock_, connections_ (+22 more)

### Community 41 - "OpenRGB SDK client — cross-platform (esp. Linux) hardware access"
Cohesion: 0.13
Nodes (13): LED discovery, Data model (`RGBControllerInterface.h`) — what the server actually exposes, Deeper pass (2026-10-29, same day): SignalRgb.exe native binary + OpenRGB current-master re-check, Full decompile, targeted (2026-10-29, same session), Handshake (`NetworkClient::ConnectionThreadFunction`, `NetworkClient.cpp:1707`), License note (not a legal determination), Mapping onto this app's architecture, OpenRGB current-master re-check (`build/research/openrgb`, `aac7e551`) (+5 more)

### Community 42 - "ComponentEditor.cpp"
Cohesion: 0.09
Nodes (3): CoordinateDelegate, QPainter, button()

### Community 43 - "Porting OpenRGB/SignalRGB hardware support into this app — motherboard tier"
Cohesion: 0.14
Nodes (14): 162-byte MSI family — corroborated, not conflicting, ASRock Polychrome USB (`26CE:01A2`) — mostly confirmed, one open discrepancy, ASRock Polychrome USB — implemented (2026-10-29, same session), ENE SMBus (RAM) — register map confirmed 3-way, detection method rejected, Lenovo motherboard — implemented (2026-10-29, same session), Not yet cross-checked this pass, Open questions, Our board (`0DB0:0076`) — triple-confirmed, including the shared bug (+6 more)

### Community 44 - "test_output_ui.cpp"
Cohesion: 0.23
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

### Community 48 - "ConflictService"
Cohesion: 0.11
Nodes (13): ConflictService, acceptsStop, commandLine, name, ownProcess, pid, state, IConflictProcesses (+5 more)

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "MsiZoneSettings"
Cohesion: 0.14
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

### Community 54 - "PairedTransport"
Cohesion: 0.22
Nodes (3): PairedTransport, control, leds

### Community 55 - "ComponentPlacement"
Cohesion: 0.18
Nodes (6): ComponentPlacement, componentId, id, locked, origin, rotation

### Community 56 - "test_scheduler.cpp"
Cohesion: 0.26
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.20
Nodes (6): validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 61 - "EneRamProtocol"
Cohesion: 0.18
Nodes (4): EneRamProtocol, address_, transfer_, AuxiliaryTest

### Community 64 - "inventory_hardware.py"
Cohesion: 0.05
Nodes (13): arguments(), build(), git(), integer(), js_return(), priority(), registrations(), snapshot() (+5 more)

### Community 65 - "ServiceRuntime.cpp"
Cohesion: 0.17
Nodes (5): Scan, key(), ServiceRuntime::ServiceRuntime(), writeFile(), identity()

### Community 67 - "OutputScheduler"
Cohesion: 0.10
Nodes (13): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification, OutputScheduler, mutex_ (+5 more)

### Community 68 - "FakeHidDevice"
Cohesion: 0.14
Nodes (12): Completed: HID transport (2026-10-23), Task 8: Canvas render loop, FakeHidDevice, featureReportResponse, getFeatureReportCalls, inputReportResponse, opened_, openSucceeds (+4 more)

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "Status"
Cohesion: 0.29
Nodes (4): Status, error, generation, state

### Community 73 - "src/main.cpp"
Cohesion: 0.11
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 74 - "ColorListEditor"
Cohesion: 0.12
Nodes (10): ColorListEditor, add_, gradient_, maximum_, minimum_, public, remove_, rows_ (+2 more)

### Community 77 - "StudioWidgets.cpp"
Cohesion: 0.08
Nodes (18): openStudioColorPicker(), ComponentEditor::ComponentEditor(), updatePlaybackButton(), ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), disclosure(), installParameterTooltips(), QToolBar (+10 more)

### Community 78 - "Effect implementation checklist"
Cohesion: 0.20
Nodes (9): Additional native patterns, Additional native source batch, Capability gate and particle follow-up, Editing follow-up, Effect implementation checklist, First implementation batch, Second batch and color picker, Second batch and remaining candidates (+1 more)

### Community 79 - ".setNativeMode_rejectsInvalidEffectsAndClosedDevice"
Cohesion: 0.29
Nodes (3): identity(), LenovoMotherboardTest, profile()

### Community 80 - "PaletteStrip"
Cohesion: 0.15
Nodes (12): PaletteStrip, add, begin, colors, dragging, end, gradient, move (+4 more)

### Community 81 - "EffectKind"
Cohesion: 0.05
Nodes (41): EffectKind, Aurora, Burst, Checkerboard, Circle, ConicalGradient, Cross, Diamond (+33 more)

### Community 82 - "SaturationValueField"
Cohesion: 0.08
Nodes (12): ColorPopup, finished, gestureBegun, sampling, SaturationValueField, brightness, changed, hue (+4 more)

### Community 84 - "ChannelAssignment"
Cohesion: 0.15
Nodes (11): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+3 more)

### Community 85 - "ConflictProcesses.cpp"
Cohesion: 0.15
Nodes (20): conflictReason(), creationTime(), isSignalRgbService(), matchesProcessIdentity(), mysticLightServiceStopped(), NativeServices, manager_, verifiedPid_ (+12 more)

### Community 88 - ".pushColors"
Cohesion: 0.11
Nodes (7): Task 10: Roccat Kone Aimo driver, feature(), output(), BuildKoneColorsReport(), BuildKoneInitReport(), BuildVulcanColorPackets(), BuildVulcanEnableDirectReport()

### Community 89 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 90 - "Layer visibility, brush size and Delete"
Cohesion: 0.22
Nodes (9): Artwork alignment and custom device geometry, Detailed hardware artwork, Fixed-device settings versus attachable headers, Gaming workspace and responsive panels, Layer visibility, brush size and Delete, Mask playback, navigation and cosmetic hardware variants, Model labels, fullsize keyboards and zero-angle defaults, Scrollbar contrast and Effects preview divider (+1 more)

### Community 91 - "test_hp_omen.cpp"
Cohesion: 0.22
Nodes (5): HpOmenTest, identity(), OutputOnlyHid, featureWrites, profile()

### Community 92 - "HandleEntry"
Cohesion: 0.15
Nodes (9): HandleEntry, access, attributes, backtrace, handle, object, pid, reserved (+1 more)

### Community 93 - ".cancelGesture"
Cohesion: 0.10
Nodes (3): MaskTool, Brush, Gradient

### Community 94 - "FakeProcesses"
Cohesion: 0.06
Nodes (27): Responsive service and installed conflict dialog - 2026-11-16, FakeProcesses, adminActive, adminNeverStops, adminPendingPolls, adminPollError, adminQueryError, adminRequestError (+19 more)

### Community 95 - "Layer model proposal"
Cohesion: 0.20
Nodes (10): Acceptance matrix, Creation and selection contract, Delivery in reviewable increments, Explain the blends with a live result, Explicitly outside the first implementation, Feasibility evidence and resolved review points, Layer model proposal, Problem confirmed in the current code (+2 more)

### Community 96 - "HardwarePicker::HardwarePicker"
Cohesion: 0.25
Nodes (5): Task 4: Component data model, generic(), HardwarePicker::HardwarePicker(), installedProducts(), isFan()

### Community 97 - "HardwarePicker"
Cohesion: 0.12
Nodes (13): HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity, name (+5 more)

### Community 99 - "NativeEffect"
Cohesion: 0.10
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 100 - "LightingDocument"
Cohesion: 0.06
Nodes (15): parseChannelAssignment(), LightingDocument, colorPreviewBefore_, componentNames_, frameCache_, frameRevision_, frameSeconds_, identify_ (+7 more)

### Community 102 - "Lighting Studio GUI and interaction audit"
Cohesion: 0.18
Nodes (10): Color picker, Consolidation, Evidence, Findings, ordered by impact, Identity and direction, Implementation follow-up, Implementation order and acceptance, Integer scales (+2 more)

### Community 104 - "OutputProof"
Cohesion: 0.20
Nodes (6): OutputProof, controlIdentity, identity, initial, profile, reports

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 106 - ".fromJson"
Cohesion: 0.16
Nodes (7): Component catalog schema 1, chainOffsets(), chainSharing(), channelMapping(), integer(), requiredText(), ComponentsTest

### Community 107 - "Gradient travel and blend verification (2026-10-24)"
Cohesion: 0.25
Nodes (8): Canvas playback, fullscreen and growing Layout, Gradient travel and blend verification (2026-10-24), Identify flash (2026-11-15), Live layout movement and output selection, Per-device LED arrangement, Physical LEDs and reserved protocol slots, RAM component geometry correction, Restored integrated lighting

### Community 108 - "Lightning and Fireworks visual review (2026-10-29)"
Cohesion: 0.50
Nodes (3): Before and after, Lightning and Fireworks visual review (2026-10-29), Sources and observation limits

### Community 109 - "OutputPanel.cpp"
Cohesion: 0.20
Nodes (5): channelKey(), OutputPanel::OutputPanel(), stateName(), prepareOutput(), sameOutputScope()

### Community 110 - "qfile"
Cohesion: 0.14
Nodes (9): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), readBoardIdentity(), present(), sameEndpoint() (+1 more)

### Community 112 - "IHidDevice"
Cohesion: 0.10
Nodes (8): Task 18: Final wire-up and real-hardware verification, Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, AuxiliaryDriver, channel_, PeripheralDriver, channel_, initialized_, IHidDevice

### Community 113 - "service/main.cpp"
Cohesion: 0.19
Nodes (8): control(), knownFolder(), main(), readJson(), report(), run(), serviceMain(), verifyInstall()

### Community 114 - "ServiceProtocol.cpp"
Cohesion: 0.26
Nodes (11): authorizedClient(), currentUserSid(), decode(), encode(), installedHost(), isSystem(), pipeName(), processSid() (+3 more)

### Community 115 - "AsrockPolychromeDriver"
Cohesion: 0.09
Nodes (10): AsrockPolychromeDriver, channels_, state_, AsrockPolychromeModule, mapSpeed(), ZoneState, applied, color (+2 more)

### Community 116 - "MsiColor"
Cohesion: 0.15
Nodes (16): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness, Task 2: MSI wire-protocol encoders (+8 more)

### Community 117 - "StudioWidgets.h"
Cohesion: 0.13
Nodes (10): ColorEditor, hex_, picker_, public, QDoubleSpinBox, QLabel, QLineEdit, QPushButton (+2 more)

### Community 118 - "ScanResult"
Cohesion: 0.19
Nodes (6): ScanResult, blocked, error, inventory, scanFailed, connect_

### Community 119 - "BuildApex3ColorsPacket"
Cohesion: 0.36
Nodes (3): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), BuildApex3BrightnessPacket(), BuildApex3ColorsPacket()

### Community 120 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 121 - "QToolButton"
Cohesion: 0.07
Nodes (6): QToolButton, LayerDelegate, PreviewDivider, PreviewSplitter, studioThemeColor(), StudioSectionButton

### Community 122 - "Studio interaction and motion review"
Cohesion: 0.06
Nodes (28): Editor interaction implementation, Identity and color sampling, Persistence and limits, Pointer behavior, Rendering and output status, Result, Acceptance and implementation gates, Effect editor interaction research (+20 more)

### Community 125 - "DeviceConflicts.cpp"
Cohesion: 0.11
Nodes (28): alive(), creationTime(), enableDebugPrivilege(), endpointBusy(), endpointLabel(), folderOf(), HandleCloser, hostedService() (+20 more)

### Community 129 - "ComponentEditor"
Cohesion: 0.25
Nodes (4): ComponentEditor, category_, manufacturer_, nonPhysicalSlots_

### Community 131 - "Tool"
Cohesion: 0.18
Nodes (10): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pan (+2 more)

### Community 133 - ".load"
Cohesion: 0.17
Nodes (3): fail(), ModuleRegistry, entries_

### Community 134 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 135 - "PawnIO integration and service feasibility"
Cohesion: 0.11
Nodes (16): Access control and process boundary, Conclusion, Coverage and next gate, Exact evidence, Installation and licensing boundary, PawnIO integration and service feasibility, Performance and recovery, Primary sources (+8 more)

### Community 137 - "CONTINUE.md"
Cohesion: 0.17
Nodes (10): Branching and releases - 2026-11-15, Editor fixes and small audit fixes - 2026-11-14 (GS-06-R7, build/r4), GS-06-R2 UI correction - 2026-11-13, GS-06-R9 - 2026-11-15, Header LED count - 2026-11-15, Identify flash - 2026-11-15, JARGB2 shared routing - 2026-11-14, JARGB2 topology follow-up - updated 2026-11-14 (+2 more)

### Community 138 - "Lighting Studio editor UX review"
Cohesion: 0.15
Nodes (13): Additional procedural sources, Captures and measured checks, Crash evidence and validation boundaries, Creative-software research, Direction and scope, Findings and implementation, Gaming-Pro-Studio refinement, GS-06 finish (+5 more)

### Community 140 - "cmath"
Cohesion: 0.12
Nodes (4): importLegacyHardware(), SnapGrid, enabled, spacing

### Community 143 - "JARGB2 physical response tests - 2026-11-13 / 2026-11-14"
Cohesion: 0.22
Nodes (8): Approved tests and observations, Follow-up observations - 2026-11-14, JARGB2 physical response tests - 2026-11-13 / 2026-11-14, Pattern 6 - sequential fan-prefix qualification, Sleep/resume event, Software implications, Software status - 2026-11-14, Supported interpretation

### Community 146 - "ConflictProcess"
Cohesion: 0.07
Nodes (27): ConflictProcess, executable, manualStop, path, pid, provenHolder, reason, started (+19 more)

### Community 150 - "Hardware migration inventory and priorities"
Cohesion: 0.33
Nodes (6): Browse the full list, Hardware migration inventory and priorities, Prioritized migration queue, Reproduction and next decisions, Scale and counting units, Source provenance and limitations

### Community 151 - "OutputFrame"
Cohesion: 0.13
Nodes (11): buildFrames(), ChannelOutput, channelId, colors, native, software, OutputFrame, channels (+3 more)

### Community 153 - ".resize"
Cohesion: 0.23
Nodes (6): NumericFieldStyle, QAbstractSpinBox, SliderWheelStep, remainder_, value_, styleStudioNumericField()

### Community 154 - "BuildBasiliskColorsReport"
Cohesion: 0.57
Nodes (3): BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport(), CalculateCrc()

### Community 155 - "QJsonObject"
Cohesion: 0.18
Nodes (7): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text(), QLocalSocket, QJsonObject

### Community 156 - "Hardware modules"
Cohesion: 0.17
Nodes (12): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, Native GPU endpoint (+4 more)

### Community 157 - "README.md"
Cohesion: 0.10
Nodes (12): OpenRGB controller-family worklist, HP Omen 30L migration: first independent-module increment, Integration and qualification, Pinned reference comparison, Selected protocol and scope, Entwurf: Kommentar für GitLab-Issues #4935 / #5429, Changes, Lighting Studio 0.1.0 (+4 more)

### Community 158 - "EffectParameter"
Cohesion: 0.22
Nodes (7): EffectParameter, EffectLayer, id, label, maximum, minimum, schemaVersion

### Community 160 - "EffectRenderer.h"
Cohesion: 0.18
Nodes (9): EffectDefinition, description, id, kind, label, particleCount, patternSites, patternSpeed (+1 more)

### Community 163 - "Hardware support strategy"
Cohesion: 0.33
Nodes (6): Evidence and qualification, Execution boundary, Hardware support strategy, Independent hardware modules, Installed manufacturer software and peripheral settings, Mandatory comparison of reference implementations

### Community 164 - "QMap"
Cohesion: 0.25
Nodes (4): SharedRun, firstLed, lastLed, source

### Community 165 - "LightingCanvas.h"
Cohesion: 0.13
Nodes (8): Task 10: Qt tray app shell, LightingCanvas::LightingCanvas(), QAction, QMenu, QPainter, QPushButton, QScrollBar, QTimer

### Community 167 - "Workspace navigation and catalog identity"
Cohesion: 0.67
Nodes (3): Addressable LEDs and NV5 MKII evidence, MSI board configuration versus spatial pixel frames, Workspace navigation and catalog identity

### Community 168 - "ServiceServer.cpp"
Cohesion: 0.09
Nodes (8): Service pipe access list - 2026-11-14 (Windows only), ServiceServer, clients_, ServiceServer::connected(), error_, handler_, owner_, ServiceServer::ServiceServer()

### Community 170 - "Artwork, continuous gestures and spatial controls (GS-06)"
Cohesion: 0.67
Nodes (3): Artwork, continuous gestures and spatial controls (GS-06), Image-editor crop and visible preview scope (2026-11-13), Startup compatibility hotfix (2026-10-30)

### Community 171 - "Unified Editor implementation"
Cohesion: 0.33
Nodes (5): Project compatibility, Rendering, color and numeric behavior, Unified Editor implementation, Verification, Workspace and interaction

### Community 172 - "OutputConnection"
Cohesion: 0.40
Nodes (3): OutputConnection, initial, present

### Community 175 - "ChannelEditor::ChannelEditor"
Cohesion: 0.18
Nodes (6): chainStarts(), channelAssignmentJson(), channelHasFixedComponent(), sharedColorNote(), validateChannelAssignments(), ChannelEditor::ChannelEditor()

### Community 180 - "LightingDocument.h"
Cohesion: 0.23
Nodes (4): LightingState, ChannelEditor, fixedComponent_, initial_

### Community 181 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 183 - "State"
Cohesion: 0.50
Nodes (4): State, Connected, Disconnected, Faulted

### Community 184 - "HidApiDevice"
Cohesion: 0.06
Nodes (26): Resolved: `IHidDevice` read capability added (API 3), hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer (+18 more)

### Community 186 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 187 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

## Knowledge Gaps
- **1132 isolated node(s):** `mode`, `speed`, `color`, `applied`, `channels_` (+1127 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1698 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **47 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `LightingWindow.cpp`, `ServicePanel.cpp`, `test_editor.cpp`, `ComponentDefinition`, `LightingCanvas`, `.edit`, `ServicePanel`, `OutputPanel`, `EffectRenderer.cpp`, `ServiceRuntime`, `test_output_ui.cpp`, `ComponentPlacement`, `.addComponent`, `src/main.cpp`, `Layer model proposal`, `HardwarePicker`, `LightingDocument`, `OutputPanel.cpp`, `QToolButton`?**
  _High betweenness centrality (0.120) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `IDriver`, `AuxiliaryTransport`, `.load`, `RgbColor`, `MsiDriver`, `authorizeHardwareOutput`, `Recording`, `Transport`, `Hardware modules`, `stdexcept`, `OutputSession`, `LenovoMotherboardModule.cpp`, `PeripheralModule`, `Porting OpenRGB/SignalRGB hardware support into this app — motherboard tier`, `PairedTransport`, `HidApiDevice`, `.connect`, `Transport`, `FakeHidDevice`, `VirtualModule`, `SignalRGB repository research`, `msiprotocol`, `NativeEffect`, `AsrockPolychromeDriver`, `MsiColor`, `BuildApex3ColorsPacket`, `Record`, `algorithm`?**
  _High betweenness centrality (0.109) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `OutputScheduler` to `LightingDocument`, `.load`, `OutputFrame`, `.connect`, `README.md`?**
  _High betweenness centrality (0.085) - this node is a cross-community bridge._
- **Are the 85 inferred relationships involving `LightingWindow` (e.g. with `Problem confirmed in the current code` and `main()`) actually correct?**
  _`LightingWindow` has 85 INFERRED edges - model-reasoned connections that need verification._
- **Are the 8 inferred relationships involving `catalog()` (e.g. with `.componentLedOffsets()` and `.componentName()`) actually correct?**
  _`catalog()` has 8 INFERRED edges - model-reasoned connections that need verification._
- **Are the 81 inferred relationships involving `document_` (e.g. with `main()` and `addEffect()`) actually correct?**
  _`document_` has 81 INFERRED edges - model-reasoned connections that need verification._
- **What connects `mode`, `speed`, `color` to the rest of the system?**
  _1132 weakly-connected nodes found - possible documentation gaps or missing edges._