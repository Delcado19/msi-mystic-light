# Graph Report - msi-mystic-light  (2026-11-18)

## Corpus Check
- 268 files · ~1,862,157 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 23 file(s) not represented in the graph (top: .csv 6, .cmd 4, (none) 3)

## Summary
- 4877 nodes · 11130 edges · 234 communities (185 shown, 49 thin omitted)
- Extraction: 83% EXTRACTED · 17% INFERRED · 0% AMBIGUOUS · INFERRED: 1863 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `8ddf7d96`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- live-browser.js
- AuxiliaryTransport
- Implementation status
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- connectSSE
- setLiveState
- MsiDriver
- MsiZoneSettings
- Development Tools
- LightingWindow.cpp
- stdexcept
- test_editor.cpp
- el
- modern-screenshot.umd.js
- initPageChat
- PawnIoRam.cpp
- initGlobalBar
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
- Lighting Studio
- ServicePanel.cpp
- updatePendingCounter
- OutputPanel
- EffectLayer
- LightingWindow
- OutputFrame
- EffectRenderer.cpp
- Lighting editor
- LenovoMotherboardModule.cpp
- ServiceRuntime
- IDriver
- ComponentEditor.cpp
- IHidDevice
- test_output_ui.cpp
- Lighting Studio — Design Doc (v2)
- ConflictDialog
- GeometryView
- IConflictProcesses
- Record
- live.md
- Lighting Studio: whole-interface and unified-editor review
- DriverInstance
- new-work.md
- RecordingTransport
- LightingState
- test_scheduler.cpp
- .connect
- HidDescriptor
- Transport
- EneRamProtocol
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
- VirtualModule
- SKILL.md
- test_service.cpp
- PaletteStrip
- EffectKind
- SaturationValueField
- showToast
- ChannelAssignment
- ConflictProcesses.cpp
- mountSvelteComponentVariant
- adapt.md
- .pushColors
- ServiceServer
- Layer visibility, brush size and Delete
- document.md
- AeroBackend
- .cancelGesture
- FakeProcesses
- Layer model proposal
- QMap
- HardwarePicker
- array
- IDriverModule.h
- LightingDocument
- Lighting Studio GUI and interaction audit
- onboard.md
- Test review: recent lighting changes (2026-10-24)
- The Toolkit
- Gradient travel and blend verification (2026-10-24)
- Lightning and Fireworks visual review (2026-10-29)
- ServiceProtocol.cpp
- main
- PawnIO AMD SMBus module
- actOnAgentTarget
- service/main.cpp
- ServiceServer.cpp
- AsrockPolychromeDriver
- createLiveBrowserDomHelpers
- StudioWidgets.cpp
- createLiveBrowserSessionState
- UsbWrite
- Record
- PreviewDivider
- Studio interaction and motion review
- DocumentEdit
- hw_inventory_linux.sh
- DeviceConflicts.cpp
- LightingCanvas.cpp
- .mouseMoveEvent
- ComponentDefinition
- closeWindow
- Tool
- .load
- Connection
- PawnIO integration and service feasibility
- CONTINUE.md
- Lighting Studio editor UX review
- closeWindow
- onAnnotDown
- branding/README.md
- JARGB2 physical response tests - 2026-11-13 / 2026-11-14
- ConflictProcess
- Hardware migration inventory and priorities
- OutputConnection
- Impeccable Asset Producer
- animate.md
- HardwareArtwork.cpp
- Generate Report
- README.md
- audit.native.md
- ParameterSpinBox
- test_hid_backend.cpp
- Uninstall-LightingStudio.ps1
- Handle `generate`
- MsiGen1AdvArea
- OutputPanel.cpp
- LightingCanvas.h
- PeripheralModule
- Workspace navigation and catalog identity
- Hardware support strategy
- Output session backend
- Artwork, continuous gestures and spatial controls (GS-06)
- ModulesTest
- optimize.md
- Distinct source capabilities and particle authoring
- critique.md
- Project files and boundaries
- Simplify the Design
- Hardening Dimensions
- clarify.md
- ChannelEditor
- QPainter
- ScCloser
- HidApiDevice
- Effect editor interaction research
- Nielsen's 10 Heuristics
- polish.md
- LightingDocument.h
- quieter.md
- EffectParameter
- Responsive Design
- Generate Combined Critique Report
- Init flow
- Common Cognitive Load Violations
- iOS platform
- Operate mode depth (and Read notes)
- Shape
- Android platform
- Persona-Based Design Testing
- Hardware modules
- Extract Flow
- Capture
- Impeccable Finish Reviewer
- Impeccable Manual Edit Applier
- Cognitive Load Assessment
- Impeccable Finish Reviewer
- Impeccable Manual Edit Applier
- live-browser-ignores.js
- Reliability and layer-design work (2026-11-11)
- Visualize: Direction Comps & Asset Production
- Impeccable Documenter
- `Mystic Light Online Data.dat` — decryption and content
- Heuristics Scoring Guide
- RgbColor
- doctor.md
- Review Focus
- Impeccable Documenter
- I2cInfo
- EffectRenderer.h
- Effect implementation checklist
- ScanMailbox
- SignalForge editor reference
- LayerBlend
- studioToolIcon
- impl_
- IConflictServices
- Lighting Studio 0.1.0
- DeviceProfile.cpp
- Child
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
- `Task 18: Final wire-up and real-hardware verification` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → src/hid/IHidDevice.h
- `Output scheduler` --references--> `OutputScheduler`  [INFERRED]
  docs/OUTPUT_SESSION.md → src/output/OutputScheduler.h
- `Task 7: `CometEffect` and `MarqueeEffect`` --references--> `x()`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → .github/skills/impeccable/scripts/modern-screenshot.umd.js

## Import Cycles
- None detected.

## Communities (234 total, 49 thin omitted)

### Community 0 - "live-browser.js"
Cohesion: 0.03
Nodes (128): absolutizeUrls(), applyGlobalBarLabelState(), applyOriginalAttrsToSvelteAnchor(), applyPlaceholderSizingStyles(), averageRgb01(), bindEditBadgeProxy(), bufferToBase64(), buildCollapsible() (+120 more)

### Community 1 - "AuxiliaryTransport"
Cohesion: 0.15
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

### Community 6 - "connectSSE"
Cohesion: 0.06
Nodes (87): abortSvelteComponentInjection(), applyParamDefaults(), applyParamValue(), applySavedParamValues(), applySavedSessionMeta(), buildParamsPanel(), clampVariantIndex(), clearHandled() (+79 more)

### Community 7 - "setLiveState"
Cohesion: 0.07
Nodes (77): applyEditing(), beginNewLiveConfiguration(), buildInsertPlaceholderSnapshotFromDom(), buildLocatorForLeaf(), buildPickedAnchorSnapshot(), cancelEditing(), cancelEditingToPicking(), cancelInsertConfigure() (+69 more)

### Community 8 - "MsiDriver"
Cohesion: 0.19
Nodes (6): MsiDriver, armed_, channels_, shieldColor_, shieldSent_, state_

### Community 9 - "MsiZoneSettings"
Cohesion: 0.05
Nodes (42): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness, Task 3: HID transport interface and hidapi backend (+34 more)

### Community 11 - "LightingWindow.cpp"
Cohesion: 0.05
Nodes (12): Task 4: Menus are available where they act, once, spin, button(), colorIcon(), LayerTree, reordered, LightingWindow::LightingWindow(), scan_ (+4 more)

### Community 13 - "test_editor.cpp"
Cohesion: 0.06
Nodes (7): Task 1: Gradient stop colors survive Apply, document_, addEffect(), artifactDirectory(), catalog(), EditorTest, QTemporaryDir

### Community 14 - "el"
Cohesion: 0.07
Nodes (57): actionLabel(), applyConfigureBarChrome(), bindConfigureCountPillTooltip(), bindConfigureInlineControlHover(), bindConfigureModifierPillHover(), buildConfigureActionControl(), buildConfigureCountControl(), buildConfigureRow() (+49 more)

### Community 15 - "modern-screenshot.umd.js"
Cohesion: 0.09
Nodes (55): ae(), be(), bt(), Ce(), s(), Ct(), de(), dt() (+47 more)

### Community 16 - "initPageChat"
Cohesion: 0.07
Nodes (55): agentHasWorkInFlight(), armPageChatForTyping(), attachSteerFocusDebug(), attachSteerFocusGuard(), buildSteerProcessingDots(), buildSteerQueueHint(), clearSteerAwaitTimer(), clearSteerFocusRecoverTimer() (+47 more)

### Community 17 - "PawnIoRam.cpp"
Cohesion: 0.17
Nodes (10): call(), ~Impl(), libraryPath(), locked(), onPrimary(), PawnIoRam, error, pawnIoRamDiscoveryError() (+2 more)

### Community 18 - "initGlobalBar"
Cohesion: 0.08
Nodes (46): agentStatusText(), applyLiveBarPreference(), barPaletteForTheme(), brandMarkSvg(), buildDesignHeader(), cursorForInsertAxis(), designPanelCss(), detectPageTheme() (+38 more)

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
Cohesion: 0.10
Nodes (13): Evidence, Ice workspace - GS-06-R2, Project compatibility, Rendering, color and numeric behavior, Unified Editor implementation, Verification, Workspace and interaction, Behavioral evidence (+5 more)

### Community 23 - "IDriverModule"
Cohesion: 0.06
Nodes (6): AuxiliaryModule, HpOmenModule, MsiModule, IDriverModule, IncompatibleModule, MissingInterface

### Community 24 - "LightingCanvas"
Cohesion: 0.02
Nodes (76): LightingCanvas, activeLayer_, additiveMarquee_, autoFit_, autoScroll_, autoScrollClock_, autoScrollSpeed_, componentSeconds_ (+68 more)

### Community 25 - ".edit"
Cohesion: 0.12
Nodes (11): parseChannelAssignment(), canCreateEffect(), isParticle(), isPattern(), isShape(), renderEffect(), validateEffect(), state_ (+3 more)

### Community 26 - "Transport"
Cohesion: 0.12
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
Cohesion: 0.06
Nodes (28): command(), host(), ServiceConflictProcesses, elevate_, rpc_, ServiceConflictProcesses::ServiceConflictProcesses(), serviceInventory(), ServicePanel (+20 more)

### Community 31 - "updatePendingCounter"
Cohesion: 0.08
Nodes (46): addManualContextText(), canRestoreManualEditElement(), clearStoredManualApplyState(), collectManualContextPieces(), walk(), contextElementForManualEdit(), cssIdent(), directMixedTextRestoreNodes() (+38 more)

### Community 32 - "OutputPanel"
Cohesion: 0.08
Nodes (17): OutputPanel, authorize_, authorizing_, automatic_, channelDetails_, configured_, connections_, endpoints_ (+9 more)

### Community 33 - "EffectLayer"
Cohesion: 0.03
Nodes (58): EffectLayer, allChannels, breathe, center, channelTargets, color, colorCycle, colorCyclePositions (+50 more)

### Community 34 - "LightingWindow"
Cohesion: 0.03
Nodes (71): ColorEditor, ColorListEditor, LightingWindow, activeLayer_, adjustingPanels_, advanced_, allChannels_, animation_ (+63 more)

### Community 35 - "OutputFrame"
Cohesion: 0.15
Nodes (11): buildFrames(), ChannelOutput, channelId, colors, native, software, OutputFrame, channels (+3 more)

### Community 36 - "EffectRenderer.cpp"
Cohesion: 0.19
Nodes (13): effectTransform(), interpolate(), isGradient(), noise2d(), periodic(), polygon(), renderAtTime(), renderBaseEffect() (+5 more)

### Community 37 - "Lighting editor"
Cohesion: 0.09
Nodes (23): Authoring controls follow-up, Channel targets and clocks (schema 9), Colors, paint sources and gradient stops (schema 11), Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Integer controls, live color and identity (2026-10-26) (+15 more)

### Community 38 - "LenovoMotherboardModule.cpp"
Cohesion: 0.07
Nodes (18): LenovoMotherboardDriver, channels_, LenovoMotherboardModule, mapBrightness(), mapSpeed(), zoneFromAddress(), BuildLenovoMotherboardReports(), LenovoMotherboardMode (+10 more)

### Community 39 - "ServiceRuntime"
Cohesion: 0.04
Nodes (43): Retired, endpoint, scheduler, ScanResult, blocked, error, guarded, inventory (+35 more)

### Community 41 - "IDriver"
Cohesion: 0.08
Nodes (21): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+13 more)

### Community 43 - "IHidDevice"
Cohesion: 0.06
Nodes (24): 162-byte MSI family — corroborated, not conflicting, ASRock Polychrome USB (`26CE:01A2`) — mostly confirmed, one open discrepancy, ASRock Polychrome USB — implemented (2026-10-29, same session), ENE SMBus (RAM) — register map confirmed 3-way, detection method rejected, Lenovo motherboard — implemented (2026-10-29, same session), Not yet cross-checked this pass, Open questions, Our board (`0DB0:0076`) — triple-confirmed, including the shared bug (+16 more)

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

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "live.md"
Cohesion: 0.06
Nodes (33): Apply at system scale, Audit before choosing, Choose a strategy, Contrast and perception, Live-mode signature params, Verify, Visitor mode, Step 1: Parse the request (+25 more)

### Community 51 - "Lighting Studio: whole-interface and unified-editor review"
Cohesion: 0.18
Nodes (10): Editor interaction contract, Elevation and stable hardware inventory, Evidence and limits, Findings and priorities, Implementation order and acceptance, Layers, colors, numeric controls and appearance, Lighting Studio: whole-interface and unified-editor review, Main decision (+2 more)

### Community 52 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 53 - "new-work.md"
Cohesion: 0.07
Nodes (23): Craft (deprecated alias), Apply, Live-mode signature params, Set the spatial thesis, Two isolated assessments, Verify, Visitor mode, Comps (+15 more)

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
Cohesion: 0.09
Nodes (19): previewOutputReports(), nativeKey(), OutputSession, appliedNative_, driver_, identity_, pending_, profile_ (+11 more)

### Community 58 - "HidDescriptor"
Cohesion: 0.15
Nodes (10): hid_device_, HidDescriptor, interfaceNumber, manufacturer, path, product, productId, usage (+2 more)

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
Cohesion: 0.13
Nodes (8): Hung device open and project-only conflict check - 2026-11-17, Task 2: Project edits update running hardware without reopening it, prepareOutput(), sameOutputScope(), Scan, key(), ServiceRuntime::ServiceRuntime(), writeFile()

### Community 67 - "OutputScheduler"
Cohesion: 0.13
Nodes (7): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "FakeHidDevice"
Cohesion: 0.15
Nodes (11): Completed: HID transport (2026-10-23), FakeHidDevice, featureReportResponse, getFeatureReportCalls, inputReportResponse, opened_, openSucceeds, readInputReportCalls (+3 more)

### Community 70 - "OutputScheduler.cpp"
Cohesion: 0.09
Nodes (7): DeviceChanges, generation, mutex, wake, onHidInterface(), OutputScheduler::OutputScheduler(), watchDevices()

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
Nodes (24): Component catalog schema 1, Task 5: `BreathingEffect`, Task 7: `CometEffect` and `MarqueeEffect`, Pitfalls, ColorEditor, hex_, picker_, public (+16 more)

### Community 78 - "SKILL.md"
Cohesion: 0.07
Nodes (25): Before you finish, Scope is sovereign, The amplification, The skeleton test, Why it reads flat, Craft floor, Refuse, Verify (+17 more)

### Community 79 - "test_service.cpp"
Cohesion: 0.14
Nodes (6): cmd(), identity(), project(), ServiceTest, private, ServiceTest::protocolBounds()

### Community 80 - "PaletteStrip"
Cohesion: 0.14
Nodes (12): PaletteStrip, add, begin, colors, dragging, end, gradient, move (+4 more)

### Community 81 - "EffectKind"
Cohesion: 0.05
Nodes (41): EffectKind, Aurora, Burst, Checkerboard, Circle, ConicalGradient, Cross, Diamond (+33 more)

### Community 82 - "SaturationValueField"
Cohesion: 0.05
Nodes (26): 1. Decide what is already true, 2. Ask what will change the work, 3. Choose the right amount of invention, 4. Commit the world, 5. Record the decision, 6. Build with full commitment, 7. Inspect and finish, Both paths (+18 more)

### Community 83 - "showToast"
Cohesion: 0.11
Nodes (28): abandonForeignSession(), abandonSupersededGo(), cleanup(), clearScrollY(), discardedWrappers(), discardOrphanedSession(), discardStateStyleId(), handleDiscard() (+20 more)

### Community 84 - "ChannelAssignment"
Cohesion: 0.09
Nodes (18): chainOffsets(), chainSharing(), channelMapping(), chainStarts(), ChannelAssignment, channelId, endpoint, id (+10 more)

### Community 85 - "ConflictProcesses.cpp"
Cohesion: 0.14
Nodes (21): belongsToMysticLightService(), conflictReason(), creationTime(), isSignalRgbService(), matchesProcessIdentity(), mysticLightServiceStopped(), NativeServices, manager_ (+13 more)

### Community 86 - "mountSvelteComponentVariant"
Cohesion: 0.12
Nodes (26): acceptedDomAlreadyClean(), clearHandledWrapperReloadStamp(), commitAcceptedSvelteComponentToDom(), commitAcceptedVariantToDom(), componentModuleCandidates(), deferredRecoverySuperseded(), describeMountFailure(), detectDevServerBase() (+18 more)

### Community 87 - "adapt.md"
Cohesion: 0.08
Nodes (22): Assess Adaptation Challenge, Content Adaptation, Desktop Adaptation (Mobile → Desktop), Email Adaptation (Web → Email), Implement Adaptations, Layout Adaptation Techniques, Mobile Adaptation (Desktop → Mobile), Adaptation Strategies (+14 more)

### Community 88 - ".pushColors"
Cohesion: 0.08
Nodes (13): Task 10: Roccat Kone Aimo driver, feature(), output(), PeripheralDriver, channel_, initialized_, BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport() (+5 more)

### Community 89 - "ServiceServer"
Cohesion: 0.17
Nodes (8): Service pipe access list - 2026-11-14 (Windows only), QLocalServer, QLocalSocket, QWinEventNotifier, ServiceServer, clients_, error_, owner_

### Community 90 - "Layer visibility, brush size and Delete"
Cohesion: 0.22
Nodes (9): Artwork alignment and custom device geometry, Detailed hardware artwork, Fixed-device settings versus attachable headers, Gaming workspace and responsive panels, Layer visibility, brush size and Delete, Mask playback, navigation and cosmetic hardware variants, Model labels, fullsize keyboards and zero-angle defaults, Scrollbar contrast and Effects preview divider (+1 more)

### Community 91 - "document.md"
Cohesion: 0.09
Nodes (22): Component translation rules, Narrative mapping, Scan mode (approach C: auto-extract, then confirm descriptive language), Schema, Seed mode, Step 1: Find the design assets, Step 1: Route through new-work's workshop, Step 2: Auto-extract what can be auto-extracted (+14 more)

### Community 92 - "AeroBackend"
Cohesion: 0.11
Nodes (14): AeroBackend, attempted_, failed_, handle_, kChannel, lastSend_, library_, read_ (+6 more)

### Community 93 - ".cancelGesture"
Cohesion: 0.09
Nodes (3): MaskTool, Brush, Gradient

### Community 94 - "FakeProcesses"
Cohesion: 0.05
Nodes (39): ConflictService, acceptsStop, commandLine, name, ownProcess, pid, state, State (+31 more)

### Community 95 - "Layer model proposal"
Cohesion: 0.20
Nodes (10): Acceptance matrix, Creation and selection contract, Delivery in reviewable increments, Explain the blends with a live result, Explicitly outside the first implementation, Feasibility evidence and resolved review points, Layer model proposal, Problem confirmed in the current code (+2 more)

### Community 96 - "QMap"
Cohesion: 0.20
Nodes (5): SharedRun, firstLed, lastLed, source, sharedColorNote()

### Community 97 - "HardwarePicker"
Cohesion: 0.11
Nodes (14): TODO: custom hardware lives only inside the project - 2026-11-17, HardwarePicker, headers_, leds_, products_, productsList_, HeaderChoice, identity (+6 more)

### Community 98 - "array"
Cohesion: 0.16
Nodes (9): AsrockPolychromeZone, Argb1, Argb2, Audio, IoCover, Pcb, Pch, Rgb1 (+1 more)

### Community 99 - "IDriverModule.h"
Cohesion: 0.12
Nodes (9): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+1 more)

### Community 100 - "LightingDocument"
Cohesion: 0.06
Nodes (20): ComponentPlacement, componentId, id, locked, origin, rotation, LightingDocument, colorPreviewBefore_ (+12 more)

### Community 102 - "Lighting Studio GUI and interaction audit"
Cohesion: 0.18
Nodes (10): Color picker, Consolidation, Evidence, Findings, ordered by impact, Identity and direction, Implementation follow-up, Implementation order and acceptance, Integer scales (+2 more)

### Community 104 - "onboard.md"
Cohesion: 0.09
Nodes (22): Assess Onboarding Needs, Context Over Ceremony, Contextual Help, Design Onboarding Experiences, Documentation & Help, Empty State Design, Feature Discovery & Adoption, Guided Tours & Walkthroughs (+14 more)

### Community 105 - "Test review: recent lighting changes (2026-10-24)"
Cohesion: 0.25
Nodes (7): Defects exposed before production changes, Final validation, Independent expected results, Limits, Mutation checks, Previously adjusted expectations, Test review: recent lighting changes (2026-10-24)

### Community 106 - "The Toolkit"
Cohesion: 0.10
Nodes (20): Animate complex properties, Assess What "Extraordinary" Means Here, For data-heavy interfaces, For functional UI, For performance-critical UI, For visual/marketing surfaces, Implement with Discipline, Interact with the device (+12 more)

### Community 107 - "Gradient travel and blend verification (2026-10-24)"
Cohesion: 0.25
Nodes (8): Canvas playback, fullscreen and growing Layout, Gradient travel and blend verification (2026-10-24), Identify flash (2026-11-15), Live layout movement and output selection, Per-device LED arrangement, Physical LEDs and reserved protocol slots, RAM component geometry correction, Restored integrated lighting

### Community 108 - "Lightning and Fireworks visual review (2026-10-29)"
Cohesion: 0.50
Nodes (3): Before and after, Lightning and Fireworks visual review (2026-10-29), Sources and observation limits

### Community 109 - "ServiceProtocol.cpp"
Cohesion: 0.20
Nodes (13): Installed editor could not reach the service - 2026-11-17, authorizedClient(), currentUserSid(), decode(), encode(), installedHost(), isSystem(), pipeName() (+5 more)

### Community 110 - "main"
Cohesion: 0.06
Nodes (23): scanAuxiliaryHardware(), BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), ProfileCandidate, device (+15 more)

### Community 112 - "actOnAgentTarget"
Cohesion: 0.22
Nodes (20): actOnAgentTarget(), agentTargetBusyReason(), agentTargetOverlayGone(), agentTargetTaken(), claimAgentTarget(), claimAndActOnAgentTarget(), declineAgentTargetBusy(), declineAgentTargetUnresolvable() (+12 more)

### Community 113 - "service/main.cpp"
Cohesion: 0.11
Nodes (13): Containment, Painted material, Region map, What varies independently, control(), knownFolder(), logWarning(), main() (+5 more)

### Community 114 - "ServiceServer.cpp"
Cohesion: 0.12
Nodes (7): Act on the receipt, Assemble and review, Plan and asset review, Plan, capture, serve, ServiceServer::connected(), handler_, ServiceServer::ServiceServer()

### Community 115 - "AsrockPolychromeDriver"
Cohesion: 0.07
Nodes (11): AsrockPolychromeDriver, channels_, state_, AsrockPolychromeModule, mapSpeed(), ZoneState, applied, color (+3 more)

### Community 116 - "createLiveBrowserDomHelpers"
Cohesion: 0.16
Nodes (15): createLiveBrowserDomHelpers(), activeElementDeep(), cloneWithoutChrome(), copyTextViaSelection(), cssId(), liveUiRoot(), makeFrozenAnchor(), raiseHitModal() (+7 more)

### Community 117 - "StudioWidgets.cpp"
Cohesion: 0.07
Nodes (23): ChannelEditor::ChannelEditor(), openStudioColorPicker(), ComponentEditor::ComponentEditor(), ColorEditor::ColorEditor(), ColorListEditor::ColorListEditor(), disclosure(), installParameterTooltips(), NumericFieldStyle (+15 more)

### Community 118 - "createLiveBrowserSessionState"
Cohesion: 0.19
Nodes (17): createLiveBrowserSessionState(), clearHandled(), clearScrollY(), clearSession(), isHandled(), loadSession(), markHandled(), nextCheckpointRevision() (+9 more)

### Community 119 - "UsbWrite"
Cohesion: 0.15
Nodes (12): ProcessEntry, parent, path, pid, started, UsbWrite, count, device (+4 more)

### Community 120 - "Record"
Cohesion: 0.14
Nodes (6): Record, closes, mutex, reports, Transport, record_

### Community 122 - "Studio interaction and motion review"
Cohesion: 0.09
Nodes (20): Editor interaction implementation, Identity and color sampling, Persistence and limits, Pointer behavior, Rendering and output status, Result, Data and transport boundaries, Design principles checked (+12 more)

### Community 125 - "DeviceConflicts.cpp"
Cohesion: 0.13
Nodes (31): Responsive service and installed conflict dialog - 2026-11-16, alive(), captureUsbWrites(), creationTime(), deviceInstanceFromPath(), endpointBusy(), endpointLabel(), findWriters() (+23 more)

### Community 129 - "ComponentDefinition"
Cohesion: 0.06
Nodes (29): Task 4: Component data model, ComponentDefinition, category, id, manufacturer, mapping, name, nonPhysicalSlots (+21 more)

### Community 131 - "Tool"
Cohesion: 0.18
Nodes (10): Tool, Ellipse, Eraser, Fill, Gradient, Line, Move, Pan (+2 more)

### Community 133 - ".load"
Cohesion: 0.08
Nodes (14): fail(), ModuleRegistry, entries_, AsrockPolychromeTest, identity(), profile(), HpOmenTest, identity() (+6 more)

### Community 134 - "Connection"
Cohesion: 0.33
Nodes (4): Connection, approved, generation, scheduler

### Community 135 - "PawnIO integration and service feasibility"
Cohesion: 0.11
Nodes (16): Access control and process boundary, Conclusion, Coverage and next gate, Exact evidence, Installation and licensing boundary, PawnIO integration and service feasibility, Performance and recovery, Primary sources (+8 more)

### Community 137 - "CONTINUE.md"
Cohesion: 0.12
Nodes (15): Branching and releases - 2026-11-15, CodeRabbit round 2 fixes, R12 - 2026-11-17, Editor fixes and menu cleanup, R16 - 2026-11-17, Editor fixes and small audit fixes - 2026-11-14 (GS-06-R7, build/r4), Event-driven presence check - 2026-11-18, GS-06-R2 UI correction - 2026-11-13, GS-06-R9 - 2026-11-15, Header LED count - 2026-11-15 (+7 more)

### Community 138 - "Lighting Studio editor UX review"
Cohesion: 0.15
Nodes (13): Additional procedural sources, Captures and measured checks, Crash evidence and validation boundaries, Creative-software research, Direction and scope, Findings and implementation, Gaming-Pro-Studio refinement, GS-06 finish (+5 more)

### Community 140 - "onAnnotDown"
Cohesion: 0.18
Nodes (19): applyPlaceholderDimensions(), beginEditPin(), buildAnnotationsForCapture(), buildPinElement(), cancelEditingPin(), finalizeEditingPin(), initAnnotOverlay(), localCoords() (+11 more)

### Community 143 - "JARGB2 physical response tests - 2026-11-13 / 2026-11-14"
Cohesion: 0.22
Nodes (8): Approved tests and observations, Follow-up observations - 2026-11-14, JARGB2 physical response tests - 2026-11-13 / 2026-11-14, Pattern 6 - sequential fan-prefix qualification, Sleep/resume event, Software implications, Software status - 2026-11-14, Supported interpretation

### Community 146 - "ConflictProcess"
Cohesion: 0.07
Nodes (26): ConflictProcess, elevatedStop, executable, manualStop, path, pid, provenHolder, reason (+18 more)

### Community 150 - "Hardware migration inventory and priorities"
Cohesion: 0.33
Nodes (6): Browse the full list, Hardware migration inventory and priorities, Prioritized migration queue, Reproduction and next decisions, Scale and counting units, Source provenance and limitations

### Community 151 - "OutputConnection"
Cohesion: 0.33
Nodes (4): OutputConnection, factory, initial, present

### Community 153 - "Impeccable Asset Producer"
Cohesion: 0.12
Nodes (14): Core Rule, Decision Comps, Impeccable Asset Producer, Input Contract, Output Contract, Review handoff, The job, Core Rule (+6 more)

### Community 154 - "animate.md"
Cohesion: 0.12
Nodes (14): Accessibility and control, Choose material by meaning, Find the job, Implement to the runtime, Set the motion thesis, Timing and easing, Verify, Visitor mode (+6 more)

### Community 156 - "Generate Report"
Cohesion: 0.12
Nodes (15): 1. Accessibility (A11y), 2. Performance, 3. Theming, 4. Responsive Design, 5. Implementation Integrity (CRITICAL), Audit Health Score, Detailed Findings by Severity, Diagnostic Scan (+7 more)

### Community 157 - "README.md"
Cohesion: 0.13
Nodes (6): OpenRGB controller-family worklist, HP Omen 30L migration: first independent-module increment, Integration and qualification, Pinned reference comparison, Selected protocol and scope, Entwurf: Kommentar für GitLab-Issues #4935 / #5429

### Community 158 - "audit.native.md"
Cohesion: 0.12
Nodes (15): 1. Accessibility (VoiceOver / TalkBack), 2. Performance, 3. Appearance & Theming, 4. Platform Conformance (CRITICAL), 5. Adaptivity, Audit Health Score, Detailed Findings by Severity, Diagnostic Scan (+7 more)

### Community 159 - "ParameterSpinBox"
Cohesion: 0.18
Nodes (6): maximum, minimum, ParameterSpinBox, scale_, typed_, typedAt_

### Community 160 - "test_hid_backend.cpp"
Cohesion: 0.19
Nodes (8): HID_API_CALL hid_close(), HID_API_CALL hid_error(), HID_API_CALL hid_free_enumeration(), HID_API_CALL hid_get_feature_report(), HID_API_CALL hid_read_timeout(), HID_API_CALL hid_send_feature_report(), HID_API_CALL hid_write(), hid_device_

### Community 162 - "Handle `generate`"
Cohesion: 0.12
Nodes (16): 1. Read the screenshot (if present), 2. Wrap the element, 3. Load the action's reference, 4. Plan three variants: identity first, then mode, then axes, 5. Apply the freeform prompt (if present), 6. Deliver variants, 7. Parameters (composition-sized, 0-4 per variant), 8. Signal done (+8 more)

### Community 163 - "MsiGen1AdvArea"
Cohesion: 0.07
Nodes (26): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiGen1AdvArea, JAF (+18 more)

### Community 164 - "OutputPanel.cpp"
Cohesion: 0.25
Nodes (3): channelKey(), OutputPanel::OutputPanel(), stateName()

### Community 165 - "LightingCanvas.h"
Cohesion: 0.29
Nodes (7): Task 10: Qt tray app shell, QAction, QMenu, QPainter, QPushButton, QScrollBar, QTimer

### Community 167 - "Workspace navigation and catalog identity"
Cohesion: 0.67
Nodes (3): Addressable LEDs and NV5 MKII evidence, MSI board configuration versus spatial pixel frames, Workspace navigation and catalog identity

### Community 168 - "Hardware support strategy"
Cohesion: 0.33
Nodes (6): Evidence and qualification, Execution boundary, Hardware support strategy, Independent hardware modules, Installed manufacturer software and peripheral settings, Mandatory comparison of reference implementations

### Community 169 - "Output session backend"
Cohesion: 0.33
Nodes (6): Automatic GUI output and presence, Output scheduler, Output session backend, Restored native transports, Transient layout sampling and per-channel pause, Verification

### Community 170 - "Artwork, continuous gestures and spatial controls (GS-06)"
Cohesion: 0.67
Nodes (3): Artwork, continuous gestures and spatial controls (GS-06), Image-editor crop and visible preview scope (2026-11-13), Startup compatibility hotfix (2026-10-30)

### Community 172 - "optimize.md"
Cohesion: 0.14
Nodes (13): Animation Performance, Assess Performance Issues, Core Web Vitals Optimization, Cumulative Layout Shift (CLS < 0.1), Interaction to Next Paint (INP < 200ms), Largest Contentful Paint (LCP < 2.5s), Loading Performance, Network Optimization (+5 more)

### Community 175 - "critique.md"
Cohesion: 0.17
Nodes (11): Action Summary, Ask the User, Assessment A: Design Review, Assessment B: Detector + Browser Evidence, Assessment Orchestration, Deliver the Report, Hard Invariants, Persist the Snapshot (+3 more)

### Community 177 - "Simplify the Design"
Cohesion: 0.17
Nodes (11): Assess Current State, Code Simplification, Content Simplification, Document Removed Complexity, Information Architecture, Interaction Simplification, Layout Simplification, Plan Simplification (+3 more)

### Community 178 - "Hardening Dimensions"
Cohesion: 0.17
Nodes (11): Accessibility Resilience, Assess Hardening Needs, Edge Cases & Boundary Conditions, Error Handling, Hardening Dimensions, Input Validation & Sanitization, Internationalization (i18n), Performance Resilience (+3 more)

### Community 179 - "clarify.md"
Cohesion: 0.18
Nodes (10): Actions and navigation, Audit the language, Errors and permissions, Forms, Help and instructional text, Loading, empty, and success states, Rewrite by function, Set the message hierarchy (+2 more)

### Community 180 - "ChannelEditor"
Cohesion: 0.13
Nodes (8): TODO: shared LED colors are hard to find - 2026-11-17, ChannelEditor, fixedComponent_, initial_, ComponentEditor, category_, manufacturer_, nonPhysicalSlots_

### Community 181 - "QPainter"
Cohesion: 0.24
Nodes (3): SnapGrid, enabled, spacing

### Community 183 - "HidApiDevice"
Cohesion: 0.12
Nodes (8): Resolved: `IHidDevice` read capability added (API 3), HidApiDevice, handle_, initialized_, HidRuntime, ready, initializeHid(), HidBackendTest

### Community 184 - "Effect editor interaction research"
Cohesion: 0.25
Nodes (8): Acceptance and implementation gates, Effect editor interaction research, Findings in current source, Interaction contract, Interface and identity, Recommended content model, Reference patterns, Scope and evidence

### Community 185 - "Nielsen's 10 Heuristics"
Cohesion: 0.18
Nodes (11): 10. Help and Documentation, 1. Visibility of System Status, 2. Match Between System and Real World, 3. User Control and Freedom, 4. Consistency and Standards, 5. Error Prevention, 6. Recognition Rather Than Recall, 7. Flexibility and Efficiency of Use (+3 more)

### Community 186 - "polish.md"
Cohesion: 0.18
Nodes (10): 1. Establish the system, 2. Gather the evidence, 3. Triage, 4. Polish the whole path, 5. Verify and finish, Color, imagery, and icons, Content and code, Flow and hierarchy (+2 more)

### Community 188 - "quieter.md"
Cohesion: 0.18
Nodes (10): Assess Current State, Color Refinement, Composition Refinement, Motion Reduction, Plan Refinement, Refine the Design, Simplification, Verify Quality (+2 more)

### Community 190 - "EffectParameter"
Cohesion: 0.25
Nodes (6): EffectParameter, EffectLayer, id, label, schemaVersion, practicalEffectRange()

### Community 191 - "Responsive Design"
Cohesion: 0.20
Nodes (10): Breakpoints: Content-Driven, Detect Input Method, Not Just Screen Size, Layout Adaptation Patterns, Mobile-First: Write It Right, Picture Element for Art Direction, Responsive Design, Responsive Images: Get It Right, Safe Areas: Handle the Notch (+2 more)

### Community 192 - "Generate Combined Critique Report"
Cohesion: 0.20
Nodes (10): Design Health Score, Design Specificity Verdict, Generate Combined Critique Report, Minor Observations, Overall Impression, Persona Red Flags, Priority Issues, Questions to Consider (+2 more)

### Community 193 - "Init flow"
Cohesion: 0.20
Nodes (10): Completion gate, Init flow, Step 1: Load current state, Step 2: Explore the project, Step 3: Interview for product truth, Step 4: Write PRODUCT.md, Step 5: Record workflow defaults, Step 6: Wrap up or resume (+2 more)

### Community 194 - "Common Cognitive Load Violations"
Cohesion: 0.22
Nodes (9): 1. The Wall of Options, 2. The Memory Bridge, 3. The Hidden Navigation, 4. The Jargon Barrier, 5. The Visual Noise Floor, 6. The Inconsistent Pattern, 7. The Multi-Task Demand, 8. The Context Switch (+1 more)

### Community 196 - "iOS platform"
Cohesion: 0.22
Nodes (9): Color & materials, Components & controls, iOS platform, Layout & structure, Motion, The iOS slop test, Touch targets, Typography (+1 more)

### Community 197 - "Operate mode depth (and Read notes)"
Cohesion: 0.22
Nodes (9): Color, Components, Layout, Motion, Operate mode depth (and Read notes), Product constraints, Product permissions, The product slop test (+1 more)

### Community 198 - "Shape"
Cohesion: 0.22
Nodes (8): Cadence, Confirm and stop, Phase 1: Discovery interview, Phase 2: Resolve the design direction, Phase 3: Write the brief, Round 1: purpose, people, and outcome, Round 2: material, behavior, and boundaries, Shape

### Community 199 - "Android platform"
Cohesion: 0.25
Nodes (8): Android platform, Color & theming, Components & motion, Layout & structure, The Android slop test, Touch targets, Typography, Verifying the build

### Community 200 - "Persona-Based Design Testing"
Cohesion: 0.25
Nodes (8): 1. Impatient Power User: "Alex", 2. Confused First-Timer: "Jordan", 3. Accessibility-Dependent User: "Sam", 4. Deliberate Stress Tester: "Riley", 5. Distracted Mobile User: "Casey", Persona-Based Design Testing, Project-Specific Personas, Selecting Personas

### Community 201 - "Hardware modules"
Cohesion: 0.07
Nodes (25): Adding support, Build and inspect, Compatibility and trust, Direct PawnIO RAM endpoints, Hardware discovery, Hardware modules, Integrated component assignment contract, LED discovery (+17 more)

### Community 202 - "Extract Flow"
Cohesion: 0.25
Nodes (7): Extract Flow, Step 1: Discover the Design System, Step 2: Identify Patterns, Step 3: Plan Extraction, Step 4: Extract & Enrich, Step 5: Migrate, Step 6: Document

### Community 203 - "Capture"
Cohesion: 0.14
Nodes (10): Capture, devices, instances, writes, eventProperty(), onUsbEvent(), pointerProperty(), Seen (+2 more)

### Community 204 - "Impeccable Finish Reviewer"
Cohesion: 0.29
Nodes (6): Checks, in order, Disposition, Impeccable Finish Reviewer, Input Contract, Output Contract, Verdict Pass

### Community 205 - "Impeccable Manual Edit Applier"
Cohesion: 0.29
Nodes (6): Checks, Entry Atomicity, Impeccable Manual Edit Applier, Input Contract, Output Contract, Workflow

### Community 206 - "Cognitive Load Assessment"
Cohesion: 0.29
Nodes (7): Cognitive Load Assessment, Cognitive Load Checklist, Extraneous Load: Bad Design, Germane Load: Learning Effort, Intrinsic Load: The Task Itself, The Working Memory Rule, Three Types of Cognitive Load

### Community 207 - "Impeccable Finish Reviewer"
Cohesion: 0.29
Nodes (6): Checks, in order, Disposition, Impeccable Finish Reviewer, Input Contract, Output Contract, Verdict Pass

### Community 208 - "Impeccable Manual Edit Applier"
Cohesion: 0.29
Nodes (6): Checks, Entry Atomicity, Impeccable Manual Edit Applier, Input Contract, Output Contract, Workflow

### Community 209 - "live-browser-ignores.js"
Cohesion: 0.52
Nodes (6): globToRegex(), matchesScope(), normalizeIgnoreRule(), normalizeIgnoreValue(), pageCandidates(), resolveDetectIgnores()

### Community 210 - "Reliability and layer-design work (2026-11-11)"
Cohesion: 0.33
Nodes (6): Collaboration contract, Current status, Decisions and validation boundaries, Final software validation, Package and scope limits, Reliability and layer-design work (2026-11-11)

### Community 211 - "Visualize: Direction Comps & Asset Production"
Cohesion: 0.33
Nodes (5): After approval: the comp becomes a spec, Generate three compositional options, One approval point, Plates and provenance, Visualize: Direction Comps & Asset Production

### Community 212 - "Impeccable Documenter"
Cohesion: 0.40
Nodes (4): Impeccable Documenter, Input Contract, Output Contract, Workflow

### Community 213 - "`Mystic Light Online Data.dat` — decryption and content"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 214 - "Heuristics Scoring Guide"
Cohesion: 0.50
Nodes (4): Heuristics Scoring Guide, Issue Severity (P0–P3), Reference Material, Score Summary

### Community 215 - "RgbColor"
Cohesion: 0.09
Nodes (15): Responsibilities, AuxiliaryDriver, channel_, HpOmenDriver, channels_, NativeEffect, brightness, colors (+7 more)

### Community 217 - "doctor.md"
Cohesion: 0.25
Nodes (7): Monorepo notes, Opting out of the boot check, Step 1: Run the pass, Step 2: Act by severity, Step 3: Deprecated fields are binding, Step 4: Do not overclaim on truth drift, What this owns, and what it does not

### Community 218 - "Review Focus"
Cohesion: 0.29
Nodes (6): Editor Fixes and Menu Cleanup Implementation Plan, Global Constraints, Review Focus, Task 3: Portable start and its documentation, Task 5: Effect library with sorted categories and a clear add button, Task 6: Release R16

### Community 219 - "Impeccable Documenter"
Cohesion: 0.40
Nodes (4): Impeccable Documenter, Input Contract, Output Contract, Workflow

### Community 220 - "I2cInfo"
Cohesion: 0.18
Nodes (11): I2cInfo, address, ddc, displayMask, port, portSet, reg, regSize (+3 more)

### Community 221 - "EffectRenderer.h"
Cohesion: 0.14
Nodes (9): EffectDefinition, description, id, kind, label, particleCount, patternSites, patternSpeed (+1 more)

### Community 222 - "Effect implementation checklist"
Cohesion: 0.20
Nodes (9): Additional native patterns, Additional native source batch, Capability gate and particle follow-up, Editing follow-up, Effect implementation checklist, First implementation batch, Second batch and color picker, Second batch and remaining candidates (+1 more)

### Community 223 - "ScanMailbox"
Cohesion: 0.33
Nodes (4): ScanMailbox, finished, mutex, owner

### Community 224 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 225 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

### Community 226 - "studioToolIcon"
Cohesion: 0.11
Nodes (5): QToolButton, LayerDelegate, studioThemeColor(), StudioSectionButton, studioToolIcon()

### Community 227 - "impl_"
Cohesion: 0.29
Nodes (6): impl_, address, library, ready, sent, temporaryFailure

### Community 229 - "Lighting Studio 0.1.0"
Cohesion: 0.50
Nodes (3): Changes, Lighting Studio 0.1.0, Requirements and limitations

### Community 230 - "DeviceProfile.cpp"
Cohesion: 0.36
Nodes (5): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text()

### Community 232 - "Third-party runtime components"
Cohesion: 0.50
Nodes (3): Hardware artwork, HP Omen 30L protocol module, Third-party runtime components

## Knowledge Gaps
- **1578 isolated node(s):** `mode`, `speed`, `color`, `applied`, `channels_` (+1573 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 2179 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **49 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `OutputPanel`, `HardwarePicker`, `ComponentDefinition`, `LightingDocument`, `OutputPanel.cpp`, `src/main.cpp`, `ComponentEditor.cpp`, `LightingWindow.cpp`, `test_output_ui.cpp`, `test_editor.cpp`, `main`, `test_service.cpp`, `ServicePanel.cpp`, `LightingCanvas`, `PreviewDivider`, `Review Focus`, `.edit`, `Layer model proposal`?**
  _High betweenness centrality (0.087) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `AuxiliaryTransport`, `.load`, `MsiDriver`, `MsiZoneSettings`, `stdexcept`, `Recording`, `IDriverModule`, `Transport`, `MsiGen1AdvArea`, `LenovoMotherboardModule.cpp`, `PeripheralModule`, `IDriver`, `RecordingTransport`, `HidApiDevice`, `.connect`, `Transport`, `FakeHidDevice`, `Hardware modules`, `VirtualModule`, `RgbColor`, `.pushColors`, `array`, `IDriverModule.h`, `AsrockPolychromeDriver`, `Record`?**
  _High betweenness centrality (0.086) - this node is a cross-community bridge._
- **Why does `text()` connect `DeviceProfile.cpp` to `service/main.cpp`, `SaturationValueField`, `SKILL.md`, `ServicePanel.cpp`?**
  _High betweenness centrality (0.077) - this node is a cross-community bridge._
- **Are the 90 inferred relationships involving `LightingWindow` (e.g. with `Problem confirmed in the current code` and `Editor Fixes and Menu Cleanup Implementation Plan`) actually correct?**
  _`LightingWindow` has 90 INFERRED edges - model-reasoned connections that need verification._
- **Are the 8 inferred relationships involving `catalog()` (e.g. with `.componentLedOffsets()` and `.componentName()`) actually correct?**
  _`catalog()` has 8 INFERRED edges - model-reasoned connections that need verification._
- **What connects `mode`, `speed`, `color` to the rest of the system?**
  _1578 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `live-browser.js` be split into smaller, more focused modules?**
  _Cohesion score 0.03164627987880148 - nodes in this community are weakly interconnected._