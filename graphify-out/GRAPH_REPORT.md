# Graph Report - msi-mystic-light  (2026-10-20)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 108 nodes · 106 edges · 11 communities (10 shown, 1 thin omitted)
- Extraction: 100% EXTRACTED · 0% INFERRED · 0% AMBIGUOUS
- Token cost: 1,073 input · 114 output

## Graph Freshness
- Built from commit: `a94f0a3b`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Lighting Studio Implementation
- Lighting Studio Design
- Project Scaffolding and Drivers
- Documentation and Protocols
- Standalone App Design
- MSI Mainboard LED Protocol
- Mystic Light Network Interfaces
- Mystic Light System Architecture
- Device Data and Decryption
- Skeleton Implementation Plan
- Development Tools

## God Nodes (most connected - your core abstractions)
1. `Lighting Studio Implementation Plan` - 22 edges
2. `File Structure` - 13 edges
3. `Lighting Studio — Design Doc (v2)` - 12 edges
4. `Standalone RGB Lighting App — Design` - 9 edges
5. `Mystic Light — local network interfaces` - 7 edges
6. `Mystic Light — Overall Architecture` - 7 edges
7. `MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`)` - 6 edges
8. `Standalone Lighting App — v1 Skeleton Implementation Plan` - 5 edges
9. `Canvas architecture` - 4 edges
10. `Report `0x50` ("Gen1"), 290 bytes — status and default configuration` - 4 edges

## Surprising Connections (you probably didn't know these)
- None detected - all connections are within the same source files.

## Communities (11 total, 1 thin omitted)

### Community 0 - "Lighting Studio Implementation"
Cohesion: 0.09
Nodes (22): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Self-Review Notes, Task 10: Roccat Kone Aimo driver, Task 11: Razer Basilisk V3 driver, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 13: Extend `Settings` — placement, source, global effect, profiles (+14 more)

### Community 1 - "Lighting Studio Design"
Cohesion: 0.12
Nodes (15): Anti-goals, Application structure, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals (+7 more)

### Community 2 - "Project Scaffolding and Drivers"
Cohesion: 0.15
Nodes (13): File Structure, Task 10: Qt tray app shell, Task 11: Windows autostart registration, Task 12: Application wire-up and first real-hardware verification, Task 1: Project scaffolding and test harness, Task 2: MSI wire-protocol encoders, Task 3: HID transport interface and hidapi backend, Task 4: Component data model (+5 more)

### Community 3 - "Documentation and Protocols"
Cohesion: 0.29
Nodes (4): Entwurf: Kommentar für GitLab-Issues #4935 / #5429, Contents, MSI Mystic Light — Reverse Engineering, Related repos

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

### Community 8 - "Device Data and Decryption"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 9 - "Skeleton Implementation Plan"
Cohesion: 0.40
Nodes (4): Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan

## Knowledge Gaps
- **83 isolated node(s):** `Global Constraints`, `Review Focus`, `Self-Review Notes`, `Task 10: Roccat Kone Aimo driver`, `Task 11: Razer Basilisk V3 driver` (+78 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 88 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **1 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`)` connect `MSI Mainboard LED Protocol` to `Documentation and Protocols`?**
  _High betweenness centrality (0.046) - this node is a cross-community bridge._
- **Why does `Mystic Light — local network interfaces` connect `Mystic Light Network Interfaces` to `Documentation and Protocols`?**
  _High betweenness centrality (0.042) - this node is a cross-community bridge._
- **What connects `Global Constraints`, `Review Focus`, `Self-Review Notes` to the rest of the system?**
  _83 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Lighting Studio Implementation` be split into smaller, more focused modules?**
  _Cohesion score 0.08695652173913043 - nodes in this community are weakly interconnected._
- **Should `Lighting Studio Design` be split into smaller, more focused modules?**
  _Cohesion score 0.125 - nodes in this community are weakly interconnected._