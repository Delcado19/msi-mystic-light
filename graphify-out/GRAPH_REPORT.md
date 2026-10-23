# Graph Report - msi-mystic-light  (2026-10-23)

## Corpus Check
- 17 files · ~23,039 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 2 file(s) not represented in the graph (top: (none) 2)

## Summary
- 188 nodes · 220 edges · 14 communities (12 shown, 2 thin omitted)
- Extraction: 88% EXTRACTED · 12% INFERRED · 0% AMBIGUOUS · INFERRED: 26 edges (avg confidence: 0.93)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `96eb4897`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation Plan
- Lighting Studio Design
- File Structure
- README.md
- Standalone App Design
- MSI Mainboard LED Protocol
- Mystic Light Network Interfaces
- Mystic Light System Architecture
- BuildReport0x51
- MsiGen1AdvArea
- Development Tools
- MsiZoneSettings
- MsiLightingMode

## God Nodes (most connected - your core abstractions)
1. `MsiGen1AdvArea` - 22 edges
2. `Lighting Studio Implementation Plan` - 22 edges
3. `MsiZoneSettings` - 20 edges
4. `MsiLightingMode` - 15 edges
5. `MsiColor` - 13 edges
6. `File Structure` - 13 edges
7. `BuildReport0x51()` - 12 edges
8. `Lighting Studio — Design Doc (v2)` - 12 edges
9. `BuildReport0x50()` - 11 edges
10. `Standalone RGB Lighting App — Design` - 9 edges

## Surprising Connections (you probably didn't know these)
- `Completed: first v1 increment (2026-10-23)` --references--> `BuildReport0x50()`  [INFERRED]
  CONTINUE.md → src/protocol/MsiProtocol.cpp
- `Task 2: MSI wire-protocol encoders` --references--> `BuildReport0x50()`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/protocol/MsiProtocol.cpp
- `Completed: first v1 increment (2026-10-23)` --references--> `BuildReport0x51()`  [INFERRED]
  CONTINUE.md → src/protocol/MsiProtocol.cpp
- `Task 2: MSI wire-protocol encoders` --references--> `BuildReport0x51()`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/protocol/MsiProtocol.cpp
- `Task 5: Channel model` --references--> `MsiColor`  [INFERRED]
  docs/superpowers/plans/2026-10-18-v1-skeleton.md → src/protocol/MsiProtocol.h

## Import Cycles
- None detected.

## Communities (14 total, 2 thin omitted)

### Community 0 - "Lighting Studio Implementation Plan"
Cohesion: 0.09
Nodes (21): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+13 more)

### Community 1 - "Lighting Studio Design"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 2 - "File Structure"
Cohesion: 0.11
Nodes (20): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 1: Project scaffolding and test harness (+12 more)

### Community 3 - "README.md"
Cohesion: 0.11
Nodes (15): Completed: first v1 increment (2026-10-23), Implementation status, Next steps, Validation, Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi) (+7 more)

### Community 4 - "Standalone App Design"
Cohesion: 0.20
Nodes (9): Anti-goals, Constraints, Goals, How it gets made (decided), Intent and why, Non-goals (v1), Settled during review (builder-check pass), Standalone RGB Lighting App — Design (+1 more)

### Community 5 - "MSI Mainboard LED Protocol"
Cohesion: 0.22
Nodes (9): MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`), Record layout (16 bytes, per zone/"area"), Report `0x50` ("Gen1"), 290 bytes — status and default configuration, Report `0x51` ("Gen1_Adv"), 727 bytes — per-LED addressing of individual segments, Report `0x90`+port / `0xB0`+port ("Gen2"/"Gen2_Adv") — newer strip model, ⚠️ Safety-relevant finding: the `Store` flag is set on every write, Upstream OpenRGB bug found while cross-checking: `FeaturePacket_PerLED_761` is 727 bytes, not 761, Verification against the existing capture template (+1 more)

### Community 6 - "Mystic Light Network Interfaces"
Cohesion: 0.25
Nodes (8): 1. TCP 26820 — local HTTP "Web API" (`MLModule.dll:CWebAPI.cs`), 2. UDP 26822 — mobile app pairing (`CBaseServer.cs`/`CBaseFN.cs`), 3. UDP 26823 — "party sync" between multiple PCs (`CSyncServer.cs`/`CSyncFN.cs`), 4. UDP 61337 — SignalRGB bridge ("LEDKeeper 2" service), Live verification (2026-10-17): the whole MLModule network layer is dead code, Mystic Light — local network interfaces, Protocol (confirmed via code, extends the `Msi.js` reconstruction), Still open

### Community 7 - "Mystic Light System Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 8 - "BuildReport0x51"
Cohesion: 0.13
Nodes (5): Task 12: Application wire-up and first real-hardware verification, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest

### Community 9 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 11 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 12 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

## Knowledge Gaps
- **123 isolated node(s):** `r`, `g`, `b`, `Off`, `Wave` (+118 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 136 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **2 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Completed: first v1 increment (2026-10-23)` connect `README.md` to `BuildReport0x51`?**
  _High betweenness centrality (0.287) - this node is a cross-community bridge._
- **Why does `BuildReport0x51()` connect `BuildReport0x51` to `MsiGen1AdvArea`, `File Structure`, `README.md`?**
  _High betweenness centrality (0.239) - this node is a cross-community bridge._
- **Are the 2 inferred relationships involving `MsiGen1AdvArea` (e.g. with `Task 2: MSI wire-protocol encoders` and `Task 6: MSI motherboard driver`) actually correct?**
  _`MsiGen1AdvArea` has 2 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `MsiZoneSettings` (e.g. with `Self-Review` and `Task 2: MSI wire-protocol encoders`) actually correct?**
  _`MsiZoneSettings` has 3 INFERRED edges - model-reasoned connections that need verification._
- **Are the 2 inferred relationships involving `MsiLightingMode` (e.g. with `Self-Review` and `Task 2: MSI wire-protocol encoders`) actually correct?**
  _`MsiLightingMode` has 2 INFERRED edges - model-reasoned connections that need verification._
- **Are the 6 inferred relationships involving `MsiColor` (e.g. with `Self-Review` and `Task 2: MSI wire-protocol encoders`) actually correct?**
  _`MsiColor` has 6 INFERRED edges - model-reasoned connections that need verification._
- **What connects `r`, `g`, `b` to the rest of the system?**
  _123 weakly-connected nodes found - possible documentation gaps or missing edges._