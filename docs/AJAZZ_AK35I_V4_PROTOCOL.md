# AJAZZ AK35I V4 — native RGB integration specification

Research date: 2026-11-25. Target: Lighting Studio on Windows 11, native USB HID. Implementation handoff for Claude Code. This is a research specification, not a declaration of shipping hardware support.

## 1. Scope and evidence

Primary goal: expose the 105 independently addressable keyboard LEDs to Lighting Studio's existing software-rendered canvas/effect pipeline, using the manufacturer's real-time RGB transaction (04 20). Secondary goal: native firmware effects, persistent per-key colors, optional readback. TFT, key remapping, macros, firmware update and wireless configuration are separate research, not required for RGB MVP.

Evidence labels: VERIFIED = actual AK35I V4 response or source configuration directly inspected; CAPTURED = observed vendor-driver packet exchange on this device, not a new implementation hardware test; INFERRED = interpretation requiring validation; REFERENCE = another model, never sufficient for V4 support; PROPOSED = implementation decision. A successful START handshake proves protocol family access, not full compatibility. Do not silently promote inferred fields or another model's capability to verified.

This is the proprietary (non-QMK) ISO keyboard, sold as AK35I V4 / AK35I V4 Tri-Mode. Vendor installer mislabels it AK45I V4; do not infer hardware identity from installer name. Owner tested wired USB and 2.4 GHz typing after update. Vendor firmware changed USB bcdDevice 0102 to 0109; the owner recovered the wireless pairing after update. No further flash is necessary or authorized. RGB capture fixtures were acquired before the firmware update unless independently proven otherwise; repeat RGB validation on 0109. OpenKey USB handshake was obtained after update. No native Lighting Studio output has yet been hardware-tested.

Implementation status (2026-11-26): Phase A done offline. Encoder and ACK check
(`src/protocol/AjazzAk35iV4Protocol.*`, test `ajazz_ak35i_v4_protocol`), scripted fake HID, driver branch in
`lighting.peripherals` (API 3), profile `profiles/experimental/ajazz-ak35i-v4.json`, 105-key component,
dry-run review gate. Model selection (D1): Edit > Settings "AJAZZ AK35I V4 keyboard (experimental, I own
this exact model)", default off, takes effect after a restart; the editor loads the experimental profile
only then. The service always loads it (it runs as LocalSystem and cannot read the editor setting) and
drives only channels a project assigns (test `experimentalProfileInactiveByDefault`).

Phase B hardware test 2026-11-26 (owner present, wired USB, firmware 0109, one step at a time, hidapi
x64 from a probe script): exactly one FF13:0001 collection (interface 3). START `04 18` -> 65-byte GET
`04 18 00 01` + zeros (65-byte GET verified on Windows hidapi; the reply carries no model identity, so
START cannot tell V4 from AK820 / V3 MAX). One Direct RGB frame (header `04 20 .. 08`, eight chunks,
`04 02`): both ACKs as documented; owner confirmed Esc red, F1 green, F2 blue (~25 %), other keys dark.
Output through Lighting Studio works (owner, wired). Stress test (probe script, service channel paused):
a full transaction takes ~36 ms (about 27 frames/s at most); the keyboard periodically answers ACKs with
status 00 (re-reading after a few ms often clears it) or FF for up to ~2 s, then recovers by itself. The
driver skips busy frames instead of failing them (fault only after 150 in a row); before, each busy
answer faulted the output and the reconnect left the keys black for ~3 s.
Not yet tested: full key map on 0109, 2.4 GHz dongle
(0C45:FEFE, needs its own capture) and Bluetooth (no data).

## 2. Identity, transport, exact matching

| Property | Wired | Receiver |
|---|---|---|
| VID:PID | 0C45:8009 | 0C45:FEFE |
| Product | USB KEYBOARD | 2.4G Dongle (some API reports blank) |
| OpenKey | sonix-feature, VERIFIED handshake | no matching adapter, LOW confidence |
| RGB support | candidate | unknown / disabled |

VID/PID is shared by other AJAZZ keyboards including AK820 MAX PLUS and AK35I V3 MAX. Do NOT automatically assign V4 from this alone. Select explicit user-approved model/profile, validate exact path and descriptor before writing, and reject ambiguous physical instances. An identical descriptor fingerprint is not proof of exact model. Product name and bcdDevice are supplementary, not unique. OpenKey does not expose bcdDevice on WebHID.

Observed WebHID collections (lengths exclude report-ID byte):

| Usage page:usage | Input | Output | Feature | Role |
|---|---:|---:|---:|---|
| FF13:0001 | ID0 64 | ID0 64 | ID0 64 | configuration, RGB |
| FF68:0061 | ID0 64 | ID0 4096 | none | bulk TFT data |
| 000C:0001 | ID3 2 | none | none | consumer control |
| FFFF:0001 | ID5 3 | none | none | unknown |

Fingerprint: `0C45:8009#000C:0001[i3:2|o|f] FF13:0001[i0:64|o0:64|f0:64] FF68:0061[i0:64|o0:4096|f] FFFF:0001[i5:3|o|f]`.

Native control interface observed as MI_03; WebHID collection enumeration indices are not Windows interface numbers. Select FF13:0001 by descriptors and exact path, not generic open(VID, PID), not keyboard input, not consumer control. RGB does not need to open FF68. In particular do not GET_FEATURE on TFT collection (V3 reference warns of firmware failure). On Windows hidapi feature buffers use **65 bytes**, byte 0 = report ID 0, bytes 1..64 = payload. All offsets in this document are zero-based **payload** offsets, NOT native buffer offsets. Check actual return length and report ID; do not silently accept truncated reads. Verify IHidDevice semantics in current code before writing wrapper. Do not use interrupt output instead of feature SET simply because sizes match.

Discovery must be descriptor-only; no feature reads/writes in --scan, --inspect, constructors, acceptsProfile, or routine hotplug enumeration. Explicit probe only with consent and exact profile. No generic fallback to 0C45:FEFE. Bluetooth input works independently of configuration; no claim of Bluetooth RGB support.

## 3. Protocol primitives

All control packets below are 64 payload bytes, zero padded unless specified. Feature SET and GET through FF13. Command prefix 04, opcode at byte 1. Typical command acknowledgment echoes 04/opcode and byte 3 = 01. Some replies contain additional nonzero bytes, particularly 04 02: validate prefix/status, not entire zero-padded reply. Do not assume transport success means firmware success. Data reports need not start 04. Never parse a data packet by opcode heuristics; use transaction state.

| Opcode | Purpose | Evidence |
|---|---|---|
| 04 18 | START | VERIFIED via OpenKey |
| 04 13 | prepare global lighting; byte 8 = 1 | CAPTURED |
| 04 20 | real-time RGB; byte 8 = 8 | CAPTURED |
| 04 23 | persistent color table; byte 8 = 9 | CAPTURED |
| 04 02 | transaction completion/status | CAPTURED; exact semantics INFERRED |
| 04 F0 | end of persistent transaction | CAPTURED; persistence semantics INFERRED |
| 04 F5 | read per-key table; byte 8 = 9 | CAPTURED |
| 04 28 | clock-related command | research only |
| 04 72 | TFT upload | research only |

START request: 04 18 + 62 zero bytes. Response: 04 18 00 01 + 60 zero bytes. OpenKey 0.2.0 on Windows: passive GET all zero at t=9ms (2.3ms call); START SET t=47ms (2.0ms); GET ACK t=87ms (2.0ms). These are call durations, not sustainable frame rate. The vendor XML cmd_delaytime=35. Captured persistent transactions use ~35–40ms intervals; real-time path is much faster (see fixtures). Timing is empirical, not a guarantee. Use bounded waits, monotonic clock, no busy loop; start conservatively and tune only after hardware testing.

## 4. Direct RGB (MVP)

The manufacturer's music mode repeatedly submits frames with the following structure. See companion fixture JSON, two consecutive transactions. **Use all eight data reports, even though report 8 is entirely zero.** Each data report holds 16 four-byte records [LED ID, R, G, B]. Concatenate 105 records in manufacturer XML order (not numeric ID order, not USB usage order, not row/column sorted), then 23 zero records (92 bytes), total 512 bytes. LED IDs sparse, up to 123; no dummy records between valid LEDs. Record channels are interpreted RGB from other color captures; confirm physical RGB and single-key mapping on 0109. Global brightness adjustment should initially scale software colors exactly once in the existing renderer, not write persistent brightness per frame.

Transaction:
1. SET header 04 20 00 00 00 00 00 00 08; pad to 64.
2. GET ack 04 20 00 01 ... 08 at offset 8.
3. SET each of eight consecutive 64-byte color chunks in order; no intermediate GET in captured sequence.
4. SET 04 02 + zeros.
5. GET ack 04 02 00 01 ...; only then regard frame as sent.

In the first fixture, header->ACK ~8.36ms, first data immediately after ACK, data SETs spaced ~4–6ms, last data->04 02 ~9ms, final ACK ~8.69ms later, total ~57.36ms. Next frame begins immediately afterward (observed approximate ~15–17 frames/s, not a tested upper bound). START may be needed for initial arming, but not per frame in consecutive capture; verify initial entry and any exit conditions from full capture before hardware enablement. Do not add 04 F0 or a persistent custom-table write per frame. Do not infer that volatile behavior is guaranteed merely because OpenKey labels it SAFE_VOLATILE for a different device; verify state after disconnect, mode switch and power cycle. Do not transmit a frame on connection construction.

Pacing: use existing output worker and latest-frame replacement, not a separate queue, timer or GUI blocking path. A frame transaction is indivisible relative to other operations on the same physical device. Start at ~10–15 fps maximum, with measured per-command timing, do not catch up missed frames. A worker that waits 33ms after completing a ~57ms transaction is naturally slower than 15fps; do not rewrite the global scheduler for this device. Optimize only after timing measurements. On STOP no implicit black frame, reset or persistent restore; follow repository output semantics. A partially failed transaction is not replayed blindly; fault the endpoint, close handle, revalidate and rearm only on explicit/current host retry policy. Never automatically fall back to persistent 04 23.

## 5. Persistent global lighting (secondary, explicit action)

Captured sequence from 03-brightness.pcapng:
START SET/GET, 04 13 with byte8=01 SET/GET, raw lighting payload SET, 04 02 SET/GET, 04 F0 SET. Optional subsequent 04 F5 readback. No additional ACK for raw payload or F0 in this fixture. Preserve this distinction. Use captured conservative timings; do not use the fast frame timings here.

Raw lighting payload:
| Payload offset | Meaning |
|---:|---|
| 0 | effect code, 00 off, 01 static, 02..13 hex (2..19 decimal) other effects; 80 custom |
| 1..3 | R,G,B |
| 4..7 | zeros observed |
| 8 | color/multicolor flag candidate; verify per effect |
| 9 | brightness 1..6 from XML and captures; 0 behavior unknown |
| 10 | speed 1..6 from XML; 0 present in custom payload |
| 11 | direction candidate, not verified |
| 12..13 | zeros observed |
| 14..15 | AA 55 |
| 16..63 | zero |

Example static blue brightness2 speed3: 01 00 00 FF 00 00 00 00 00 02 03 00 00 00 AA 55, remaining 48 bytes zero. Device returns 04 02 00 01 04 02 ... in that capture. Custom mode at 80: 80 00 00 00 00 00 00 00 00 06 00 00 00 00 AA 55, remaining zero. XML default mode 11, default brightness6, speed3. Do not expose native modes whose names and parameter semantics have not been verified against V4 UI. Use explicit opt-in for persistent settings, no effect write during discovery. Off behavior and restore defaults must be separately validated; do not equate stop playback with off.

## 6. Persistent per-key table and readback (secondary)

Different layout from 04 20! The table uses numeric slot index, 0..127, eight reports with 16 slots each, then ninth report containing zeros and AA 55 at payload offsets 62,63. For occupied LED ID i, record at absolute byte offset 4*i is [i,R,G,B]; holes and slot0 all zero in captured WRITE. Do not fill absent LED slots with synthetic IDs. The ninth report is not optional. Highest occupied ID 123. Captured Esc red table first records: 00 00 00 00 01 FF 00 00 02 00 00 00 ... . The XML ordered layout and the numeric table must be separate encoders backed by the same mapping.

Captured sequence in 05-per-key-rgb.pcapng: first global transaction selects mode 80 and ends F0; then START SET/GET; 04 23 SET/GET (byte8=09); eight table reports plus ninth tail; 04 02 SET/GET (response prefix 04 02 00 01 8B 1B); F0 SET. ~0.9s including mode selection. No implicit persistent write in a software-rendered frame.

Readback (03-brightness.pcapng): 04 F5 SET with byte8=09; GET ACK echoes F5, byte3=01, byte8=09; then nine sequential GETs (first ~10ms after ACK, later ~5ms apart). In READ data, even unused numeric slots may contain their slot ID (e.g. 0E 00 00 00); ignore unoccupied IDs using map. Last read contains slots 128..143, not a write-tail sentinel. Validate actual complete data length and ID ordering; do not assume the read tail matches write tail. Captured readback blue intensity 0x5F despite global RGB FF with brightness 2; this suggests scaling/state differences, not a confirmed scaling formula. F5 is not a backup of all device settings. Use readback for validation, never automatic full-state restore.

## 7. LED layout

105 physical LED records extracted from rgb-keyboard.xml. Full map in companion fixture. Source key_index and light_index should not be presumed equivalent for other profiles. The manufacturer XML uses US legends for some physical ISO-DE positions; translate display labels independently from wire IDs. HID usage 0x1D is physical Z position under US layout, labeled Y on DE keyboard; never derive mapping from localized character. ISO extra key usage 0x64 ID111, ISO Enter ID85, Esc ID1, space ID94, Backspace ID103. One LED per physical key. Knob/TFT are not additional LEDs. Preserve rect coordinates from XML for visual component geometry (not calibrated millimeters). 105 entries must be unique; assert no duplicate LED IDs, exact count and correct range. Normalize center positions for canvas or follow current component conventions. Enter rectangle is approximate; do not change LED ID to improve visual layout.

## 8. Integration into existing Lighting Studio

Repository inspected on master: src/hid/HidApiDevice.h, src/modules/IDriverModule.h, docs/OUTPUT_SESSION.md, module listing, CLAUDE.md. Read current files again before coding; docs can drift. Follow CLAUDE.md graphify workflow (query when graph available; update after code changes), project instructions and existing tests.

Relevant paths:
- src/hid/HidApiDevice.{h,cpp}, IHidDevice: existing exact path open, sendFeatureReport/getFeatureReport; no need for new USB library or kernel driver.
- src/modules/IDriverModule.h: IDriver::pushColors(channel, QVector<RgbColor>), setNativeMode, discoverLeds; acceptsProfile and create must not perform I/O.
- src/modules/ModuleRegistry.*, src/output/OutputSession.*, docs/OUTPUT_SESSION.md: typed frame validation, profile/identity, worker lifecycle, cache behavior.
- modules/peripherals/PeripheralModule.cpp, module.json, src/protocol/RoccatVulcanProtocol.*, profiles/vulcan-*.json, components/peripheral-vulcan.json: existing keyboard integration precedent; inspect exact current conventions, do not assume Roccat protocol.
- docs/HARDWARE_MODULES.md, HARDWARE_SUPPORT_STRATEGY.md, LIGHTING_EDITOR.md, WINDOWS_SERVICE.md, root CMakeLists and test targets: check profile schema, packaging, service discovery, component mapping and tests.

Proposed surgical design: dedicated AJAZZ protocol encoder/transaction implementation inside the existing peripheral module if compatible with its structure (otherwise justify a small separate module). Add exact V4 profile and keyboard component/105-LED channel. Keep HID framing and semantic packet generation separate so recording transports can test native 65-byte buffers. IDriver pushColors validates exact count/channel, maps colors through ordered IDs, emits transaction, returns false on invalid reply/short transfer. No extra background thread, no web browser dependency, no vendor DLL, no PawnIO requirement, no flashing. Use existing endpoint identity, conflict guard and worker scheduling. Ensure service and portable modes both use correct path; service must not probe or write a newly recognized ambiguous device automatically. If native effects are not implemented yet, advertise software-only rather than fake native support. Existing session stage validation must reject unsupported modes. If existing IHidDevice bool return loses short-write information, inspect implementation and adjust narrowly only if required with regression tests. No repository-wide refactoring.

Device ownership: close AJAZZ DeviceDriver, OpenKey session, other RGB controllers before test; use existing conflict resolution, no process termination without user choice. The device may have several paths; use FF13 only for MVP, pair via physical container only when required for later TFT. Handle unplug, sleep/resume, hotplug with host lifecycle, not permanent stale path. No multi-device broadcasting based on VID/PID. On invalid count/identity, no writes. On partial transaction, fault rather than send speculative recovery commands. Use existing automatic retry/Stop policy, avoid retries that bypass user opt-in or repeat persistent writes.

## 9. Tests and acceptance gates

Phase A (offline only): add actual recording fake-HID test against fixture bytes. Test constructor/discovery zero traffic, profile reject wrong usages/PID/ambiguous model, exactly 105 colors, all-zero and captured nonzero frame, ordering, chunk size and zero padding, report-ID prefix, exact header/ACK/completion order. Check native buffers length65. Reject short GET, wrong opcode, bad status, wrong count, wrong channel, disconnect, I/O failure at each step; no remaining sends after failure. Test successive frames and latest-frame replacement. No sleeping in unit tests; inject pacing seam only if necessary. Compare fixture fields independently, avoid expected values derived by the encoder under test. Run current CMake/CTest tests and packaging checks where feasible. Graphify update as project requires.

Phase B (owner-approved hardware test): exact wired profile, no other software; inspect-only enumeration first; then one START; verify ACK. Send low-brightness single frame with known pattern (e.g. Esc red, F1 green, F2 blue; all other keys dim/off only after consent); verify physical color channels, mapping and no typing disruption. Next test ISO extra key, Enter, space, arrows, numpad and sparse/high IDs. Measure full transaction times and start/stop/resume, no cumulative delay; test disconnect/reconnect and ordinary typing. Compare 0109 captures to pre-update fixtures. Confirm effects survive or revert as expected on Stop, USB unplug and power cycle; do not claim volatile until tested. Test service and portable output, multiple active Lighting Studio devices. Keep model experimental until these pass. Document actual results and any failed ACKs without logging keystrokes.

Phase C: only after direct RGB works, consider 04 F5 read-only retrieval, native global static and persistent custom mode; persistent writes require separate consent. No TFT, macro, remap, bootloader, firmware erase or factory reset in this implementation. No 2.4 GHz support until a new native capture and descriptor inspection prove appropriate protocol and report IDs. Do not apply AK820 receiver ping blindly.

Definition of done (MVP): existing editor can assign 105 physical keyboard LEDs to software RGB; correct model selected explicitly; correct color/position on hardware; worker output stays responsive and bounded; tests pass; no unrelated code changes; docs record measured device results and limitations. Before hardware approval, only offline protocol implementation and tests can be declared done.

## 10. Other functions (reference only)

Vendor XML: screen width240 height135, RGB565 little endian observed, 64800 bytes/frame, gif_headlength256, gif_maxframes135, gif_count1. TFT command 04 72 with block count at offsets8–9 LE, data chunks4096 on FF68, ACK input 01 5A 02 observed. Header first byte frame count, following delay bytes, FF padding; delay 2ms/unit inferred from V3 and matching 05 for 10ms GIF frame; slot field and last block/padding still need validation. Clock-related 00 01 5A date/time packet observed; do not implement from this incomplete description. Generic OpenKey profile advertises 128x128 and 255 frames; wrong for V4, never use it for uploads.

OpenKey manifest labels many other functions WRITE_ONLY (keymap, Fn, macros, global effects, sleep, locks); UI mirrors last write, not hardware state. Readback only documented for per-key colors on AK820, not a complete backup. No firmware/bootloader/erase commands in OpenKey adapter. Battery via receiver 20 01 response byte index varies across references, not verified for V4. QMK/VIA FF60:0061 collection absent; normal browser key tester not proof of VIA. Declared USB interval 1ms in generic AK820 profile is not measured V4 polling or RGB rate.

## 11. Provenance and open items

Primary local source files (owner retains originals; not committed to public repository): rgb-keyboard.xml, config.xml, 03-brightness.pcapng, 04-effects.pcapng, 05-per-key-rgb.pcapng, 06-music-mode(1).pcapng, 07-tft-static.pcapng, 07-tft.pcapng, fwupdate(3).pcapng. Original manufacturer binaries and Procmon logs are not needed for RGB MVP and must not be published. Raw USB captures may contain input or unrelated devices; only selected RGB feature payloads and derived layout facts are included in companion fixtures. OpenKey USB diagnostic text and trace supplied by owner: version0.2.0, Windows, FF13/FF68, START ACK. Earlier receiver trace empty, support bundle 0C45:FEFE only consumer/system input collections. Packet numbers in fixtures refer to original capture numbering; timing relative to first packet. No claim that payloads were captured on new Lighting Studio implementation.

External references (read for comparison, not license-free copy):
- https://github.com/BakulBd/OpenKey — sonix-feature adapter, fixtures/device-info/ak820-max-plus-start-handshake-hw.json, fixtures/rgb/ak820-max-plus-read-per-key-hw.json, docs/reverse-engineering/official-driver-analysis.md; exact references from manifest. AK820 MAX PLUS fw1.07 hardware read/handshake verified; writes experimental.
- https://github.com/dantje/ajazz-ak35i-v3-max — V3 MAX protocol reference, PROTOCOL.md if present; compare offsets, table lengths, TFT timing; do not assume identical.
- https://github.com/Aiacos/ajazz-control-center — native C++ HID approach, other models; compare transport conventions and clock/status.
- Verify current source and license before importing any external code. Prefer independent implementation from owner captures; record provenance/required notices. No need to fork any project for Lighting Studio MVP.

Open items: exact model discrimination across shared PID; initial Direct RGB arming/exit and minimum delays; V1.09 parity; physical channel order, intensity scaling, all 105 positions; native effect names and flag semantics; readback behavior on V4 after flash; wireless control descriptors; TFT details. The fixtures are sufficient to build a candidate offline encoder, not to bypass hardware acceptance.
