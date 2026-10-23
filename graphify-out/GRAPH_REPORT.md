# Graph Report - msi-mystic-light  (2026-10-23)

## Corpus Check
- 74 files · ~68,078 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 3 file(s) not represented in the graph (top: (none) 2, .cmake 1)

## Summary
- 1148 nodes · 2120 edges · 67 communities (47 shown, 20 thin omitted)
- Extraction: 83% EXTRACTED · 17% INFERRED · 0% AMBIGUOUS · INFERRED: 357 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `ee321d20`
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
- ComponentEditor.h
- ARCHITECTURE.md
- Reference repository reuse review
- MSI Mystic Light — Reverse Engineering
- ConflictDialog.cpp
- test_editor.cpp
- EffectLayer
- LightingWindow
- ModuleRegistry
- Offline lighting editor
- LightingWindow::LightingWindow
- LightingWindow.h
- ConflictProcesses.cpp
- EffectKind
- LightingCanvas.cpp
- ComponentCatalog.cpp
- NativeEffect
- ConflictDialog
- LightingState
- SignalForge editor reference
- VirtualModule
- MsiModule
- .load
- Hardware modules
- Lighting Studio — Design Doc (v2)
- QTreeWidget
- ComponentPlacement
- LightingDocument.h
- Standalone Lighting App — v1 Skeleton Implementation Plan
- MaskTool
- .paintEvent
- QComboBox

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 73 edges
2. `LightingCanvas` - 58 edges
3. `EffectLayer` - 45 edges
4. `LightingDocument` - 38 edges
5. `IHidDevice` - 30 edges
6. `OutputSession` - 28 edges
7. `EffectKind` - 26 edges
8. `EditorTest` - 26 edges
9. `Implementation status` - 25 edges
10. `ChannelAssignment` - 23 edges

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

## Communities (67 total, 20 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.11
Nodes (19): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+11 more)

### Community 1 - "ChannelAssignment"
Cohesion: 0.15
Nodes (11): ChannelAssignment, channelId, endpoint, id, native, placements, profileId, software (+3 more)

### Community 2 - "File Structure"
Cohesion: 0.14
Nodes (15): File Structure, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 12: Application wire-up and first real-hardware verification, Task 1: Project scaffolding and test harness, Task 4: Component data model, Task 5: Channel model, Task 7: Effect interface and built-in effects (+7 more)

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
Cohesion: 0.05
Nodes (30): Output session backend, Verification, ChannelOutput, channelId, colors, native, software, nativeKey() (+22 more)

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
Cohesion: 0.18
Nodes (7): DriverInstance, driver_, loader_, Entry, id, loader, module

### Community 14 - "MsiDriver"
Cohesion: 0.24
Nodes (4): MsiDriver, armed_, channels_, state_

### Community 15 - "IDriverModule"
Cohesion: 0.15
Nodes (4): Adding support, IDriverModule, IncompatibleModule, MissingInterface

### Community 16 - "findProfileCandidates"
Cohesion: 0.14
Nodes (9): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), ProfileCandidate, device, profileId (+1 more)

### Community 17 - "BuildReport0x51"
Cohesion: 0.19
Nodes (6): Completed: first v1 increment (2026-10-23), Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 19 - "Recording"
Cohesion: 0.12
Nodes (10): Recording, closes, destroyed, failAt, opened, opens, reports, throwWrite (+2 more)

### Community 20 - "IHidDevice"
Cohesion: 0.11
Nodes (11): Completed: HID transport (2026-10-23), Self-Review, Task 3: HID transport interface and hidapi backend, Task 18: Final wire-up and real-hardware verification, Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, IHidDevice, FakeHidDevice, opened_ (+3 more)

### Community 21 - "Implementation status"
Cohesion: 0.09
Nodes (23): Accepted effect authoring follow-ups (2026-10-23), Completed: accepted offline effect authoring scope (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23) (+15 more)

### Community 23 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 24 - "LightingCanvas"
Cohesion: 0.06
Nodes (24): LightingCanvas, cropTool_, dragging_, gray_, handle_, hardness_, last_, maskTarget_ (+16 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 27 - "ARCHITECTURE.md"
Cohesion: 0.28
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

### Community 36 - "ModuleRegistry"
Cohesion: 0.22
Nodes (4): main(), fail(), ModuleRegistry, entries_

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

### Community 42 - "EffectKind"
Cohesion: 0.06
Nodes (44): EffectDefinition, id, kind, label, EffectKind, Burst, Circle, ConicalGradient (+36 more)

### Community 44 - "ComponentCatalog.cpp"
Cohesion: 0.09
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

### Community 52 - "Hardware modules"
Cohesion: 0.20
Nodes (9): Build and inspect, Compatibility and trust, Hardware discovery, Hardware modules, LED discovery, Output-session ownership, Profile schema 1, Startup process conflicts (+1 more)

### Community 53 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.13
Nodes (14): Anti-goals, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals, Intent and why (+6 more)

### Community 54 - "QTreeWidget"
Cohesion: 0.60
Nodes (3): LayerTree, reordered, QTreeWidget

### Community 55 - "ComponentPlacement"
Cohesion: 0.20
Nodes (6): ComponentPlacement, componentId, id, origin, rotation, scale

### Community 57 - "LightingDocument.h"
Cohesion: 0.18
Nodes (6): LightingState, LayerBlend, Multiply, Normal, Overlay, Screen

### Community 58 - "Standalone Lighting App — v1 Skeleton Implementation Plan"
Cohesion: 0.50
Nodes (3): Global Constraints, Review Focus, Standalone Lighting App — v1 Skeleton Implementation Plan

### Community 63 - "MaskTool"
Cohesion: 0.50
Nodes (3): MaskTool, Brush, Gradient

## Knowledge Gaps
- **411 isolated node(s):** `channels_`, `armed_`, `state_`, `manufacturer`, `model` (+406 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 610 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **20 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `ModuleRegistry`, `IDriverModule.h`, `HidDescriptor`, `OutputSession`, `NativeEffect`, `MsiDriver`, `IDriverModule`, `BuildReport0x51`, `MsiModule`, `VirtualModule`, `Recording`, `SignalRGB repository research`?**
  _High betweenness centrality (0.150) - this node is a cross-community bridge._
- **Why does `LightingDocument` connect `test_editor.cpp` to `ChannelAssignment`, `LightingWindow`, `OutputSession`, `LightingCanvas.cpp`, `IDriverModule`, `findProfileCandidates`, `LightingState`, `LightingCanvas`, `LightingDocument.h`, `ComponentEditor.h`?**
  _High betweenness centrality (0.134) - this node is a cross-community bridge._
- **Why does `LightingCanvas` connect `LightingCanvas` to `test_editor.cpp`, `.paintEvent`, `LightingWindow`, `IDriverModule.h`, `OutputSession`, `EffectKind`, `LightingCanvas.cpp`, `.canvasPoint`, `MaskTool`?**
  _High betweenness centrality (0.114) - this node is a cross-community bridge._
- **Are the 12 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.advancedControls()`) actually correct?**
  _`LightingWindow` has 12 INFERRED edges - model-reasoned connections that need verification._
- **Are the 10 inferred relationships involving `IHidDevice` (e.g. with `Completed: HID transport (2026-10-23)` and `Adding support`) actually correct?**
  _`IHidDevice` has 10 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _411 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.1067193675889328 - nodes in this community are weakly interconnected._