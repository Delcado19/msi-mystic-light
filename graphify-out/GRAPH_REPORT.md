# Graph Report - msi-mystic-light  (2026-10-23)

## Corpus Check
- 22 files · ~24,024 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 3 file(s) not represented in the graph (top: (none) 2, .cmake 1)

## Summary
- 247 nodes · 323 edges · 14 communities (12 shown, 2 thin omitted)
- Extraction: 83% EXTRACTED · 17% INFERRED · 0% AMBIGUOUS · INFERRED: 56 edges (avg confidence: 0.9)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `87e02098`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- Lighting Studio Design
- File Structure
- Mystic Light — Overall Architecture
- Standalone App Design
- MSI Mainboard LED Protocol
- Mystic Light Network Interfaces
- test_hid_backend.cpp
- BuildReport0x51
- MsiGen1AdvArea
- Development Tools
- MsiZoneSettings
- MsiLightingMode

## God Nodes (most connected - your core abstractions)
1. `MsiGen1AdvArea` - 22 edges
2. `Lighting Studio Implementation Plan` - 22 edges
3. `MsiZoneSettings` - 20 edges
4. `FakeHidDevice` - 20 edges
5. `IHidDevice` - 17 edges
6. `MsiLightingMode` - 15 edges
7. `MsiColor` - 13 edges
8. `File Structure` - 13 edges
9. `BuildReport0x51()` - 12 edges
10. `Lighting Studio — Design Doc (v2)` - 12 edges

## Surprising Connections (you probably didn't know these)
- `Task 8: Canvas render loop` --references--> `FakeHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → tests/hid/FakeHidDevice.h
- `Completed: HID transport (2026-10-23)` --references--> `IHidDevice`  [INFERRED]
  CONTINUE.md → src/hid/IHidDevice.h
- `Self-Review` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/hid/IHidDevice.h
- `Task 3: HID transport interface and hidapi backend` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/hid/IHidDevice.h
- `Task 6: MSI motherboard driver` --references--> `IHidDevice`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/hid/IHidDevice.h

## Import Cycles
- None detected.

## Communities (14 total, 2 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.06
Nodes (27): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+19 more)

### Community 1 - "Lighting Studio Design"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 2 - "File Structure"
Cohesion: 0.11
Nodes (19): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness (+11 more)

### Community 3 - "Mystic Light — Overall Architecture"
Cohesion: 0.10
Nodes (18): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll, Content (+10 more)

### Community 4 - "Standalone App Design"
Cohesion: 0.20
Nodes (9): Anti-goals, Constraints, Goals, How it gets made (decided), Intent and why, Non-goals (v1), Settled during review (builder-check pass), Standalone RGB Lighting App — Design (+1 more)

### Community 5 - "MSI Mainboard LED Protocol"
Cohesion: 0.22
Nodes (9): MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`), Record layout (16 bytes, per zone/"area"), Report `0x50` ("Gen1"), 290 bytes — status and default configuration, Report `0x51` ("Gen1_Adv"), 727 bytes — per-LED addressing of individual segments, Report `0x90`+port / `0xB0`+port ("Gen2"/"Gen2_Adv") — newer strip model, ⚠️ Safety-relevant finding: the `Store` flag is set on every write, Upstream OpenRGB bug found while cross-checking: `FeaturePacket_PerLED_761` is 727 bytes, not 761, Verification against the existing capture template (+1 more)

### Community 6 - "Mystic Light Network Interfaces"
Cohesion: 0.25
Nodes (8): 1. TCP 26820 — local HTTP "Web API" (`MLModule.dll:CWebAPI.cs`), 2. UDP 26822 — mobile app pairing (`CBaseServer.cs`/`CBaseFN.cs`), 3. UDP 26823 — "party sync" between multiple PCs (`CSyncServer.cs`/`CSyncFN.cs`), 4. UDP 61337 — SignalRGB bridge ("LEDKeeper 2" service), Live verification (2026-10-17): the whole MLModule network layer is dead code, Mystic Light — local network interfaces, Protocol (confirmed via code, extends the `Msi.js` reconstruction), Still open

### Community 7 - "test_hid_backend.cpp"
Cohesion: 0.09
Nodes (9): hid_device_, HidApiDevice, handle_, initialized_, HidRuntime, ready, HID_API_CALL hid_send_feature_report(), hid_device_ (+1 more)

### Community 8 - "BuildReport0x51"
Cohesion: 0.10
Nodes (11): Completed: first v1 increment (2026-10-23), Completed: HID transport (2026-10-23), Implementation status, Next steps, Validation, Task 12: Application wire-up and first real-hardware verification, Task 2: MSI wire-protocol encoders, Task 6: MSI motherboard driver (+3 more)

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.12
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

## Knowledge Gaps
- **123 isolated node(s):** `ready`, `handle_`, `initialized_`, `r`, `g` (+118 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 156 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **2 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `IHidDevice` connect `Lighting Studio Implementation Plan` to `BuildReport0x51`, `File Structure`, `MsiLightingMode`, `test_hid_backend.cpp`?**
  _High betweenness centrality (0.255) - this node is a cross-community bridge._
- **Why does `FakeHidDevice` connect `Lighting Studio Implementation Plan` to `BuildReport0x51`, `File Structure`?**
  _High betweenness centrality (0.157) - this node is a cross-community bridge._
- **Are the 2 inferred relationships involving `MsiGen1AdvArea` (e.g. with `Task 2: MSI wire-protocol encoders` and `Task 6: MSI motherboard driver`) actually correct?**
  _`MsiGen1AdvArea` has 2 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `MsiZoneSettings` (e.g. with `Self-Review` and `Task 2: MSI wire-protocol encoders`) actually correct?**
  _`MsiZoneSettings` has 3 INFERRED edges - model-reasoned connections that need verification._
- **Are the 8 inferred relationships involving `FakeHidDevice` (e.g. with `Completed: HID transport (2026-10-23)` and `Self-Review`) actually correct?**
  _`FakeHidDevice` has 8 INFERRED edges - model-reasoned connections that need verification._
- **Are the 8 inferred relationships involving `IHidDevice` (e.g. with `Completed: HID transport (2026-10-23)` and `Self-Review`) actually correct?**
  _`IHidDevice` has 8 INFERRED edges - model-reasoned connections that need verification._
- **What connects `ready`, `handle_`, `initialized_` to the rest of the system?**
  _123 weakly-connected nodes found - possible documentation gaps or missing edges._