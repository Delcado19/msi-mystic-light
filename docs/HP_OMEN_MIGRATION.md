# HP Omen 30L migration: first independent-module increment

Date: 2026-10-30. Status: implemented and transport-double tested, not
capture-validated or physically verified. No real device was opened or written.

## Pinned reference comparison

OpenRGB reference: local build/research/openrgb, commit
aac7e5511439e020ddae88e78bd400c71b8d4a9c. Inspected detector, controller,
header and RGB controller under Controllers/HPOmen30LController/.
Upstream: https://gitlab.com/CalcProgrammer1/OpenRGB.
The reference and our adapted module carry SPDX GPL-2.0-or-later.

SignalRGB reference: local build/research/signalrgb-plugins, beta commit
9511979fa21a0a00a35a236d0aac8c438179094f (442 tracked plugin paths).
Filename/content searches for Omen 25/30 and PID 84FD in the available plugin
and add-on snapshots found no corresponding implementation. This is not proof
of absence in all releases. No second implementation was available to reconcile;
compare any matching plugin found later before extending support. No SignalRGB
source was copied.

## Selected protocol and scope

- HID identity 103C:84FD; exact descriptor matching, no bus probing.
- API 2 output reports, 58 bytes; no feature-report writes.
- Seven independently colored zones, one shared RGB value per zone: logo, bar,
  front fan, CPU cooler, bottom/middle/top fans (zone IDs 1..7).
- Canvas uses Direct 04, zone-indexed ARGB slots and intensity 100.
- Native modes: Static 01, Off 05, Breathing 06, Color cycle 07, Blinking 08,
  Wave 09 and Radial 0A. Static requires one color; animations accept 1..6,
  transmitted one packet per palette entry.
- Host brightness 0..5 maps to 0..100 in steps of 20. Host speed 0..2 maps
  to upstream slow/medium/fast values 1..3. Physical behavior is unverified.
- Powered-on bank 1 only. Suspend bank 2 and predefined themes are not exposed.
- Constructors perform no I/O. Topology read-back remains Unknown. Seven listed
  zones do not prove seven populated physical outputs.
- No save/reset/firmware command is exposed. Persistence after disconnect has
  not been verified; do not infer volatile behavior from the absence of a save.

Static packets use RGB stride 3; Direct packets use ARGB stride 4. Exact packet
tests cover both, including the final zone to detect indexing errors. Unlike
upstream's repeated fan-name assignment, each profile channel has a distinct
name. Validate color counts rather than reproduce unchecked vector indexing.
Reject inward direction instead of silently ignoring it. A failed palette
write stops further packets and reports failure; it cannot roll back packets
already accepted by the device.

## Integration and qualification

Independent hp_omen_module DLL, ID hp.omen30l; profile hp-omen-30l.json.
The visible profile name explicitly marks hardware as unverified. Existing
profiles and document schema are unchanged. The current host already supports
API 2; no host ABI extension is needed. The portable whitelist includes the DLL. Installer copy/hash lists and the
protected service allowlist include all six bundled modules, fixing the older
three-module installer restriction. Update host, modules and installer together;
do not copy only a new host into an older installed tree.

Tests cover profile validation, every native mode, exact packets, all seven
Direct zones, no constructor writes, output-only transport, closed devices,
invalid effects, failed palette writes and identity mismatch. The module has no
Windows-specific code, but Linux compilation, permissions and physical output
remain unqualified.

Rollback: remove the HP profile and corresponding module together from a stopped
portable installation. Other profiles/modules are unaffected. A profile without
its required module deliberately fails catalog validation. Older hosts lacking
API 2 must reject the module metadata.

Next physical qualification requires an HP Omen owner: model/firmware, populated
zones, color order, native modes, brightness/speed and reconnect behavior. Show
exact intended bytes before the first device write. Manufacturer peripheral
settings and THX research remain separate, unimplemented work items.
