# Graph Report - msi-mystic-light  (2026-11-11)

## Corpus Check
- 199 files · ~1,172,855 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 18 file(s) not represented in the graph (top: .csv 6, .bin 3, (none) 2)

## Summary
- 3214 nodes · 7064 edges · 163 communities (123 shown, 40 thin omitted)
- Extraction: 79% EXTRACTED · 21% INFERRED · 0% AMBIGUOUS · INFERRED: 1469 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `8771baf8`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- IDriver
- AuxiliaryTransport
- .render
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- stdexcept
- HpOmenDriver
- RgbColor
- MsiGen1AdvArea
- Development Tools
- LightingWindow.cpp
- MsiColor
- test_editor.cpp
- AeroBackend
- PaletteStrip
- NativeEffect
- PawnIoRam.cpp
- ServiceProtocol.cpp
- Recording
- AsrockPolychromeMode
- Implementation status
- README.md
- IDriverModule
- LightingCanvas
- authorizeHardwareOutput
- Mystic Light — Overall Architecture
- Reference repository reuse review
- MSI Mystic Light - Lighting Studio
- ServicePanel
- BuildReport0x51
- test_service.cpp
- EffectLayer
- LightingWindow
- AuxiliaryDriver
- EffectRenderer.cpp
- Lighting editor
- LenovoMotherboardModule.cpp
- ServiceRuntime
- OpenRGB SDK client — cross-platform (esp. Linux) hardware access
- ComponentEditor.cpp
- QToolButton
- OutputPanel
- Lighting Studio — Design Doc (v2)
- ConflictDialog
- GeometryView
- ServiceRuntime.cpp
- Record
- MsiZoneSettings
- Lighting Studio: whole-interface and unified-editor review
- DriverInstance
- MsiLightingMode
- PairedTransport
- ComponentPlacement
- .latestDuringSlowWrite
- .connect
- .edit
- Transport
- EneRamProtocol
- inventory_hardware.py
- PreviewDivider
- StartupTest
- OutputScheduler
- FakeHidDevice
- .start
- Windows RGB access investigation
- .status
- src/main.cpp
- HardwarePicker
- LightingCanvas.cpp
- Effect implementation checklist
- AsrockPolychromeZone
- LightingState
- EffectKind
- SaturationValueField
- LightingDocument
- .load
- ConflictProcesses.cpp
- ColorListEditor
- HidApiDevice
- .pushColors
- QPainter
- .setNativeMode_rejectsInvalidEffectsAndClosedDevice
- test_hp_omen.cpp
- OutputProof
- OutputFrame
- FakeProcesses
- Layer model proposal
- StudioWidgets.cpp
- IHidDevice
- VirtualModule
- Studio interaction and motion review
- Lighting Studio GUI and interaction audit
- Hardware modules
- Test review: recent lighting changes (2026-10-24)
- ComponentsTest
- ParameterSpinBox
- Ice workspace (GS-06)
- ConflictService
- Reliability and layer-design work (2026-11-11)
- PawnIO AMD SMBus module
- .setNativeMode_rejectsInvalidEffectsAndClosedDevice
- service/main.cpp
- CONTINUE.md
- AsrockPolychromeDriver
- LenovoMotherboardMode
- I2cInfo
- Transport
- SignalForge editor reference
- .create
- Effect editor interaction research
- Unified Editor implementation
- DocumentEdit
- hw_inventory_linux.sh
- LightingDocument.h
- colorIcon
- IConflictServices
- OutputSession
- closeWindow
- Tool
- SignalRGB repository research
- PawnIO integration and service feasibility
- State
- Lighting Studio editor UX review
- ServiceServer
- ChannelAssignment
- Lighting Studio branding
- ScCloser
- Hardware migration inventory and priorities
- Output session backend
- Third-party runtime components
- QJsonObject
- Connection
- ProfileCandidate
- HP Omen 30L migration: first independent-module increment
- EffectRenderer.h
- Hardware support strategy
- PeripheralModule
- EffectParameter
- moduleregistry
- Lighting Studio 0.1.0
- .cancelGesture

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 210 edges
2. `LightingCanvas` - 176 edges
3. `EditorTest` - 131 edges
4. `catalog()` - 84 edges
5. `document_` - 74 edges
6. `EffectLayer` - 71 edges
7. `LightingDocument` - 66 edges
8. `Implementation status` - 66 edges
9. `IHidDevice` - 59 edges
10. `EffectKind` - 53 edges

## Surprising Connections (you probably didn't know these)
- `Project compatibility` --references--> `locked()`  [INFERRED]
  docs/design/UNIFIED_EDITOR_IMPLEMENTATION_2026-10-27.md → src/backends/PawnIoRam.cpp
- `Task 10: Qt tray app shell` --references--> `QAction`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/editor/LightingCanvas.h
- `Problem confirmed in the current code` --references--> `LightingWindow`  [INFERRED]
  docs/LAYER_MODEL_PLAN.md → src/editor/LightingWindow.h
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Task 18: Final wire-up and real-hardware verification` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → src/hid/IHidDevice.h

## Import Cycles
- None detected.

## Communities (163 total, 40 thin omitted)

### Community 0 - "IDriver"
Cohesion: 0.08
Nodes (21): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+13 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.13
Nodes (6): AuxiliaryTransport, gpu_, ram_, ready_, openAuxiliaryTransport(), ~Impl()

### Community 2 - ".render"
Cohesion: 0.16
Nodes (7): validateChannelAssignments(), canCreateEffect(), isPattern(), renderEffect(), validateEffect(), validate(), layer()

### Community 3 - "Mystic Light — local network interfaces"
Cohesion: 0.25
Nodes (8): 1. TCP 26820 — local HTTP "Web API" (`MLModule.dll:CWebAPI.cs`), 2. UDP 26822 — mobile app pairing (`CBaseServer.cs`/`CBaseFN.cs`), 3. UDP 26823 — "party sync" between multiple PCs (`CSyncServer.cs`/`CSyncFN.cs`), 4. UDP 61337 — SignalRGB bridge ("LEDKeeper 2" service), Live verification (2026-10-17): the whole MLModule network layer is dead code, Mystic Light — local network interfaces, Protocol (confirmed via code, extends the `Msi.js` reconstruction), Still open

### Community 4 - "Standalone RGB Lighting App — Design"
Cohesion: 0.15
Nodes (13): Accepted effect authoring scope (2026-10-23), Anti-goals, Constraints, Goals, Hardware module architecture (2026-10-23), How it gets made (decided), Intent and why, Non-goals (v1) (+5 more)

### Community 5 - "MSI Mainboard LED Protocol"
Cohesion: 0.22
Nodes (9): MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`), Record layout (16 bytes, per zone/"area"), Report `0x50` ("Gen1"), 290 bytes — status and default configuration, Report `0x51` ("Gen1_Adv"), 727 bytes — per-LED addressing of individual segments, Report `0x90`+port / `0xB0`+port ("Gen2"/"Gen2_Adv") — newer strip model, ⚠️ Safety-relevant finding: the `Store` flag is set on every write, Upstream OpenRGB bug found while cross-checking: `FeaturePacket_PerLED_761` is 727 bytes, not 761, Verification against the existing capture template (+1 more)

### Community 6 - "stdexcept"
Cohesion: 0.12
Nodes (3): ComponentDefinition::toJson(), channelAssignmentJson(), parseChannelAssignment()

### Community 8 - "RgbColor"
Cohesion: 0.08
Nodes (12): Responsibilities, MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_, MsiModule (+4 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "LightingWindow.cpp"
Cohesion: 0.06
Nodes (10): spin, button(), LayerTree, reordered, LightingWindow::LightingWindow(), scan_, QTreeWidgetItem, setEffectControlRange() (+2 more)

### Community 12 - "MsiColor"
Cohesion: 0.11
Nodes (19): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness (+11 more)

### Community 13 - "test_editor.cpp"
Cohesion: 0.07
Nodes (6): document_, addEffect(), artifactDirectory(), catalog(), EditorTest, QTemporaryDir

### Community 14 - "AeroBackend"
Cohesion: 0.12
Nodes (14): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+6 more)

### Community 15 - "PaletteStrip"
Cohesion: 0.17
Nodes (12): PaletteStrip, add, begin, colors, dragging, end, gradient, move (+4 more)

### Community 16 - "NativeEffect"
Cohesion: 0.11
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.11
Nodes (15): call(), libraryPath(), locked(), onPrimary(), PawnIoRam, impl_, address, error (+7 more)

### Community 18 - "ServiceProtocol.cpp"
Cohesion: 0.22
Nodes (8): authorizedClient(), currentUserSid(), installedHost(), isSystem(), pipeName(), processSid(), tokenSid(), trustedServer()

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "AsrockPolychromeMode"
Cohesion: 0.13
Nodes (15): AsrockPolychromeMode, Breathing, Cram, Music, Neon, Off, Rainbow, Random (+7 more)

### Community 21 - "Implementation status"
Cohesion: 0.03
Nodes (64): Accepted effect authoring follow-ups (2026-10-23), Aero visibility and build cleanup (2026-10-25), Artwork geometry and frame regression fixes (2026-10-25), Canvas scrollbars and right-hand minimap (2026-10-24), Canvas Start/Stop, fullscreen and growing Layout (2026-10-24), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23) (+56 more)

### Community 22 - "README.md"
Cohesion: 0.14
Nodes (4): Before and after, Lightning and Fireworks visual review (2026-10-29), Sources and observation limits, Entwurf: Kommentar für GitLab-Issues #4935 / #5429

### Community 23 - "IDriverModule"
Cohesion: 0.08
Nodes (5): AuxiliaryModule, HpOmenModule, IDriverModule, IncompatibleModule, MissingInterface

### Community 24 - "LightingCanvas"
Cohesion: 0.02
Nodes (71): LightingCanvas, activeLayer_, additiveMarquee_, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, createdObject_ (+63 more)

### Community 25 - "authorizeHardwareOutput"
Cohesion: 0.18
Nodes (5): authorizeHardwareOutput(), loadModules(), previewOutputReports(), RecordingTransport, reports_

### Community 27 - "Mystic Light — Overall Architecture"
Cohesion: 0.14
Nodes (12): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll, Content (+4 more)

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "MSI Mystic Light - Lighting Studio"
Cohesion: 0.25
Nodes (8): Contents, MSI Mystic Light - Lighting Studio, Per-device LED arrangement, Preparing a Windows release, Procedural control and visual refinement (GS-06), Related repos, Research documents, Standalone app implementation

### Community 30 - "ServicePanel"
Cohesion: 0.08
Nodes (21): command(), host(), serviceInventory(), ServicePanel, active_, changedAt_, clockDirty_, commands_ (+13 more)

### Community 31 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 32 - "test_service.cpp"
Cohesion: 0.14
Nodes (3): Child, process, ServiceTest::protocolBounds()

### Community 33 - "EffectLayer"
Cohesion: 0.03
Nodes (59): EffectLayer, allChannels, blend, breathe, center, channelTargets, color, colorCycle (+51 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (70): ColorEditor, ColorListEditor, LightingWindow, activeLayer_, adjustingPanels_, advanced_, allChannels_, animation_ (+62 more)

### Community 36 - "EffectRenderer.cpp"
Cohesion: 0.15
Nodes (16): effectTransform(), interpolate(), isGradient(), isParticle(), isShape(), noise2d(), periodic(), polygon() (+8 more)

### Community 37 - "Lighting editor"
Cohesion: 0.04
Nodes (47): Addressable LEDs and NV5 MKII evidence, Artwork alignment and custom device geometry, Artwork, continuous gestures and spatial controls (GS-06), Authoring controls follow-up, Canvas playback, fullscreen and growing Layout, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import (+39 more)

### Community 38 - "LenovoMotherboardModule.cpp"
Cohesion: 0.14
Nodes (6): LenovoMotherboardDriver, channels_, LenovoMotherboardModule, mapBrightness(), mapSpeed(), zoneFromAddress()

### Community 39 - "ServiceRuntime"
Cohesion: 0.06
Nodes (23): ServiceRuntime, catalog_, clock_, connect_, connections_, directory_, document_, enabled_ (+15 more)

### Community 41 - "OpenRGB SDK client — cross-platform (esp. Linux) hardware access"
Cohesion: 0.13
Nodes (13): LED discovery, Data model (`RGBControllerInterface.h`) — what the server actually exposes, Deeper pass (2026-10-29, same day): SignalRgb.exe native binary + OpenRGB current-master re-check, Full decompile, targeted (2026-10-29, same session), Handshake (`NetworkClient::ConnectionThreadFunction`, `NetworkClient.cpp:1707`), License note (not a legal determination), Mapping onto this app's architecture, OpenRGB current-master re-check (`build/research/openrgb`, `aac7e551`) (+5 more)

### Community 42 - "ComponentEditor.cpp"
Cohesion: 0.06
Nodes (5): ChannelEditor, fixedComponent_, initial_, CoordinateDelegate, QPainter

### Community 43 - "QToolButton"
Cohesion: 0.17
Nodes (6): QToolButton, studioThemeColor(), QToolBar, StudioHeaderToolbar, followHeaderRow_, StudioSectionButton

### Community 44 - "OutputPanel"
Cohesion: 0.05
Nodes (36): previewFrame_, channelKey(), OutputPanel, authorize_, authorizing_, automatic_, channelDetails_, configured_ (+28 more)

### Community 45 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 46 - "ConflictDialog"
Cohesion: 0.12
Nodes (14): ConflictDialog, adminServicePid_, adminStop_, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_ (+6 more)

### Community 47 - "GeometryView"
Cohesion: 0.09
Nodes (10): ComponentEditor, category_, manufacturer_, nonPhysicalSlots_, GeometryView, artwork, commit, moved (+2 more)

### Community 48 - "ServiceRuntime.cpp"
Cohesion: 0.13
Nodes (9): Scan, key(), ServiceRuntime::ServiceRuntime(), writeFile(), cmd(), identity(), project(), ServiceTest (+1 more)

### Community 49 - "Record"
Cohesion: 0.07
Nodes (21): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+13 more)

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

### Community 54 - "PairedTransport"
Cohesion: 0.22
Nodes (3): PairedTransport, control, leds

### Community 55 - "ComponentPlacement"
Cohesion: 0.17
Nodes (6): ComponentPlacement, componentId, id, locked, origin, rotation

### Community 56 - ".latestDuringSlowWrite"
Cohesion: 0.23
Nodes (7): assertClosedOnWorker(), factory(), frame(), identity(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 57 - ".connect"
Cohesion: 0.15
Nodes (7): nativeKey(), validateFrame(), frame(), identity(), OutputTest, OutputTest::latestFrameAndNativeCache(), private

### Community 58 - ".edit"
Cohesion: 0.14
Nodes (5): loadComponentCatalog(), loadComponentDefinition(), state_, applyStudioStyle(), channelProfile()

### Community 61 - "EneRamProtocol"
Cohesion: 0.23
Nodes (4): EneRamProtocol, address_, transfer_, AuxiliaryTest

### Community 64 - "inventory_hardware.py"
Cohesion: 0.05
Nodes (13): arguments(), build(), git(), integer(), js_return(), priority(), registrations(), snapshot() (+5 more)

### Community 65 - "PreviewDivider"
Cohesion: 0.10
Nodes (3): LayerDelegate, PreviewDivider, PreviewSplitter

### Community 67 - "OutputScheduler"
Cohesion: 0.14
Nodes (7): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "FakeHidDevice"
Cohesion: 0.15
Nodes (11): Completed: HID transport (2026-10-23), FakeHidDevice, featureReportResponse, getFeatureReportCalls, inputReportResponse, opened_, openSucceeds, readInputReportCalls (+3 more)

### Community 71 - "Windows RGB access investigation"
Cohesion: 0.20
Nodes (9): Evidence and limitations, Findings, MSI deployment limitations, MSI NTIOLib follow-up, Recommendation, SignalIO driver follow-up: static dispatch analysis, SignalRGB protocol and driver evidence, SignPath follow-up (2026-10-24) (+1 more)

### Community 72 - ".status"
Cohesion: 0.25
Nodes (4): Status, error, generation, state

### Community 73 - "src/main.cpp"
Cohesion: 0.12
Nodes (4): AppSession, AppSession::AppSession(), recoveryFile_, timer_

### Community 74 - "HardwarePicker"
Cohesion: 0.05
Nodes (28): channelHasFixedComponent(), drawHardwareArtwork(), drawHardwareComponentArtwork(), hardwareArtworkCategory(), hardwareBodyBounds(), hardwarePreviewLedRadius(), initializeHardwareArtwork(), hardwareCategory() (+20 more)

### Community 78 - "Effect implementation checklist"
Cohesion: 0.22
Nodes (9): Additional native patterns, Additional native source batch, Capability gate and particle follow-up, Editing follow-up, Effect implementation checklist, First implementation batch, Second batch and color picker, Second batch and remaining candidates (+1 more)

### Community 79 - "AsrockPolychromeZone"
Cohesion: 0.13
Nodes (10): AsrockPolychromeZone, Argb1, Argb2, Audio, IoCover, Pcb, Pch, Rgb1 (+2 more)

### Community 80 - "LightingState"
Cohesion: 0.29
Nodes (5): LightingState, assignments, components, layers, placements

### Community 81 - "EffectKind"
Cohesion: 0.05
Nodes (41): EffectKind, Aurora, Burst, Checkerboard, Circle, ConicalGradient, Cross, Diamond (+33 more)

### Community 82 - "SaturationValueField"
Cohesion: 0.08
Nodes (12): ColorPopup, finished, gestureBegun, sampling, SaturationValueField, brightness, changed, hue (+4 more)

### Community 83 - "LightingDocument"
Cohesion: 0.08
Nodes (13): LightingDocument, colorPreviewBefore_, componentNames_, frameCache_, frameRevision_, frameSeconds_, initialCatalog_, LightingDocument::LightingDocument() (+5 more)

### Community 84 - ".load"
Cohesion: 0.22
Nodes (4): main(), fail(), ModuleRegistry, entries_

### Community 85 - "ConflictProcesses.cpp"
Cohesion: 0.16
Nodes (16): conflictReason(), creationTime(), matchesProcessIdentity(), NativeServices, manager_, verifiedPid_, normalized(), processPath() (+8 more)

### Community 86 - "ColorListEditor"
Cohesion: 0.06
Nodes (23): Component catalog schema 1, Task 5: `BreathingEffect`, Task 7: `CometEffect` and `MarqueeEffect`, ColorEditor, hex_, picker_, public, ColorListEditor (+15 more)

### Community 87 - "HidApiDevice"
Cohesion: 0.06
Nodes (26): Resolved: `IHidDevice` read capability added (API 3), hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer (+18 more)

### Community 88 - ".pushColors"
Cohesion: 0.09
Nodes (13): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport() (+5 more)

### Community 89 - "QPainter"
Cohesion: 0.24
Nodes (3): SnapGrid, enabled, spacing

### Community 90 - ".setNativeMode_rejectsInvalidEffectsAndClosedDevice"
Cohesion: 0.29
Nodes (3): AsrockPolychromeTest, identity(), profile()

### Community 91 - "test_hp_omen.cpp"
Cohesion: 0.22
Nodes (5): HpOmenTest, identity(), OutputOnlyHid, featureWrites, profile()

### Community 92 - "OutputProof"
Cohesion: 0.22
Nodes (6): OutputProof, controlIdentity, identity, initial, profile, reports

### Community 93 - "OutputFrame"
Cohesion: 0.12
Nodes (13): OutputConnection, factory, initial, present, ChannelOutput, channelId, colors, native (+5 more)

### Community 94 - "FakeProcesses"
Cohesion: 0.06
Nodes (25): FakeProcesses, adminActive, adminNeverStops, adminPendingPolls, adminPollError, adminQueryError, adminRequestError, adminRequests (+17 more)

### Community 95 - "Layer model proposal"
Cohesion: 0.20
Nodes (10): Acceptance matrix, Creation and selection contract, Delivery in reviewable increments, Explain the blends with a live result, Explicitly outside the first implementation, Feasibility evidence and resolved review points, Layer model proposal, Problem confirmed in the current code (+2 more)

### Community 96 - "StudioWidgets.cpp"
Cohesion: 0.08
Nodes (17): ChannelEditor::ChannelEditor(), openStudioColorPicker(), ComponentEditor::ComponentEditor(), ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), disclosure(), installParameterTooltips(), NumericFieldStyle (+9 more)

### Community 97 - "IHidDevice"
Cohesion: 0.10
Nodes (15): 162-byte MSI family — corroborated, not conflicting, ASRock Polychrome USB (`26CE:01A2`) — mostly confirmed, one open discrepancy, ASRock Polychrome USB — implemented (2026-10-29, same session), ENE SMBus (RAM) — register map confirmed 3-way, detection method rejected, Lenovo motherboard — implemented (2026-10-29, same session), Not yet cross-checked this pass, Open questions, Our board (`0DB0:0076`) — triple-confirmed, including the shared bug (+7 more)

### Community 100 - "Studio interaction and motion review"
Cohesion: 0.14
Nodes (14): Data and transport boundaries, Design principles checked, Direct group dragging and automatic output (2026-10-24), Drawing and hardware regression pass (2026-10-24), Endpoint list correction and restored backends (2026-10-24), Fixed optional component captions (2026-10-26), Gaming lighting workspace review implementation (2026-10-26), Gradient/output feedback correction (2026-10-24) (+6 more)

### Community 102 - "Lighting Studio GUI and interaction audit"
Cohesion: 0.18
Nodes (10): Color picker, Consolidation, Evidence, Findings, ordered by impact, Identity and direction, Implementation follow-up, Implementation order and acceptance, Integer scales (+2 more)

### Community 104 - "Hardware modules"
Cohesion: 0.17
Nodes (12): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, Native GPU endpoint (+4 more)

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 107 - "ParameterSpinBox"
Cohesion: 0.22
Nodes (4): maximum, minimum, ParameterSpinBox, scale_

### Community 108 - "Ice workspace (GS-06)"
Cohesion: 0.40
Nodes (5): Direction and native adaptation, GS-06 interaction and effect review, Ice workspace (GS-06), Native captures, Native critique

### Community 109 - "ConflictService"
Cohesion: 0.10
Nodes (15): ConflictProcess, executable, manualStop, path, pid, reason, started, ConflictService (+7 more)

### Community 110 - "Reliability and layer-design work (2026-11-11)"
Cohesion: 0.40
Nodes (5): Collaboration contract, Current status, Decisions and validation boundaries, Reliability and layer-design work (2026-11-11), Remaining acceptance gates

### Community 112 - ".setNativeMode_rejectsInvalidEffectsAndClosedDevice"
Cohesion: 0.27
Nodes (3): identity(), LenovoMotherboardTest, profile()

### Community 113 - "service/main.cpp"
Cohesion: 0.22
Nodes (8): control(), knownFolder(), main(), readJson(), report(), run(), serviceMain(), verifyInstall()

### Community 114 - "CONTINUE.md"
Cohesion: 0.07
Nodes (26): Active-layer picking, canvas labels and transport controls (2026-10-28), Authoring controls, pan and two patterns (2026-10-28), Components fullscreen and edit-mode visibility fix (2026-10-28), Dense strip SVG visibility regression (2026-10-26), Editor interaction and channel preview follow-up (2026-10-27), Editor UX, Pan crash and four sources (2026-10-28), Fixed optional component captions (2026-10-26), Gaming lighting workspace implementation (2026-10-26) (+18 more)

### Community 115 - "AsrockPolychromeDriver"
Cohesion: 0.08
Nodes (10): AsrockPolychromeDriver, channels_, state_, AsrockPolychromeModule, mapSpeed(), ZoneState, applied, color (+2 more)

### Community 116 - "LenovoMotherboardMode"
Cohesion: 0.12
Nodes (12): BuildLenovoMotherboardReports(), LenovoMotherboardMode, Breathing, RainbowWave, Random, Sparkle, Specter, Static (+4 more)

### Community 117 - "I2cInfo"
Cohesion: 0.18
Nodes (11): I2cInfo, address, ddc, displayMask, port, portSet, reg, regSize (+3 more)

### Community 118 - "Transport"
Cohesion: 0.13
Nodes (7): Recording, opens, present, writes, Transport, opened, record

### Community 119 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 121 - "Effect editor interaction research"
Cohesion: 0.12
Nodes (14): Editor interaction implementation, Identity and color sampling, Persistence and limits, Pointer behavior, Rendering and output status, Result, Acceptance and implementation gates, Effect editor interaction research (+6 more)

### Community 122 - "Unified Editor implementation"
Cohesion: 0.40
Nodes (5): Project compatibility, Rendering, color and numeric behavior, Unified Editor implementation, Verification, Workspace and interaction

### Community 125 - "LightingDocument.h"
Cohesion: 0.22
Nodes (6): LightingState, LayerBlend, Multiply, Normal, Overlay, Screen

### Community 129 - "OutputSession"
Cohesion: 0.12
Nodes (11): OutputSession, appliedNative_, driver_, identity_, pending_, profile_, transport_, State (+3 more)

### Community 131 - "Tool"
Cohesion: 0.17
Nodes (10): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pan (+2 more)

### Community 133 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 135 - "PawnIO integration and service feasibility"
Cohesion: 0.12
Nodes (15): Access control and process boundary, Conclusion, Coverage and next gate, Exact evidence, Installation and licensing boundary, PawnIO integration and service feasibility, Performance and recovery, Primary sources (+7 more)

### Community 137 - "State"
Cohesion: 0.40
Nodes (5): State, Other, Running, Stopped, StopPending

### Community 138 - "Lighting Studio editor UX review"
Cohesion: 0.15
Nodes (13): Additional procedural sources, Captures and measured checks, Crash evidence and validation boundaries, Creative-software research, Direction and scope, Findings and implementation, Gaming-Pro-Studio refinement, GS-06 finish (+5 more)

### Community 139 - "ServiceServer"
Cohesion: 0.17
Nodes (5): ServiceServer, clients_, handler_, owner_, ServiceServer::ServiceServer()

### Community 140 - "ChannelAssignment"
Cohesion: 0.17
Nodes (10): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+2 more)

### Community 142 - "Lighting Studio branding"
Cohesion: 0.50
Nodes (3): GS-06 presentation, Ice panorama (GS-05, 2026-10-29), Lighting Studio branding

### Community 150 - "Hardware migration inventory and priorities"
Cohesion: 0.22
Nodes (7): OpenRGB controller-family worklist, Browse the full list, Hardware migration inventory and priorities, Prioritized migration queue, Reproduction and next decisions, Scale and counting units, Source provenance and limitations

### Community 151 - "Output session backend"
Cohesion: 0.33
Nodes (6): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification

### Community 153 - "Third-party runtime components"
Cohesion: 0.50
Nodes (3): Hardware artwork, HP Omen 30L protocol module, Third-party runtime components

### Community 154 - "QJsonObject"
Cohesion: 0.13
Nodes (13): ComponentDefinition::fromJson(), integer(), requiredText(), DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text() (+5 more)

### Community 155 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 156 - "ProfileCandidate"
Cohesion: 0.09
Nodes (14): scanAuxiliaryHardware(), BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), ProfileCandidate, device (+6 more)

### Community 157 - "HP Omen 30L migration: first independent-module increment"
Cohesion: 0.50
Nodes (4): HP Omen 30L migration: first independent-module increment, Integration and qualification, Pinned reference comparison, Selected protocol and scope

### Community 160 - "EffectRenderer.h"
Cohesion: 0.18
Nodes (9): EffectDefinition, description, id, kind, label, particleCount, patternSites, patternSpeed (+1 more)

### Community 164 - "Hardware support strategy"
Cohesion: 0.33
Nodes (6): Evidence and qualification, Execution boundary, Hardware support strategy, Independent hardware modules, Installed manufacturer software and peripheral settings, Mandatory comparison of reference implementations

### Community 167 - "EffectParameter"
Cohesion: 0.25
Nodes (6): EffectParameter, EffectLayer, id, label, schemaVersion, practicalEffectRange()

### Community 169 - "Lighting Studio 0.1.0"
Cohesion: 0.50
Nodes (3): Changes, Lighting Studio 0.1.0, Requirements and limitations

### Community 171 - ".cancelGesture"
Cohesion: 0.11
Nodes (3): MaskTool, Brush, Gradient

## Knowledge Gaps
- **1045 isolated node(s):** `mode`, `speed`, `color`, `applied`, `channels_` (+1040 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1574 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **40 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `test_service.cpp`, `.render`, `EffectRenderer.cpp`, `src/main.cpp`, `HardwarePicker`, `LightingWindow.cpp`, `OutputPanel`, `test_editor.cpp`, `LightingDocument`, `.load`, `LightingCanvas`, `.edit`, `ProfileCandidate`, `ServicePanel`, `Layer model proposal`?**
  _High betweenness centrality (0.114) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `IDriver`, `AuxiliaryTransport`, `OutputSession`, `SignalRGB repository research`, `stdexcept`, `HpOmenDriver`, `RgbColor`, `MsiColor`, `NativeEffect`, `Recording`, `IDriverModule`, `authorizeHardwareOutput`, `BuildReport0x51`, `AuxiliaryDriver`, `LenovoMotherboardModule.cpp`, `PeripheralModule`, `OutputPanel`, `PairedTransport`, `.connect`, `Transport`, `FakeHidDevice`, `HidApiDevice`, `.pushColors`, `msiprotocol`, `VirtualModule`, `Hardware modules`, `AsrockPolychromeDriver`, `Transport`, `.create`?**
  _High betweenness centrality (0.111) - this node is a cross-community bridge._
- **Why does `Output session backend` connect `Output session backend` to `OutputScheduler`, `LightingDocument`, `.load`, `README.md`, `.connect`, `OutputFrame`?**
  _High betweenness centrality (0.102) - this node is a cross-community bridge._
- **Are the 77 inferred relationships involving `LightingWindow` (e.g. with `Problem confirmed in the current code` and `main()`) actually correct?**
  _`LightingWindow` has 77 INFERRED edges - model-reasoned connections that need verification._
- **Are the 5 inferred relationships involving `catalog()` (e.g. with `.componentName()` and `.componentPoints()`) actually correct?**
  _`catalog()` has 5 INFERRED edges - model-reasoned connections that need verification._
- **Are the 73 inferred relationships involving `document_` (e.g. with `main()` and `addEffect()`) actually correct?**
  _`document_` has 73 INFERRED edges - model-reasoned connections that need verification._
- **What connects `mode`, `speed`, `color` to the rest of the system?**
  _1045 weakly-connected nodes found - possible documentation gaps or missing edges._