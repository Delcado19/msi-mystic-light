# Graph Report - msi-mystic-light  (2026-10-23)

## Corpus Check
- 62 files · ~50,278 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 3 file(s) not represented in the graph (top: (none) 2, .cmake 1)

## Summary
- 915 nodes · 1518 edges · 52 communities (44 shown, 8 thin omitted)
- Extraction: 88% EXTRACTED · 12% INFERRED · 0% AMBIGUOUS · INFERRED: 176 edges (avg confidence: 0.87)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `d216b2ce`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- Lighting Studio — Design Doc (v2)
- File Structure
- Mystic Light — local network interfaces
- Standalone RGB Lighting App — Design
- MSI Mainboard LED Protocol
- IDriverModule.h
- HidDescriptor
- ComponentsTest
- MsiGen1AdvArea
- Development Tools
- MsiZoneSettings
- MsiLightingMode
- ModulesTest
- MsiDriver
- IDriverModule
- findProfileCandidates
- stdexcept
- VirtualDriver
- Self-Review
- FakeHidDevice
- Implementation status
- README.md
- SignalRGB repository research
- LightingCanvas
- Mystic Light — Overall Architecture
- test_editor.cpp
- `Mystic Light Online Data.dat` — decryption and content
- Reference repository reuse review
- MSI Mystic Light — Reverse Engineering
- ConflictProcesses.cpp
- ConflictDialog
- LightingDocument.cpp
- EffectLayer
- LightingWindow
- LightingWindow.cpp
- Offline lighting editor
- IHidDevice
- Hardware modules
- ComponentCatalog.cpp
- EffectKind
- MsiModule
- NativeEffect
- DeviceProfile.cpp
- RgbColor
- ProfileCandidate
- SignalForge editor reference
- VirtualModule
- DiscoveryStatus

## God Nodes (most connected - your core abstractions)
1. `LightingWindow` - 59 edges
2. `LightingCanvas` - 49 edges
3. `EffectLayer` - 29 edges
4. `IHidDevice` - 27 edges
5. `LightingDocument` - 25 edges
6. `Lighting Studio Implementation Plan` - 23 edges
7. `MsiZoneSettings` - 22 edges
8. `MsiGen1AdvArea` - 22 edges
9. `EffectKind` - 21 edges
10. `IDriver` - 20 edges

## Surprising Connections (you probably didn't know these)
- `Device plugins and gaps in our current contract` --references--> `IHidDevice`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/hid/IHidDevice.h
- `Inventory and pinned revisions` --references--> `main()`  [INFERRED]
  docs/SIGNALRGB_REPOSITORY_RESEARCH.md → src/main.cpp
- `Spatial model (extends, doesn't replace, v1's effect model)` --references--> `IDriver`  [INFERRED]
  docs/superpowers/specs/2026-10-19-lighting-studio-design.md → src/modules/IDriverModule.h
- `Task 8: Canvas render loop` --references--> `FakeHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → tests/hid/FakeHidDevice.h
- `Component catalog schema 1` --references--> `channelMapping()`  [INFERRED]
  docs/HARDWARE_MODULES.md → src/components/ComponentCatalog.cpp

## Import Cycles
- None detected.

## Communities (52 total, 8 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.11
Nodes (18): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 13: Extend `Settings` — placement, source, global effect, profiles, Task 14: `LightingCanvasWidget` (+10 more)

### Community 1 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.13
Nodes (14): Anti-goals, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals, Intent and why (+6 more)

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
Cohesion: 0.20
Nodes (4): LedTopology, error, ledCounts, status

### Community 7 - "HidDescriptor"
Cohesion: 0.05
Nodes (22): hid_device_, HidApiDevice, handle_, initialized_, HidDescriptor, interfaceNumber, manufacturer, path (+14 more)

### Community 8 - "ComponentsTest"
Cohesion: 0.19
Nodes (4): Component catalog schema 1, channelMapping(), loadComponentCatalog(), ComponentsTest

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.14
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 13 - "ModulesTest"
Cohesion: 0.08
Nodes (12): main(), DriverInstance, driver_, loader_, Entry, id, loader, module (+4 more)

### Community 14 - "MsiDriver"
Cohesion: 0.24
Nodes (4): MsiDriver, armed_, channels_, state_

### Community 15 - "IDriverModule"
Cohesion: 0.15
Nodes (4): Adding support, IDriverModule, IncompatibleModule, MissingInterface

### Community 16 - "findProfileCandidates"
Cohesion: 0.19
Nodes (6): BoardIdentity, manufacturer, model, deviceIdentity(), findProfileCandidates(), readBoardIdentity()

### Community 17 - "stdexcept"
Cohesion: 0.16
Nodes (7): Completed: first v1 increment (2026-10-23), Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 19 - "Self-Review"
Cohesion: 0.40
Nodes (4): Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan

### Community 20 - "FakeHidDevice"
Cohesion: 0.17
Nodes (7): Completed: HID transport (2026-10-23), Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, FakeHidDevice, opened_, openSucceeds, sendSucceeds, sentReports

### Community 21 - "Implementation status"
Cohesion: 0.12
Nodes (17): Accepted effect authoring follow-ups (2026-10-23), Completed: animation preview increment (2026-10-23), Completed: embedded image effects (2026-10-23), Completed: first offline editor increment (2026-10-23), Completed: gradient and shape effects (2026-10-23), Completed: local component catalog and channel mapping (2026-10-23), Completed: local hardware module framework (2026-10-23), Completed: read-only hardware discovery (2026-10-23) (+9 more)

### Community 23 - "SignalRGB repository research"
Cohesion: 0.22
Nodes (9): Components are data, not hardware drivers, Device plugins and gaps in our current contract, Exact MSI B850 findings, Import validation and provenance, Inventory and pinned revisions, Recommended next implementation increment, Repository installation and update evidence, Result (+1 more)

### Community 24 - "LightingCanvas"
Cohesion: 0.06
Nodes (25): Application structure, LightingCanvas, dragging_, gray_, hardness_, last_, LightingCanvas::LightingCanvas(), maskTarget_ (+17 more)

### Community 25 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 26 - "test_editor.cpp"
Cohesion: 0.12
Nodes (6): document_, catalog(), EditorTest, private, EditorTest::rgbaAndMask(), layer()

### Community 27 - "`Mystic Light Online Data.dat` — decryption and content"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 28 - "Reference repository reuse review"
Cohesion: 0.12
Nodes (14): CompGen, Decision summary, Implementation and validation sequence, LightScript Workshop, Lueur, Lumen, Other previously listed integrations, Reference repository reuse review (+6 more)

### Community 29 - "MSI Mystic Light — Reverse Engineering"
Cohesion: 0.40
Nodes (5): Contents, MSI Mystic Light — Reverse Engineering, Related repos, Research documents, Standalone app implementation

### Community 30 - "ConflictProcesses.cpp"
Cohesion: 0.05
Nodes (25): closeWindow(), ConflictProcess, executable, manualStop, path, pid, reason, started (+17 more)

### Community 31 - "ConflictDialog"
Cohesion: 0.12
Nodes (11): ConflictDialog, busy_, ConflictDialog::ConflictDialog(), conflicts_, ignore_, status_, table_, terminateAll_ (+3 more)

### Community 32 - "LightingDocument.cpp"
Cohesion: 0.07
Nodes (18): EffectLayer, interpolate(), isGradient(), polygon(), renderBaseEffect(), renderEffect(), validateEffect(), DocumentEdit (+10 more)

### Community 33 - "EffectLayer"
Cohesion: 0.06
Nodes (29): ComponentPlacement, componentId, id, origin, EffectLayer, blend, center, color (+21 more)

### Community 34 - "LightingWindow"
Cohesion: 0.05
Nodes (37): Task 15: `LightingStudioWindow`, LightingWindow, animation_, animationControls_, blend_, catalog_, color_, editingMask_ (+29 more)

### Community 35 - "LightingWindow.cpp"
Cohesion: 0.09
Nodes (6): button(), colorIcon(), LayerTree, reordered, LightingWindow::LightingWindow(), QTreeWidget

### Community 37 - "Offline lighting editor"
Cohesion: 0.29
Nodes (6): Animation preview, Effects, Layout, Offline lighting editor, Project files, Validation and remaining work

### Community 38 - "IHidDevice"
Cohesion: 0.20
Nodes (3): Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 18: Final wire-up and real-hardware verification, IHidDevice

### Community 39 - "Hardware modules"
Cohesion: 0.22
Nodes (8): Build and inspect, Compatibility and trust, Hardware discovery, Hardware modules, LED discovery, Profile schema 1, Startup process conflicts, Validation

### Community 41 - "ComponentCatalog.cpp"
Cohesion: 0.24
Nodes (3): ComponentDefinition::fromJson(), integer(), requiredText()

### Community 42 - "EffectKind"
Cohesion: 0.09
Nodes (21): EffectDefinition, id, kind, label, EffectKind, Circle, ConicalGradient, Cross (+13 more)

### Community 44 - "NativeEffect"
Cohesion: 0.29
Nodes (6): NativeEffect, brightness, colors, id, inward, speed

### Community 45 - "DeviceProfile.cpp"
Cohesion: 0.36
Nodes (6): DeviceProfile::fromJson(), DeviceProfile::load(), DeviceProfile::matches(), integer(), text(), QJsonObject

### Community 46 - "RgbColor"
Cohesion: 0.33
Nodes (5): Responsibilities, RgbColor, b, g, r

### Community 47 - "ProfileCandidate"
Cohesion: 0.25
Nodes (3): ProfileCandidate, device, profileId

### Community 48 - "SignalForge editor reference"
Cohesion: 0.40
Nodes (5): Evidence and useful concepts, Proposed adaptation for this app, Reuse boundary, SignalForge editor reference, Sources

### Community 52 - "DiscoveryStatus"
Cohesion: 0.40
Nodes (5): DiscoveryStatus, Available, Failed, Unknown, Unsupported

## Knowledge Gaps
- **323 isolated node(s):** `channels_`, `armed_`, `state_`, `manufacturer`, `model` (+318 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 485 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **8 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `LightingWindow` connect `LightingWindow` to `LightingDocument.cpp`, `LightingWindow.cpp`, `ModulesTest`, `LightingCanvas`, `test_editor.cpp`?**
  _High betweenness centrality (0.172) - this node is a cross-community bridge._
- **Why does `IHidDevice` connect `IHidDevice` to `Lighting Studio Implementation Plan`, `File Structure`, `IDriverModule.h`, `HidDescriptor`, `MsiModule`, `ModulesTest`, `MsiDriver`, `IDriverModule`, `stdexcept`, `VirtualDriver`, `Self-Review`, `FakeHidDevice`, `VirtualModule`, `SignalRGB repository research`?**
  _High betweenness centrality (0.132) - this node is a cross-community bridge._
- **Why does `main()` connect `ModulesTest` to `LightingWindow`, `HidDescriptor`, `ComponentsTest`, `findProfileCandidates`, `main.cpp`, `SignalRGB repository research`?**
  _High betweenness centrality (0.129) - this node is a cross-community bridge._
- **Are the 7 inferred relationships involving `LightingWindow` (e.g. with `main()` and `.animationPreview()`) actually correct?**
  _`LightingWindow` has 7 INFERRED edges - model-reasoned connections that need verification._
- **Are the 10 inferred relationships involving `IHidDevice` (e.g. with `Completed: HID transport (2026-10-23)` and `Adding support`) actually correct?**
  _`IHidDevice` has 10 INFERRED edges - model-reasoned connections that need verification._
- **What connects `channels_`, `armed_`, `state_` to the rest of the system?**
  _323 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation Plan` be split into smaller, more focused modules?**
  _Cohesion score 0.10822510822510822 - nodes in this community are weakly interconnected._