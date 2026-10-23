# Graph Report - msi-mystic-light  (2026-10-23)

## Corpus Check
- 70 files · ~65,892 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 3 file(s) not represented in the graph (top: (none) 2, .cmake 1)

## Summary
- 1058 nodes · 1918 edges · 65 communities (44 shown, 21 thin omitted)
- Extraction: 84% EXTRACTED · 16% INFERRED · 0% AMBIGUOUS · INFERRED: 303 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `c50a5bb4`
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
- MsiGen1AdvArea
- Development Tools
- MsiZoneSettings
- MsiLightingMode
- DriverInstance
- MsiDriver
- IDriverModule
- findProfileCandidates
- BuildReport0x51
- LightingCanvas.cpp
- test_hid_backend.cpp
- FakeHidDevice
- Implementation status
- README.md
- SignalRGB repository research
- LightingCanvas
- Mystic Light — Overall Architecture
- ComponentEditor.h
- `Mystic Light Online Data.dat` — decryption and content
- Reference repository reuse review
- MSI Mystic Light — Reverse Engineering
- ConflictDialog.cpp
- test_editor.cpp
- EffectLayer
- LightingWindow
- .create
- Offline lighting editor
- LightingWindow::LightingWindow
- LightingWindow.h
- ConflictProcesses.cpp
- EffectKind
- .cancelGesture
- ComponentCatalog.cpp
- DeviceProfile.cpp
- ConflictDialog
- LightingState
- SignalForge editor reference
- IHidDevice
- MsiModule
- ModulesTest
- Hardware modules
- ComponentsTest
- QTreeWidget
- ComponentPlacement
- channelMapping
- LayerBlend
- QComboBox

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 73 edges
2. `LightingCanvas` - 58 edges
3. `EffectLayer` - 45 edges
4. `LightingDocument` - 36 edges
5. `IHidDevice` - 27 edges
6. `EffectKind` - 26 edges
7. `EditorTest` - 26 edges
8. `Implementation status` - 24 edges
9. `ChannelAssignment` - 23 edges
10. `Lighting Studio Implementation Plan` - 23 edges

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

## Communities (65 total, 21 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.11
Nodes (18): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+10 more)

### Community 1 - "ChannelAssignment"
Cohesion: 0.06
Nodes (26): Anti-goals, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals, Intent and why (+18 more)

### Community 2 - "File Structure"
Cohesion: 0.12
Nodes (18): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness (+10 more)

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
Cohesion: 0.10
Nodes (15): DiscoveryStatus, Available, Failed, Unknown, Unsupported, LedTopology, error, ledCounts (+7 more)

### Community 7 - "HidDescriptor"
Cohesion: 0.07
Nodes (17): hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer, path (+9 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "DriverInstance"
Cohesion: 0.16
Nodes (9): DriverInstance, driver_, loader_, Entry, id, loader, module, ModuleRegistry (+1 more)

### Community 14 - "MsiDriver"
Cohesion: 0.24
Nodes (4): MsiDriver, armed_, channels_, state_

### Community 15 - "IDriverModule"
Cohesion: 0.15
Nodes (4): Adding support, IDriverModule, IncompatibleModule, MissingInterface

### Community 16 - "findProfileCandidates"
Cohesion: 0.13
Nodes (10): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), ProfileCandidate, device, profileId (+2 more)

### Community 17 - "BuildReport0x51"
Cohesion: 0.18
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 19 - "test_hid_backend.cpp"
Cohesion: 0.19
Nodes (5): HID_API_CALL hid_close(), HID_API_CALL hid_error(), HID_API_CALL hid_free_enumeration(), HID_API_CALL hid_send_feature_report(), hid_device_

### Community 20 - "FakeHidDevice"
Cohesion: 0.15
Nodes (8): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, Task 3: Canvas-global placement and "follow global effect", FakeHidDevice, opened_, openSucceeds, sendSucceeds, sentReports

### Community 21 - "Implementation status"
Cohesion: 0.09
Nodes (22): Accepted effect authoring follow-ups (2026-10-23), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23) (+14 more)

### Community 23 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 24 - "LightingCanvas"
Cohesion: 0.06
Nodes (24): LightingCanvas, cropTool_, dragging_, gray_, handle_, hardness_, last_, maskTarget_ (+16 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 27 - "`Mystic Light Online Data.dat` — decryption and content"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "MSI Mystic Light — Reverse Engineering"
Cohesion: 0.40
Nodes (5): Contents, MSI Mystic Light — Reverse Engineering, Related repos, Research documents, Standalone app implementation

### Community 32 - "test_editor.cpp"
Cohesion: 0.06
Nodes (19): channelAssignmentJson(), parseChannelAssignment(), validateChannelAssignments(), DocumentEdit, apply_, LightingDocument, initialCatalog_, LightingDocument::LightingDocument() (+11 more)

### Community 33 - "EffectLayer"
Cohesion: 0.06
Nodes (33): EffectLayer, blend, breathe, center, color, colorCycle, crop, cycleRate (+25 more)

### Community 34 - "LightingWindow"
Cohesion: 0.05
Nodes (35): LightingWindow, advanced_, animation_, animationControls_, assignments_, blend_, catalog_, color_ (+27 more)

### Community 37 - "Offline lighting editor"
Cohesion: 0.14
Nodes (13): Component authoring and import, Component sampling, Effects and direct tools, Images and crop, Layers and masks, Layout, Layout transforms, Motion and color (+5 more)

### Community 38 - "LightingWindow::LightingWindow"
Cohesion: 0.17
Nodes (3): spin, colorIcon(), LightingWindow::LightingWindow()

### Community 39 - "LightingWindow.h"
Cohesion: 0.18
Nodes (8): QCheckBox, QDoubleSpinBox, QLineEdit, QPushButton, QSlider, QSpinBox, QTabWidget, QTreeWidgetItem

### Community 41 - "ConflictProcesses.cpp"
Cohesion: 0.05
Nodes (25): closeWindow(), ConflictProcess, executable, manualStop, path, pid, reason, started (+17 more)

### Community 42 - "EffectKind"
Cohesion: 0.06
Nodes (44): EffectDefinition, id, kind, label, EffectKind, Burst, Circle, ConicalGradient (+36 more)

### Community 43 - ".cancelGesture"
Cohesion: 0.17
Nodes (4): LightingCanvas::LightingCanvas(), MaskTool, Brush, Gradient

### Community 44 - "ComponentCatalog.cpp"
Cohesion: 0.33
Nodes (7): ComponentDefinition::fromJson(), ComponentDefinition::toJson(), integer(), loadComponentCatalog(), loadComponentDefinition(), requiredText(), QJsonObject

### Community 45 - "DeviceProfile.cpp"
Cohesion: 0.36
Nodes (5): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text()

### Community 46 - "ConflictDialog"
Cohesion: 0.19
Nodes (10): ConflictDialog, busy_, conflicts_, ignore_, status_, table_, terminateAll_, QLabel (+2 more)

### Community 47 - "LightingState"
Cohesion: 0.33
Nodes (5): LightingState, assignments, components, layers, placements

### Community 48 - "SignalForge editor reference"
Cohesion: 0.22
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 49 - "IHidDevice"
Cohesion: 0.11
Nodes (5): Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 18: Final wire-up and real-hardware verification, IHidDevice, VirtualDriver, VirtualModule

### Community 52 - "Hardware modules"
Cohesion: 0.13
Nodes (13): Build and inspect, Compatibility and trust, Hardware discovery, Hardware modules, LED discovery, Profile schema 1, Responsibilities, Startup process conflicts (+5 more)

### Community 54 - "QTreeWidget"
Cohesion: 0.60
Nodes (3): LayerTree, reordered, QTreeWidget

### Community 55 - "ComponentPlacement"
Cohesion: 0.20
Nodes (6): ComponentPlacement, componentId, id, origin, rotation, scale

### Community 57 - "LayerBlend"
Cohesion: 0.40
Nodes (5): LayerBlend, Multiply, Normal, Overlay, Screen

## Knowledge Gaps
- **383 isolated node(s):** `channels_`, `armed_`, `state_`, `manufacturer`, `model` (+378 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 561 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **21 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `test_editor.cpp`, `QComboBox`, `LightingWindow.cpp`, `LightingWindow::LightingWindow`, `LightingWindow.h`, `findProfileCandidates`, `QTreeWidget`, `LightingCanvas`?**
  _High betweenness centrality (0.145) - this node is a cross-community bridge._
- **Why does `LightingCanvas` connect `LightingCanvas` to `test_editor.cpp`, `.setPreviewFrame`, `LightingWindow`, `IDriverModule.h`, `EffectKind`, `.cancelGesture`, `LightingCanvas.cpp`, `.mousePressEvent`?**
  _High betweenness centrality (0.122) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `File Structure`, `.create`, `IDriverModule.h`, `HidDescriptor`, `stdexcept`, `MsiDriver`, `IDriverModule`, `BuildReport0x51`, `MsiModule`, `FakeHidDevice`, `SignalRGB repository research`, `IHidDevice.h`?**
  _High betweenness centrality (0.111) - this node is a cross-community bridge._
- **Are the 12 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 12 INFERRED edges - model-reasoned connections that need verification._
- **Are the 10 inferred relationships involving `IHidDevice` (e.g. with `Completed: HID transport (2026-10-23)` and `Adding support`) actually correct?**
  _`IHidDevice` has 10 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _383 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.10822510822510822 - nodes in this community are weakly interconnected._