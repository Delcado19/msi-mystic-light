# Graph Report - msi-mystic-light  (2026-10-28)

## Corpus Check
- 163 files · ~346,715 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 10 file(s) not represented in the graph (top: (none) 2, .cmd 2, .qrc 2)

## Summary
- 2680 nodes · 5921 edges · 146 communities (108 shown, 38 thin omitted)
- Extraction: 79% EXTRACTED · 21% INFERRED · 0% AMBIGUOUS · INFERRED: 1253 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `39b3a7a7`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- IDriver
- AuxiliaryTransport
- MsiZoneSettings
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- NativeEffect
- HidApiDevice
- MsiDriver
- MsiGen1AdvArea
- Development Tools
- PreviewDivider
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
- LightingDocument.h
- LightingCanvas
- Mystic Light — Overall Architecture
- ARCHITECTURE.md
- Reference repository reuse review
- MSI Mystic Light - Lighting Studio
- ServicePanel
- OutputPanel.cpp
- LightingCanvas.cpp
- EffectLayer
- LightingWindow
- IHidDevice
- EffectRenderer.cpp
- Lighting editor
- Record
- ServiceRuntime
- ConflictProcess
- HardwareArtwork.cpp
- SignalRGB repository research
- test_output_ui.cpp
- Effect editor interaction research
- ConflictDialog
- GeometryView
- OutputSession
- Record
- Lighting Studio — Design Doc (v2)
- Lighting Studio: whole-interface and unified-editor review
- DriverInstance
- BuildReport0x51
- RecordingTransport
- ComponentPlacement
- .latestDuringSlowWrite
- test_output.cpp
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
- src/main.cpp
- Unified Editor implementation
- MaskTool
- StudioWidgets.cpp
- LightingWindow.cpp
- LightingState
- EffectKind
- SaturationValueField
- LightingDocument
- EffectParameter
- ConflictProcesses.cpp
- stdexcept
- .edit
- .pushColors
- .mousePressEvent
- .load
- QJsonObject
- ProfileCandidate
- test_service.cpp
- AuxiliaryDriver
- ParameterSpinBox
- OutputConnection
- PeripheralModule
- closeWindow
- Studio interaction and motion review
- Lighting Studio GUI and interaction audit
- hardwareCategory
- Test review: recent lighting changes (2026-10-24)
- .connect
- Hardware modules
- QPainter
- OutputFrame
- ServiceProtocol.cpp
- PawnIO AMD SMBus module
- ChannelAssignment
- service/main.cpp
- CONTINUE.md
- SignalForge editor reference
- IConflictProcesses
- StartupTest
- VirtualModule
- ChannelEditor::ChannelEditor
- HidDescriptor
- Editor interaction implementation
- Transport
- .componentAuthoring
- test_hid_backend.cpp
- BuildApex3ColorsPacket
- ComponentEditor
- .addHeaderComponents
- ConflictDialog.cpp
- LayerBlend
- Tool
- .release
- colorIcon
- Output session backend
- PawnIO integration and service feasibility
- .paint
- authorizeHardwareOutput
- EffectRenderer.h
- LayerTree
- branding/README.md
- Connection

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 187 edges
2. `LightingCanvas` - 170 edges
3. `EditorTest` - 105 edges
4. `LightingDocument` - 66 edges
5. `Implementation status` - 66 edges
6. `catalog()` - 63 edges
7. `document_` - 56 edges
8. `EffectLayer` - 55 edges
9. `ServiceRuntime` - 52 edges
10. `OutputPanel` - 47 edges

## Surprising Connections (you probably didn't know these)
- `Project compatibility` --references--> `locked()`  [INFERRED]
  docs/design/UNIFIED_EDITOR_IMPLEMENTATION_2026-10-27.md → src/backends/PawnIoRam.cpp
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Task 18: Final wire-up and real-hardware verification` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → src/hid/IHidDevice.h
- `Output scheduler` --references--> `OutputScheduler`  [INFERRED]
  docs/OUTPUT_SESSION.md → src/output/OutputScheduler.h
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp

## Import Cycles
- None detected.

## Communities (146 total, 38 thin omitted)

### Community 0 - "IDriver"
Cohesion: 0.10
Nodes (19): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+11 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.15
Nodes (5): AuxiliaryTransport, gpu_, ram_, ready_, openAuxiliaryTransport()

### Community 2 - "MsiZoneSettings"
Cohesion: 0.07
Nodes (32): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness (+24 more)

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
Cohesion: 0.10
Nodes (8): hid_device_, HidApiDevice, handle_, initialized_, HidRuntime, ready, initializeHid(), HidBackendTest

### Community 8 - "MsiDriver"
Cohesion: 0.13
Nodes (11): Responsibilities, MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_, RgbColor (+3 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "test_editor.cpp"
Cohesion: 0.08
Nodes (7): document_, addEffect(), artifactDirectory(), catalog(), channelProfile(), EditorTest, QTemporaryDir

### Community 14 - "AeroBackend"
Cohesion: 0.11
Nodes (14): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+6 more)

### Community 15 - "OutputPanel"
Cohesion: 0.09
Nodes (17): OutputPanel, authorize_, authorizing_, automatic_, channelDetails_, configured_, connections_, endpoints_ (+9 more)

### Community 16 - "ColorListEditor"
Cohesion: 0.07
Nodes (23): Component catalog schema 1, ColorEditor, hex_, picker_, public, ColorListEditor, add_, gradient_ (+15 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.17
Nodes (8): call(), ~Impl(), libraryPath(), locked(), onPrimary(), PawnIoRam, pawnIoRamDiscoveryError(), scanPawnIoRam()

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "HardwarePicker"
Cohesion: 0.11
Nodes (14): QPainter, HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity (+6 more)

### Community 21 - "Implementation status"
Cohesion: 0.03
Nodes (64): Accepted effect authoring follow-ups (2026-10-23), Aero visibility and build cleanup (2026-10-25), Artwork geometry and frame regression fixes (2026-10-25), Canvas scrollbars and right-hand minimap (2026-10-24), Canvas Start/Stop, fullscreen and growing Layout (2026-10-24), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23) (+56 more)

### Community 22 - "README.md"
Cohesion: 0.15
Nodes (6): Entwurf: Kommentar für GitLab-Issues #4935 / #5429, Changes, Lighting Studio 0.1.0, Requirements and limitations, Hardware artwork, Third-party runtime components

### Community 23 - "LightingDocument.h"
Cohesion: 0.08
Nodes (6): AuxiliaryModule, MsiModule, LightingState, IDriverModule, IncompatibleModule, MissingInterface

### Community 24 - "LightingCanvas"
Cohesion: 0.02
Nodes (69): LightingCanvas, activeLayer_, additiveMarquee_, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, createdObject_ (+61 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 27 - "ARCHITECTURE.md"
Cohesion: 0.28
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "MSI Mystic Light - Lighting Studio"
Cohesion: 0.29
Nodes (7): Contents, MSI Mystic Light - Lighting Studio, Per-device LED arrangement, Preparing a Windows release, Related repos, Research documents, Standalone app implementation

### Community 30 - "ServicePanel"
Cohesion: 0.08
Nodes (21): command(), host(), serviceInventory(), ServicePanel, active_, changedAt_, clockDirty_, commands_ (+13 more)

### Community 31 - "OutputPanel.cpp"
Cohesion: 0.27
Nodes (3): channelKey(), OutputPanel::OutputPanel(), stateName()

### Community 33 - "EffectLayer"
Cohesion: 0.04
Nodes (44): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+36 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (72): ColorEditor, ColorListEditor, LightingWindow, activeLayer_, adjustingPanels_, advanced_, allChannels_, animation_ (+64 more)

### Community 35 - "IHidDevice"
Cohesion: 0.08
Nodes (8): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, IHidDevice, FakeHidDevice, opened_, openSucceeds, sendSucceeds, sentReports

### Community 36 - "EffectRenderer.cpp"
Cohesion: 0.27
Nodes (10): interpolate(), isGradient(), periodic(), polygon(), renderAtTime(), renderBaseEffect(), renderClippedBaseEffect(), renderMask() (+2 more)

### Community 37 - "Lighting editor"
Cohesion: 0.05
Nodes (41): Addressable LEDs and NV5 MKII evidence, Artwork alignment and custom device geometry, Canvas playback, fullscreen and growing Layout, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Detailed hardware artwork (+33 more)

### Community 38 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 39 - "ServiceRuntime"
Cohesion: 0.05
Nodes (23): ServiceRuntime, catalog_, clock_, connect_, connections_, directory_, document_, enabled_ (+15 more)

### Community 41 - "ConflictProcess"
Cohesion: 0.17
Nodes (7): ConflictProcess, executable, manualStop, path, pid, reason, started

### Community 42 - "HardwareArtwork.cpp"
Cohesion: 0.21
Nodes (7): drawHardwareArtwork(), drawHardwareComponentArtwork(), hardwareArtworkCategory(), hardwareBodyBounds(), hardwarePreviewLedRadius(), initializeHardwareArtwork(), productIcon()

### Community 43 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 44 - "test_output_ui.cpp"
Cohesion: 0.22
Nodes (8): previewFrame_, configure(), connection(), identity(), OutputUiTest, QTemporaryDir, profile(), state()

### Community 45 - "Effect editor interaction research"
Cohesion: 0.25
Nodes (8): Acceptance and implementation gates, Effect editor interaction research, Findings in current source, Interaction contract, Interface and identity, Recommended content model, Reference patterns, Scope and evidence

### Community 46 - "ConflictDialog"
Cohesion: 0.19
Nodes (10): ConflictDialog, busy_, conflicts_, ignore_, status_, table_, terminateAll_, QLabel (+2 more)

### Community 47 - "GeometryView"
Cohesion: 0.13
Nodes (6): GeometryView, artwork, commit, moved, pressPosition, scale

### Community 48 - "OutputSession"
Cohesion: 0.15
Nodes (7): OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 51 - "Lighting Studio: whole-interface and unified-editor review"
Cohesion: 0.18
Nodes (10): Editor interaction contract, Elevation and stable hardware inventory, Evidence and limits, Findings and priorities, Implementation order and acceptance, Layers, colors, numeric controls and appearance, Lighting Studio: whole-interface and unified-editor review, Main decision (+2 more)

### Community 52 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 53 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 54 - "RecordingTransport"
Cohesion: 0.14
Nodes (5): PairedTransport, control, leds, RecordingTransport, reports_

### Community 55 - "ComponentPlacement"
Cohesion: 0.20
Nodes (6): ComponentPlacement, componentId, id, locked, origin, rotation

### Community 56 - ".latestDuringSlowWrite"
Cohesion: 0.23
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - "test_output.cpp"
Cohesion: 0.24
Nodes (5): frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 58 - ".render"
Cohesion: 0.17
Nodes (6): channelAssignmentJson(), parseChannelAssignment(), validateChannelAssignments(), isParticle(), renderEffect(), layer()

### Community 61 - "EneRamProtocol"
Cohesion: 0.10
Nodes (6): EneRamProtocol, address_, transfer_, channelMapping(), ComponentsTest, AuxiliaryTest

### Community 63 - "PaletteStrip"
Cohesion: 0.15
Nodes (11): PaletteStrip, add, colors, dragging, end, gradient, move, positions (+3 more)

### Community 64 - "package_release.py"
Cohesion: 0.09
Nodes (4): package(), promote(), ReleasePackageTest, ScreenshotPromotionTest

### Community 65 - "impl_"
Cohesion: 0.29
Nodes (6): impl_, address, error, library, ready, sent

### Community 66 - "I2cInfo"
Cohesion: 0.18
Nodes (11): I2cInfo, address, ddc, displayMask, port, portSet, reg, regSize (+3 more)

### Community 67 - "OutputScheduler"
Cohesion: 0.14
Nodes (7): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "FakeProcesses"
Cohesion: 0.15
Nodes (9): FakeProcesses, denied, exitScanDelay, failScan, forced, graceful, pendingExits, restart (+1 more)

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - "State"
Cohesion: 0.20
Nodes (8): Status, error, generation, state, State, Connected, Disconnected, Faulted

### Community 73 - "src/main.cpp"
Cohesion: 0.11
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 74 - "Unified Editor implementation"
Cohesion: 0.40
Nodes (5): Project compatibility, Rendering, color and numeric behavior, Unified Editor implementation, Verification, Workspace and interaction

### Community 77 - "MaskTool"
Cohesion: 0.50
Nodes (3): MaskTool, Brush, Gradient

### Community 78 - "StudioWidgets.cpp"
Cohesion: 0.13
Nodes (7): openStudioColorPicker(), ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), disclosure(), begin, studioMotionEnabled(), studioToolIcon()

### Community 79 - "LightingWindow.cpp"
Cohesion: 0.09
Nodes (5): isShape(), button(), LightingWindow::LightingWindow(), scan_, QTreeWidgetItem

### Community 80 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 81 - "EffectKind"
Cohesion: 0.09
Nodes (23): EffectKind, Burst, Circle, ConicalGradient, Cross, Diamond, Fireflies, Heart (+15 more)

### Community 82 - "SaturationValueField"
Cohesion: 0.08
Nodes (12): ColorPopup, finished, gestureBegun, sampling, SaturationValueField, brightness, changed, hue (+4 more)

### Community 83 - "LightingDocument"
Cohesion: 0.09
Nodes (14): validateEffect(), LightingDocument, colorPreviewBefore_, componentNames_, frameCache_, frameRevision_, frameSeconds_, initialCatalog_ (+6 more)

### Community 84 - "EffectParameter"
Cohesion: 0.32
Nodes (6): EffectParameter, EffectLayer, id, label, maximum, minimum

### Community 85 - "ConflictProcesses.cpp"
Cohesion: 0.30
Nodes (8): conflictReason(), creationTime(), matchesProcessIdentity(), normalized(), processPath(), requiresManualStop(), SystemConflictProcesses, winError()

### Community 87 - ".edit"
Cohesion: 0.17
Nodes (4): loadComponentCatalog(), loadComponentDefinition(), state_, applyStudioStyle()

### Community 88 - ".pushColors"
Cohesion: 0.08
Nodes (13): Task 10: Roccat Kone Aimo driver, feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport() (+5 more)

### Community 89 - ".mousePressEvent"
Cohesion: 0.15
Nodes (3): effectTransform(), centeredGradient(), gradientEndpoints()

### Community 90 - ".load"
Cohesion: 0.12
Nodes (5): main(), fail(), ModuleRegistry, entries_, ModulesTest

### Community 91 - "QJsonObject"
Cohesion: 0.09
Nodes (14): ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), requiredText(), DeviceProfile::fromJson(), DeviceProfile::load(), integer(), text() (+6 more)

### Community 92 - "ProfileCandidate"
Cohesion: 0.11
Nodes (9): BoardIdentity, manufacturer, model, findProfileCandidates(), ProfileCandidate, device, profileId, readBoardIdentity() (+1 more)

### Community 93 - "test_service.cpp"
Cohesion: 0.08
Nodes (12): prepareOutput(), key(), ServiceRuntime::ServiceRuntime(), writeFile(), Child, process, cmd(), identity() (+4 more)

### Community 96 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.14
Nodes (14): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Endpoint list correction and restored backends (2026-10-24), Fixed optional component captions (2026-10-26), Gaming lighting workspace review implementation (2026-10-26), Gradient/output feedback correction (2026-10-24) (+6 more)

### Community 102 - "Lighting Studio GUI and interaction audit"
Cohesion: 0.18
Nodes (10): Color picker, Consolidation, Evidence, Findings, ordered by impact, Identity and direction, Implementation follow-up, Implementation order and acceptance, Integer scales (+2 more)

### Community 104 - "hardwareCategory"
Cohesion: 0.21
Nodes (7): hardwareCategory(), hardwareManufacturer(), hardwareName(), generic(), HardwarePicker::HardwarePicker(), installedProducts(), isFan()

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 106 - ".connect"
Cohesion: 0.21
Nodes (3): nativeKey(), sameOutputScope(), validateFrame()

### Community 107 - "Hardware modules"
Cohesion: 0.14
Nodes (13): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, LED discovery (+5 more)

### Community 108 - "QPainter"
Cohesion: 0.24
Nodes (3): SnapGrid, enabled, spacing

### Community 109 - "OutputFrame"
Cohesion: 0.16
Nodes (10): previewOutputReports(), ChannelOutput, channelId, colors, native, software, OutputFrame, channels (+2 more)

### Community 110 - "ServiceProtocol.cpp"
Cohesion: 0.17
Nodes (12): authorizedClient(), currentUserSid(), decode(), encode(), installedHost(), isSystem(), pipeName(), processSid() (+4 more)

### Community 112 - "ChannelAssignment"
Cohesion: 0.12
Nodes (13): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+5 more)

### Community 113 - "service/main.cpp"
Cohesion: 0.14
Nodes (8): control(), knownFolder(), main(), readJson(), report(), run(), serviceMain(), verifyInstall()

### Community 114 - "CONTINUE.md"
Cohesion: 0.20
Nodes (9): Dense strip SVG visibility regression (2026-10-26), Editor interaction and channel preview follow-up (2026-10-27), Fixed optional component captions (2026-10-26), Gaming lighting workspace implementation (2026-10-26), GUI audit implementation and selected branding (2026-10-26), Layout presentation and playback fixes (2026-10-26), Termination verification and per-screen eyedropper hotfix (2026-10-27), Unified Editor UI audit implementation (2026-10-27) (+1 more)

### Community 115 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 119 - "ChannelEditor::ChannelEditor"
Cohesion: 0.14
Nodes (8): ChannelEditor::ChannelEditor(), ComponentEditor::ComponentEditor(), installParameterTooltips(), sliderField(), sliderPosition(), StudioBanner, artwork_, syncSliderFields()

### Community 120 - "HidDescriptor"
Cohesion: 0.12
Nodes (12): deviceIdentity(), HidDescriptor, interfaceNumber, manufacturer, path, product, productId, usage (+4 more)

### Community 121 - "Editor interaction implementation"
Cohesion: 0.22
Nodes (6): Editor interaction implementation, Identity and color sampling, Persistence and limits, Pointer behavior, Rendering and output status, Result

### Community 122 - "Transport"
Cohesion: 0.14
Nodes (6): Recording, present, writes, Transport, opened, record

### Community 124 - "test_hid_backend.cpp"
Cohesion: 0.23
Nodes (6): HID_API_CALL hid_close(), HID_API_CALL hid_error(), HID_API_CALL hid_free_enumeration(), HID_API_CALL hid_send_feature_report(), HID_API_CALL hid_write(), hid_device_

### Community 125 - "BuildApex3ColorsPacket"
Cohesion: 0.36
Nodes (3): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), BuildApex3BrightnessPacket(), BuildApex3ColorsPacket()

### Community 126 - "ComponentEditor"
Cohesion: 0.22
Nodes (4): ComponentEditor, category_, manufacturer_, nonPhysicalSlots_

### Community 130 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

### Community 131 - "Tool"
Cohesion: 0.18
Nodes (9): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pencil (+1 more)

### Community 134 - "Output session backend"
Cohesion: 0.33
Nodes (6): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification

### Community 135 - "PawnIO integration and service feasibility"
Cohesion: 0.12
Nodes (15): Access control and process boundary, Conclusion, Coverage and next gate, Exact evidence, Installation and licensing boundary, PawnIO integration and service feasibility, Performance and recovery, Primary sources (+7 more)

### Community 137 - "authorizeHardwareOutput"
Cohesion: 0.11
Nodes (11): scanAuxiliaryHardware(), authorizeHardwareOutput(), loadModules(), OutputProof, controlIdentity, identity, initial, profile (+3 more)

### Community 138 - "EffectRenderer.h"
Cohesion: 0.25
Nodes (5): EffectDefinition, id, kind, label, EffectLayer

### Community 144 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

## Knowledge Gaps
- **841 isolated node(s):** `channel_`, `channels_`, `armed_`, `state_`, `shieldSent_` (+836 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1296 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **38 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `.render`, `src/main.cpp`, `PreviewDivider`, `test_output_ui.cpp`, `test_editor.cpp`, `OutputPanel.cpp`, `LightingWindow.cpp`, `OutputPanel`, `LightingDocument`, `.edit`, `LightingCanvas`, `.load`, `ProfileCandidate`, `test_service.cpp`, `ServicePanel`, `.addHeaderComponents`?**
  _High betweenness centrality (0.155) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `Output session backend` to `OutputScheduler`, `.connect`, `OutputFrame`, `LightingDocument`, `README.md`, `test_output.cpp`, `.load`?**
  _High betweenness centrality (0.105) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `IDriver`, `AuxiliaryTransport`, `MsiZoneSettings`, `NativeEffect`, `HidApiDevice`, `MsiDriver`, `Recording`, `Record`, `SignalRGB repository research`, `OutputSession`, `BuildReport0x51`, `RecordingTransport`, `Transport`, `stdexcept`, `.pushColors`, `.load`, `AuxiliaryDriver`, `PeripheralModule`, `msiprotocol`, `.connect`, `Hardware modules`, `VirtualModule`, `Transport`, `BuildApex3ColorsPacket`?**
  _High betweenness centrality (0.099) - this node is a cross-community bridge._
- **Are the 53 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 53 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channel_`, `channels_`, `armed_` to the rest of the system?**
  _841 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `IDriver` be split into smaller, more focused modules?**
  _Cohesion score 0.09782608695652174 - nodes in this community are weakly interconnected._
- **Should `MsiZoneSettings` be split into smaller, more focused modules?**
  _Cohesion score 0.0677361853832442 - nodes in this community are weakly interconnected._