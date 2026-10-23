# SignalRGB GitHub topic triage

Reviewed 2026-10-23. The topic page returned HTTP 429; the public GitHub
repository search API (`q=topic:signalrgb`, up to 100 results) supplied the
inventory. This is a relevance triage, not a complete source/security audit.
README descriptions are upstream claims, not verified runtime behavior.

## Priorities

For the subsequent source-level comparison against our implemented backend and
accepted editor scope, see [Reference reuse review](REFERENCE_REUSE_REVIEW.md).
It distinguishes recommended Qt reimplementations, existing functionality,
deferred integrations and direct-source reuse limits.

| Project | Value to this app | Recommended action |
| --- | --- | --- |
| [SignalForge](https://github.com/flowerpot-byte/SignalForge) | Preview-first effect editor, layers, shared preview/export engine | Follow the separate editor review; adapt concepts to Qt |
| [CompGen](https://github.com/Frubi22/compGen-for-signalRGB) | Product metadata and visual LED geometry authoring, actual SignalRGB JSON import/export | High priority reference for our component editor and a future explicit import adapter |
| [SignalBridgePlugin](https://github.com/Zayrick/SignalBridgePlugin) | C++/Qt QuickJS host, metadata extraction, discovery, endpoint sessions, topology translation | High priority architecture reference before deciding whether to implement a script backend |
| [Lumen](https://github.com/rahmidaniel/Lumen) | Device/layout UI, per-LED editing, known-process conflict detection | Inspect interaction patterns and conflict handling; retain our independent drivers |
| [LightScript Workshop](https://github.com/hyperb1iss/lightscript-workshop) | Effect parameter metadata, shader/Canvas examples, live development preview | Later reference for an effects SDK; no TypeScript/WebGL dependency now |
| [Virtual RGB Rig](https://github.com/akostkasmith/signalrgb-effects) | Hardware-free preview, movable device rectangles and LED sampling, optional 3D presentation | Borrow the layout/sampling concept first; defer 3D rendering |
| [Lueur](https://github.com/Renan-projects/lueur) | Native hardware effects, sleeping between changes, conservative identification rules | Reference for native-mode lifecycle; no SMBus or PawnIO expansion in this increment |

CompGen's `index.js` actually imports/exports `LedMapping`, `LedCoordinates`
and other PascalCase component fields. This makes it a more concrete component
format reference than the previously reviewed generic plugin skill. Its schema
is not this app's schema: conversion must be explicit and validated.

SignalBridge's README describes QuickJS execution of user-provided scripts;
`EndpointSession.cpp` uses exact paths for primary/additional endpoint handles.
`QuickJsRuntime.cpp` exposes module export calls and configuration updates.
These are relevant design examples, not proof of complete SignalRGB compatibility,
script isolation or hardware qualification. Its native OpenRGB plugin is not
loadable through this app's different Qt module contract.

Lumen's `src-tauri/src/conflicts.rs` uses normalized process names and Windows
Toolhelp snapshots. It explicitly includes SignalRGB and MSI LEDKeeper2 among
known applications. This is useful prior art for detection, but a process-name
match alone does not prove a particular device is being controlled. Our requested
PID selection and termination dialog has since been implemented independently;
see `CONTINUE.md`. This triage predates that implementation.

## Useful later or only for matching hardware

- [SignalRGB-to-OpenRGB Bridge](https://github.com/Fefedu973/SignalRGB-To-OpenRGB-Bridge)
  connects SignalRGB to the OpenRGB SDK server and requires both applications.
  It could inform an optional SDK backend; it does not replace standalone drivers.
- [signalrgb-python](https://github.com/hyperb1iss/signalrgb-python) and
  [Home Assistant integration](https://github.com/hyperb1iss/signalrgb-homeassistant)
  control a running SignalRGB installation. Useful integration/API references,
  not USB discovery implementations for this application.
- Individual keyboard, GPU, LCD, DMX and WLED bridge projects in the inventory
  are leads for specific future devices. Match hardware IDs, firmware, protocol,
  license and source evidence before porting anything. No new MSI B850 attached
  LED-discovery result was established by this triage.

## Scope, licensing and evidence

Read selected READMEs and source excerpts; did not install, build or run any
project. API metadata reports MIT for LightScript Workshop, Virtual RGB Rig and
Lumen, AGPL-3.0 for CompGen, and GPL-2.0 for SignalBridge/Lueur (their READMEs
specify or-later). These labels are an inventory aid, not a license compatibility
review. No upstream source/assets were copied into production code.

Selected source snapshots:

- SignalBridge: `570c39a7ad5b635eadae325805cc6855929bf53b`
- CompGen: `4bb27742414dd5864878c05bb04b472026ddfbe2`
- Lumen: `f021bb274980157641240abb0ae042d6181c29d7`

The snapshots identify repository heads observed during inspection; source API
requests used the default branch and were not an atomic snapshot of all repos.
