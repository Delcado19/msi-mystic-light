# Graph Report - msi-mystic-light  (2026-10-23)

## Corpus Check
- 77 files · ~70,042 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 3 file(s) not represented in the graph (top: (none) 2, .cmake 1)

## Summary
- 1249 nodes · 2330 edges · 74 communities (56 shown, 18 thin omitted)
- Extraction: 83% EXTRACTED · 17% INFERRED · 0% AMBIGUOUS · INFERRED: 388 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `32df02f5`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- ChannelAssignment
- File Structure
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- IDriverModule.h
- HidDescriptor
- OutputSession
- MsiGen1AdvArea
- Development Tools
- MsiZoneSettings
- MsiLightingMode
- DriverInstance
- MsiDriver
- IDriverModule
- findProfileCandidates
- BuildReport0x51
- Recording
- IHidDevice
- Implementation status
- README.md
- SignalRGB repository research
- LightingCanvas
- Mystic Light — Overall Architecture
- ChannelEditor
- ARCHITECTURE.md
- Reference repository reuse review
- MSI Mystic Light — Reverse Engineering
- ChannelEditor.cpp
- ConflictDialog.cpp
- test_editor.cpp
- EffectLayer
- LightingWindow
- ModuleRegistry
- Offline lighting editor
- LightingWindow::LightingWindow
- LightingWindow.h
- ConflictProcesses.cpp
- EffectRenderer.cpp
- stdexcept
- NativeEffect
- ConflictDialog
- LightingState
- SignalForge editor reference
- Record
- EffectKind
- ModulesTest
- Hardware modules
- Lighting Studio — Design Doc (v2)
- QTreeWidget
- ComponentPlacement
- .latestDuringSlowWrite
- LightingDocument.h
- Self-Review
- Transport
- .cancelGesture
- .canvasSize
- QComboBox
- OutputScheduler
- EffectParameter
- test_scheduler.cpp
- .start
- SignalRGB GitHub topic triage
- Status
- LayerBlend

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 73 edges
2. `LightingCanvas` - 58 edges
3. `EffectLayer` - 45 edges
4. `LightingDocument` - 38 edges
5. `Record` - 34 edges
6. `IHidDevice` - 31 edges
7. `OutputSession` - 28 edges
8. `EffectKind` - 26 edges
9. `OutputScheduler` - 26 edges
10. `EditorTest` - 26 edges

## Surprising Connections (you probably didn't know these)
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Inventory and pinned revisions` --references--> `main()`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/main.cpp
- `Output scheduler` --references--> `OutputScheduler`  [INFERRED]
  docs/OUTPUT_SESSION.md → src/output/OutputScheduler.h
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp
- `Self-Review` --references--> `ChannelAssignment`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/editor/ChannelAssignment.h

## Import Cycles
- None detected.

## Communities (74 total, 18 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.11
Nodes (17): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+9 more)

### Community 1 - "ChannelAssignment"
Cohesion: 0.20
Nodes (9): Task 3: Canvas-global placement and "follow global effect", ChannelAssignment, channelId, endpoint, id, native, placements, profileId (+1 more)

### Community 2 - "File Structure"
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

### Community 6 - "IDriverModule.h"
Cohesion: 0.14
Nodes (9): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+1 more)

### Community 7 - "HidDescriptor"
Cohesion: 0.05
Nodes (22): hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer, path (+14 more)

### Community 8 - "OutputSession"
Cohesion: 0.06
Nodes (31): Output scheduler, Output session backend, Verification, ChannelOutput, channelId, colors, native, software (+23 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.14
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "DriverInstance"
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 14 - "MsiDriver"
Cohesion: 0.13
Nodes (5): MsiDriver, armed_, channels_, state_, MsiModule

### Community 15 - "IDriverModule"
Cohesion: 0.11
Nodes (4): IDriverModule, IncompatibleModule, MissingInterface, VirtualModule

### Community 16 - "findProfileCandidates"
Cohesion: 0.14
Nodes (10): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), ProfileCandidate, device, profileId (+2 more)

### Community 17 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "IHidDevice"
Cohesion: 0.11
Nodes (10): Completed: HID transport (2026-10-23), Task 18: Final wire-up and real-hardware verification, Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, Task 9: SteelSeries Apex 3 driver (T-Zone protocol), IHidDevice, FakeHidDevice, opened_, openSucceeds (+2 more)

### Community 21 - "Implementation status"
Cohesion: 0.08
Nodes (24): Accepted effect authoring follow-ups (2026-10-23), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23) (+16 more)

### Community 23 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 24 - "LightingCanvas"
Cohesion: 0.06
Nodes (24): LightingCanvas, cropTool_, dragging_, gray_, handle_, hardness_, last_, maskTarget_ (+16 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 26 - "ChannelEditor"
Cohesion: 0.20
Nodes (4): ChannelEditor, initial_, ComponentEditor, ComponentEditor::ComponentEditor()

### Community 27 - "ARCHITECTURE.md"
Cohesion: 0.28
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 28 - "Reference repository reuse review"
Cohesion: 0.20
Nodes (10): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+2 more)

### Community 29 - "MSI Mystic Light — Reverse Engineering"
Cohesion: 0.40
Nodes (5): Contents, MSI Mystic Light — Reverse Engineering, Related repos, Research documents, Standalone app implementation

### Community 32 - "test_editor.cpp"
Cohesion: 0.07
Nodes (19): channelAssignmentJson(), parseChannelAssignment(), validateChannelAssignments(), DocumentEdit, apply_, LightingDocument, initialCatalog_, LightingDocument::LightingDocument() (+11 more)

### Community 33 - "EffectLayer"
Cohesion: 0.06
Nodes (33): EffectLayer, blend, breathe, center, color, colorCycle, crop, cycleRate (+25 more)

### Community 34 - "LightingWindow"
Cohesion: 0.05
Nodes (35): LightingWindow, advanced_, animation_, animationControls_, assignments_, blend_, catalog_, color_ (+27 more)

### Community 36 - "ModuleRegistry"
Cohesion: 0.19
Nodes (3): fail(), ModuleRegistry, entries_

### Community 37 - "Offline lighting editor"
Cohesion: 0.15
Nodes (13): Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Layers and masks, Layout, Layout transforms, Motion and color (+5 more)

### Community 38 - "LightingWindow::LightingWindow"
Cohesion: 0.17
Nodes (3): spin, colorIcon(), LightingWindow::LightingWindow()

### Community 39 - "LightingWindow.h"
Cohesion: 0.17
Nodes (8): QCheckBox, QDoubleSpinBox, QLineEdit, QPushButton, QSlider, QSpinBox, QTabWidget, QTreeWidgetItem

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.05
Nodes (25): closeWindow(), ConflictProcess, executable, manualStop, path, pid, reason, started (+17 more)

### Community 42 - "EffectRenderer.cpp"
Cohesion: 0.23
Nodes (12): effectTransform(), interpolate(), isGradient(), isParticle(), polygon(), renderAtTime(), renderBaseEffect(), renderEffect() (+4 more)

### Community 44 - "stdexcept"
Cohesion: 0.08
Nodes (15): Component catalog schema 1, channelMapping(), ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), loadComponentDefinition(), requiredText() (+7 more)

### Community 45 - "NativeEffect"
Cohesion: 0.12
Nodes (12): Responsibilities, NativeEffect, brightness, colors, id, inward, speed, RgbColor (+4 more)

### Community 46 - "ConflictDialog"
Cohesion: 0.19
Nodes (10): ConflictDialog, busy_, conflicts_, ignore_, status_, table_, terminateAll_, QLabel (+2 more)

### Community 47 - "LightingState"
Cohesion: 0.33
Nodes (5): LightingState, assignments, components, layers, placements

### Community 48 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 49 - "Record"
Cohesion: 0.08
Nodes (19): Record, blockFactory, blockWrite, closedOn, closes, createdOn, destroyed, destroyedOn (+11 more)

### Community 50 - "EffectKind"
Cohesion: 0.10
Nodes (21): EffectKind, Burst, Circle, ConicalGradient, Cross, Diamond, Fireflies, Heart (+13 more)

### Community 52 - "Hardware modules"
Cohesion: 0.18
Nodes (10): Adding support, Build and inspect, Compatibility and trust, Hardware discovery, Hardware modules, LED discovery, Output-session ownership, Profile schema 1 (+2 more)

### Community 53 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.13
Nodes (14): Anti-goals, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals, Intent and why (+6 more)

### Community 54 - "QTreeWidget"
Cohesion: 0.60
Nodes (3): LayerTree, reordered, QTreeWidget

### Community 55 - "ComponentPlacement"
Cohesion: 0.20
Nodes (6): ComponentPlacement, componentId, id, origin, rotation, scale

### Community 56 - ".latestDuringSlowWrite"
Cohesion: 0.27
Nodes (6): assertClosedOnWorker(), factory(), frame(), SchedulerTest, SchedulerTest::latestWhileStarting(), private

### Community 58 - "Self-Review"
Cohesion: 0.40
Nodes (4): Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan

### Community 59 - "Transport"
Cohesion: 0.15
Nodes (4): ReleaseGate, record, Transport, record_

### Community 63 - ".cancelGesture"
Cohesion: 0.17
Nodes (4): LightingCanvas::LightingCanvas(), MaskTool, Brush, Gradient

### Community 67 - "OutputScheduler"
Cohesion: 0.15
Nodes (7): OutputScheduler, mutex_, pending_, public, stopRequested_, wake_, worker_

### Community 68 - "EffectParameter"
Cohesion: 0.18
Nodes (10): EffectDefinition, id, kind, label, EffectParameter, EffectLayer, id, label (+2 more)

### Community 71 - "SignalRGB GitHub topic triage"
Cohesion: 0.33
Nodes (4): Priorities, Scope, licensing and evidence, SignalRGB GitHub topic triage, Useful later or only for matching hardware

### Community 72 - "Status"
Cohesion: 0.33
Nodes (4): Status, error, generation, state

### Community 73 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

## Knowledge Gaps
- **441 isolated node(s):** `channels_`, `armed_`, `state_`, `manufacturer`, `model` (+436 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 665 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **18 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `File Structure`, `ModuleRegistry`, `IDriverModule.h`, `HidDescriptor`, `OutputSession`, `NativeEffect`, `MsiDriver`, `IDriverModule`, `BuildReport0x51`, `Recording`, `Hardware modules`, `SignalRGB repository research`, `Self-Review`, `Transport`?**
  _High betweenness centrality (0.137) - this node is a cross-community bridge._
- **Why does `LightingDocument` connect `test_editor.cpp` to `.canvasSize`, `LightingWindow`, `OutputSession`, `IDriverModule`, `findProfileCandidates`, `LightingState`, `LightingCanvas`, `LightingDocument.h`, `ChannelEditor`, `ChannelEditor.cpp`, `.cancelGesture`?**
  _High betweenness centrality (0.121) - this node is a cross-community bridge._
- **Why does `LightingCanvas` connect `LightingCanvas` to `.canvasSize`, `test_editor.cpp`, `LightingWindow`, `IDriverModule.h`, `EffectRenderer.cpp`, `LightingCanvas.cpp`, `.canvasPoint`, `LightingDocument.h`, `.cancelGesture`?**
  _High betweenness centrality (0.111) - this node is a cross-community bridge._
- **Are the 12 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 12 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _441 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.10952380952380952 - nodes in this community are weakly interconnected._
- **Should `File Structure` be split into smaller, more focused modules?**
  _Cohesion score 0.14166666666666666 - nodes in this community are weakly interconnected._