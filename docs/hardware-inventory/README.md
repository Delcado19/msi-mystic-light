# Hardware migration inventory and priorities

Snapshot: 2026-10-30. This is a complete file/family inventory of the two pinned
public reference trees and a metadata inventory of selected installed vendor
sources. It is not a complete semantic model/capability audit or a promise that
each listed device works. No drivers or device settings were changed here.

## Scale and counting units

| Source/unit | Count | Meaning |
| --- | ---: | --- |
| OpenRGB controller directories | 198 | Work packages, not necessarily one protocol/module each |
| Current partial source-family overlaps | 9 | Covered only for selected models/features through our six modules |
| OpenRGB directories without our implementation | 188 | Research/port candidates, not a required new-DLL count |
| Excluded synthetic OpenRGB DebugController | 1 | Not hardware |
| OpenRGB registration records | 2183 | Includes nonliteral labels and conditional variants |
| OpenRGB model labels within families | 1975 | Registration labels; not unique verified physical models |
| SignalRGB plugin JavaScript files | 440 | Includes dynamic identifiers and possible shared helpers |
| SignalRGB data-only component files | 218 | LED geometry/product data; not drivers |
| SignalRGB files with statically resolved VID/PID | 286 | Literal scalar return values only |
| SignalRGB files with candidate OpenRGB USB matches | 186 | Same USB pair does not establish the same model/interface/protocol |
| SignalRGB files with unresolved identity/name exports | 154 | Explicit manual-analysis queue; not silently skipped |
| Installed SteelSeries descriptor files | 489 | Filename hints only; descriptor envelopes not decoded |
| Installed Roccat module-file candidates | 3 | Kone AIMO, Vulcan and smart_key; the latter is not a device model |
| Installed Razer/G-Menu application packages | 1 each | Model catalogs not yet decoded |

The earlier motherboard estimate counted 22 source files mentioning
DEVICE_TYPE_MOTHERBOARD, 21 without Debug, spread across 16 real controller
folders. It did not establish 21 independent future modules. MSI generations and
ASRock SMBus generations are examples of several implementations in one folder.
Some families, such as ENE, span categories; our RAM path does not establish
motherboard coverage for the same family.

## Browse the full list

- [All 198 families, priorities and current coverage](FAMILIES.md).
- [OpenRGB family spreadsheet](openrgb-families.csv).
- [Every OpenRGB registration, raw selectors and resolved USB pairs](openrgb-registrations.csv).
- [All SignalRGB plugin files and unresolved fields](signalrgb-plugins.csv).
- [Cross-source comparison candidates](comparison-candidates.csv).
- [SignalRGB components requiring data conversion](signalrgb-components.csv).
- [Installed manufacturer source index](manufacturer-sources.csv).
- [Full structured reference inventory](hardware-inventory.json).
- [Installed-source hashes and scope](manufacturer-sources.json).

## Prioritized migration queue

| Priority | Work | First deliverable / completion evidence |
| --- | --- | --- |
| P0 | Roccat, Razer, SteelSeries families already in use | Exact owned-model comparison against both sources, installed-module capabilities and a settings matrix; preserve current RGB support |
| P0 | Packaging and qualification of existing six modules | Keep profile/module/installer/service lists aligned; distinguish tests from real-device results |
| P1 | Other MSI motherboard generations, Asus Aura USB, Gigabyte Fusion2 USB, JGINYUE V1/V2 | Exact model/transport comparison, bounded own module/profile and packet fixtures; no automatic hardware writes |
| P1 | Existing ASRock/Lenovo/HP implementations | Resolve missing reference behaviors and recruit device-owner verification; do not mark native-zone support as per-LED support |
| P1 | Owner AGON monitor / G-Menu | Identify exact monitor, IDs and transport, map LED presets and other exposed settings before selecting a protocol |
| P2 | Remaining HID/USB, serial and network device families | Select one bounded protocol at a time, reconcile overlaps and add independent modules with exact profile scope |
| P2 | SignalRGB data-only components | Convert geometry/wire order separately; do not count these as hardware drivers |
| P3 | Wider SMBus, Super I/O and EC access | Platform-specific transport design and qualification before a device port; do not copy unrestricted address sweeps/remapping |
| Separate audio track | THX Spatial Audio | Identify headset, installed audio processor/driver and available integration; not an RGB device-driver task |

Initial automatic OpenRGB triage: EXCLUDED=1, P0=3, P1=15, P2=141, P3=38.
These are work priorities, not assertions of safety or estimated implementation
cost. Transport hints come from detector macros/API tokens and require review.
The serial/HID paths are not automatically Linux-qualified; Windows-only APIs,
permissions, reconnect and Linux transport behavior need separate evidence.

For every exact model supported by both references, the mandatory comparison
covers detection/interfaces, initialization, packets/checksums, read-back,
LED topology/order, limits, native modes, timing, persistence and shutdown.
The candidate CSV is an input to that comparison, not its result. In particular,
Vulcan 100 and Apex 3 have literal cross-source candidates. A missing public
SignalRGB plugin is not proof that the installed SignalRGB app lacks support.

Manufacturer settings work includes motion/acceleration, DPI, button/key mapping,
DCU/lift-off, angle snapping, polling rate, macros/repeats, character repeat,
blink rate and all remaining exposed options. Record whether each behavior lives
in firmware, a host service or OS preferences. None of those non-RGB controls
has been implemented by producing this inventory.

## Source provenance and limitations

- OpenRGB: aac7e5511439e020ddae88e78bd400c71b8d4a9c.
- SignalRGB plugins/components beta: 9511979fa21a0a00a35a236d0aac8c438179094f.
- Reference trees were read from Git objects. The SignalRGB checkout was sparse
  and lacked promisor blobs; those pinned objects were downloaded before the
  complete inventory. A working-directory-only search is insufficient.
- Extraction does not execute C++ preprocessors or JavaScript. Dynamic model
  tables, aliases, subdevices, interface checks and computed identifiers remain
  manual work. Ambiguous constants are not guessed. A static macro can register
  several aliases or use a label unrelated to a retail model.
- Manufacturer index records filenames, sizes and SHA256 only. Proprietary code,
  databases, credentials and personal device settings are not redistributed.
  SteelSeries .edevice samples have BEGIN DESCRIPTOR envelopes; contents remain
  undecoded. Roccat module names alone do not prove full device-family support.
- REA static Razer package analysis reported an integrity mismatch in an unpacked
  bluetooth_hci_socket.node companion. This alone does not diagnose corruption
  or malicious modification. No capability claim is based on that package.
  G-Menu's initial bounded REA analysis timed out. Follow-up package analyses
  yielded no usable result before the caller was stopped; both catalog audits
  remain incomplete. No applications were executed or software reconfigured.

## Reproduction and next decisions

Run `python scripts/inventory_hardware.py` with the pinned reference Git trees in
build/research/openrgb and build/research/signalrgb-plugins. Missing Git objects
fail visibly; fetch the required pinned objects before rerunning offline.
The command regenerates reference JSON/CSV files, not the handwritten plan or
installed-vendor snapshot. Vendor metadata was gathered from the owner-provided
installation roots, using SteelSeries apps/engine/deviceSpecifications files,
Roccat data/*/*.dll and Razer/G-Menu app.asar files, without decoding or execution.
Hashes in manufacturer-sources.json bind those observations to installed bytes.

The remaining module count cannot yet be deduplicated reliably. Start with P0,
then select the next P1 protocol after its exact-model comparison. Do not start
188 ports in parallel or assume one DLL per upstream folder. Follow
[the hardware strategy](../HARDWARE_SUPPORT_STRATEGY.md).
