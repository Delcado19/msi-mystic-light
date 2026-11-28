# AJAZZ AK35I V4 Direct RGB (Phase A, offline) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Lighting Studio can drive the 105 keyboard LEDs of an explicitly selected AJAZZ AK35I V4 (wired, 0C45:8009) through the existing software-RGB output path, proven only by offline tests against the captured vendor bytes. No hardware I/O in this phase.

**Architecture:** A pure encoder (`src/protocol/AjazzAk35iV4Protocol.*`) builds the 64-byte payloads of the captured Direct RGB transaction (`04 20` → 8 data chunks → `04 02`) and validates ACKs. `PeripheralDriver` gains an `ajazz-ak35i-v4` protocol branch that runs one transaction per `pushColors` over the existing `IHidDevice` (feature SET + GET, 65-byte native buffers, report ID 0). One new profile and one new 105-LED component; the existing output worker, pacing and conflict handling are reused unchanged.

**Tech Stack:** C++17, Qt 6.8 (Core, Test), hidapi through `IHidDevice`, CMake/Ninja, CTest.

**Spec:** `docs/AJAZZ_AK35I_V4_PROTOCOL.md` (sections 2–4, 7–9), fixtures `docs/hardware-inventory/ajazz-ak35i-v4-rgb-fixtures.json`. Owner research: `../OpenKey/docs/reverse-engineering/ak35i-v4-owner-research.md`.

## Open decision (owner) — blocks Task 5 only

**D1: How is the V4 selected explicitly?** VID:PID 0C45:8009 and the descriptor fingerprint are shared with AK820 MAX PLUS and AK35I V3 MAX; the spec forbids assigning V4 automatically.
- **(Recommended) Experimental profile folder:** the profile ships as `profiles/experimental/ajazz-ak35i-v4.json`. Editor and service load only `profiles/*.json` (non-recursive, `src/main.cpp:99`, `src/service/main.cpp:126`), so it is inactive by default. *Edit > Settings* gets a checkbox "AJAZZ AK35I V4 (experimental, I own this exact model)"; when checked, the editor and service also load that one file. Smallest change, one switch, reversible.
- **Alternative:** a per-device confirmation in Devices ("This 0C45:8009 keyboard is an AK35I V4") that writes a local profile copy with `match.endpoint` = exact path. More precise with two such keyboards, but the path changes with the USB port (same issue as the open hotplug item).

## Global Constraints

- No hardware I/O anywhere in Phase A: no probe, no START, no frame. Tests use fakes only.
- Discovery, `acceptsProfile`, `create` and driver construction send nothing (spec §2, §8).
- Control collection only: usage page `0xFF13` (65299), usage `0x0001`, interface 3. Never open or query `FF68:0061` (TFT, GET_FEATURE may crash firmware) or `0C45:FEFE` (receiver).
- Native buffers are **65 bytes**: byte 0 = report ID `0x00`, bytes 1..64 = payload. All offsets in this plan are payload offsets.
- Direct RGB only. No `04 23`, `04 F0`, `04 F5`, `04 13`, `04 28`, `04 72`, no persistent write, no automatic fallback to persistent mode.
- Every data report is sent, including the eighth all-zero one; 105 records in manufacturer XML order (fixture `led_map.entries[].order`), then 23 zero records = 512 bytes.
- On any failed step: return `false`, send nothing further in that transaction, no replay (spec §4).
- Colors are written as given; brightness scaling stays in the renderer (spec §4).
- Licensing: independent implementation from the owner captures; do not copy code from OpenKey, `ajazz-ak35i-v3-max` or `ajazz-control-center`.
- Comments and docs in English; new code-level comments reference `docs/AJAZZ_AK35I_V4_PROTOCOL.md`.

## Review Focus

1. **ACK with extra non-zero bytes** (spec §3, especially `04 02`): must be accepted when bytes 0,1,3 = `04`,opcode,`01`; owned by Task 1 (`ackAcceptsTrailingBytes`).
2. **Short or empty GET** (`getFeatureReport` returns < 65, or report ID ≠ 0): frame fails, no further SET; Task 3 (`shortAckStopsTransaction`).
3. **Wrong color count / wrong channel**: no I/O at all; Task 3 (`invalidFrameSendsNothing`).
4. **Dry-run review gate** (`previewOutputReports`) must keep working although the transport cannot answer GETs; Task 4 (`previewAcceptsAjazzProfile`).
5. **Similar keyboards on the same VID:PID** (AK820, V3 MAX) must not pick up the V4 profile without D1 selection; Task 5 (`experimentalProfileInactiveByDefault`).

---

### Task 1: Pure encoder and ACK check

**Files:**
- Create: `src/protocol/AjazzAk35iV4Protocol.h`, `src/protocol/AjazzAk35iV4Protocol.cpp`
- Modify: `CMakeLists.txt:37-39` (add the `.cpp` to `peripherals_module`)
- Create test: `tests/protocol/test_ajazz_ak35i_v4.cpp`; register in `tests/CMakeLists.txt` as `add_test(NAME ajazz_ak35i_v4_protocol ...)` with `FIXTURE_FILE="${PROJECT_SOURCE_DIR}/docs/hardware-inventory/ajazz-ak35i-v4-rgb-fixtures.json"`

**Interfaces:**
- Produces:
  - `constexpr size_t AJAZZ_V4_PAYLOAD = 64; constexpr size_t AJAZZ_V4_REPORT = 65; constexpr int AJAZZ_V4_LEDS = 105;`
  - `extern const std::array<uint8_t, AJAZZ_V4_LEDS> AJAZZ_V4_LIGHT_IDS;` — `light_index` values in fixture `order` 0..104 (Esc=1, F1=2, …), hard-coded from the fixture.
  - `using AjazzReport = std::array<uint8_t, AJAZZ_V4_REPORT>;`
  - `AjazzReport BuildAjazzCommand(uint8_t opcode, uint8_t arg8 = 0);` → `[0]=0, [1]=0x04, [2]=opcode, [9]=arg8`, rest 0.
  - `std::array<AjazzReport, 8> BuildAjazzDirectChunks(const std::vector<MsiColor>& colors);` throws `std::invalid_argument` if `colors.size() != 105`. Record k = `{AJAZZ_V4_LIGHT_IDS[k], r, g, b}` at payload offset `4*k`.
  - `bool AjazzAckOk(const uint8_t* report, size_t length, uint8_t opcode);` → `length == 65 && report[0]==0 && report[1]==0x04 && report[2]==opcode && report[4]==0x01`; ignores all other bytes.

- [ ] **Step 1: Write failing tests** (`QTEST_GUILESS_MAIN`, Qt Test style of `tests/output/test_peripherals.cpp`). Read the fixture with `QJsonDocument`; expected bytes come from `payload_hex`, never from the encoder.
  - `lightIdsMatchFixture`: `AJAZZ_V4_LIGHT_IDS[k] == led_map.entries[k].light_index` for all 105; IDs unique, max 123; `[0]==1`, Esc; contains 111 (ISO extra key), 85 (ISO Enter), 94 (space), 103 (Backspace).
  - `headerMatchesCapture`: `BuildAjazzCommand(0x20, 8)` payload (bytes 1..64) == first `set` of `direct-frame`; `BuildAjazzCommand(0x02)` == the `04 02` set.
  - `directChunksMatchCapture`: for `direct-frame` and `direct-frame-next`, build 105 colors as black except `input_nonzero_leds` (keys are light IDs → place at the order index whose ID matches); each of the 8 chunks' payload == the 8 consecutive data `set` payloads of the capture. Chunk 8 is all zero.
  - `wrongCountThrows`: 104 and 106 colors throw.
  - `ackAcceptsTrailingBytes`: ACK `get` payloads from the fixture (both `04 20` and `04 02`) pass `AjazzAckOk`; same buffer with `[4]=0x00`, with opcode mismatch, with `length 64`, with `[0]=1` fail.
- [ ] **Step 2: Run** `build\r36-ctest.cmd`-style: `ctest --test-dir build/r36 -R ajazz_ak35i_v4_protocol --output-on-failure` → FAIL (does not compile / missing symbols).
- [ ] **Step 3: Implement** the header/source. File header comment: provenance = owner captures (fixture SHA-256s), no third-party code.
- [ ] **Step 4: Run** the same command → PASS.
- [ ] **Step 5: Commit** `feat(protocol): AJAZZ AK35I V4 Direct RGB encoder from owner captures`.

### Task 2: Scripted fake HID responses

**Files:**
- Modify: `tests/hid/FakeHidDevice.h`
- Test: `tests/hid/test_hid_backend.cpp` (new slot `fakeQueuedReads`, add it to the `hid_backend` test argument list in `tests/CMakeLists.txt:9`)

**Interfaces:**
- Produces: `std::deque<std::vector<std::uint8_t>> featureReportQueue;` and `std::vector<std::vector<std::uint8_t>> operations;` on `FakeHidDevice`. `getFeatureReport` pops the queue front when non-empty (else falls back to existing `featureReportResponse`), returns the popped size (may be shorter than requested). Every SET and GET is appended to `operations` with a leading tag byte `'S'`/`'G'` so tests can assert exact order. Existing members and behavior stay unchanged.

- [ ] **Step 1:** Test `fakeQueuedReads`: queue two responses of length 65 and 10; two GETs return 65 and 10 with the queued bytes; third GET falls back to `featureReportResponse`; `operations` is `G,G,G`.
- [ ] **Step 2:** Run `ctest -R hid_backend` → FAIL.
- [ ] **Step 3:** Implement.
- [ ] **Step 4:** Run `ctest -R "hid_backend|peripheral_reports"` → PASS (existing users unaffected).
- [ ] **Step 5:** Commit `test(hid): scripted feature-report responses in FakeHidDevice`.

### Task 3: Driver branch, profile and component

**Files:**
- Modify: `modules/peripherals/PeripheralModule.cpp` (protocol branch + `acceptsProfile`), `modules/peripherals/module.json` (`apiVersion` 2 → 3, needed for `getFeatureReport`; host accepts 3, `src/modules/ModuleRegistry.cpp:28-31`)
- Create: `profiles/experimental/ajazz-ak35i-v4.json` (location per D1; if D1 = alternative, still keep the file here as the template)
- Create: `components/peripheral-ajazz-ak35i-v4.json`
- Test: `tests/output/test_peripherals.cpp` (new slots; extend `PROFILE_DIRECTORY` use with `/experimental`)

**Interfaces:**
- Consumes: Task 1 symbols; Task 2 `featureReportQueue`, `operations`.
- Produces:
  - Profile: `id "ajazz-ak35i-v4"`, `driverId "lighting.peripherals"`, `name "AJAZZ AK35I V4"`, `match {vendorId 3141, productId 32777, usagePage 65299, usage 1, interfaceNumber 3}`, one channel `id "AjazzAk35iV4"`, `maxLeds 105`, `fixedLeds 105`, `direct true`, `nativeModes []`, `address {protocol "ajazz-ak35i-v4", componentId "peripheral.ajazz-ak35i-v4"}`.
  - Component: `id "peripheral.ajazz-ak35i-v4"`, `name "AJAZZ AK35I V4 (ISO-DE)"`, category `Keyboards`, `ledCount 105`, `mapping` 0..104 in fixture order, `positions` = rect centers (`x+w/2`, `y+h/2`) scaled by one common factor to the Vulcan component's width convention (check `components/peripheral-vulcan.json`), aspect ratio kept.
  - Driver behavior for one `pushColors` (exact order, each step must succeed before the next):
    1. first call only: `SET BuildAjazzCommand(0x18)`, `GET` → `AjazzAckOk(…,0x18)` (START; spec §4 "may be needed for initial arming"; set `initialized_` only after success)
    2. `SET BuildAjazzCommand(0x20,8)`, `GET` → `AjazzAckOk(…,0x20)`
    3. 8 × `SET` chunk (no GET between)
    4. `SET BuildAjazzCommand(0x02)`, `GET` → `AjazzAckOk(…,0x02)`
    GET buffers are 65 bytes with `[0]=0` before the call. Any failure → `return false` immediately.

- [ ] **Step 1: Failing tests** in `test_peripherals.cpp`:
  - `ajazzProfileAccepted`: `acceptsProfile` true for the shipped profile; false when `usagePage` = 65384 (FF68), `productId` = 0xFEFE, `fixedLeds` = 104, `channel.id` changed, `nativeModes` non-empty.
  - `ajazzFrameSequence`: FakeHidDevice with queued fixture ACKs (START, 04 20, 04 02); first `pushColors` with fixture `direct-frame` colors → `operations` = `S18,G,S20,G,S×8,S02,G`; each SET is 65 bytes, `[0]==0`, payload equals fixture. Second call (queue 2 ACKs) has no START: `S20,G,S×8,S02,G`.
  - `shortAckStopsTransaction`: queue a 10-byte reply for `04 20` → `pushColors` false, no data chunk sent after it; repeat with bad status byte and with wrong opcode.
  - `invalidFrameSendsNothing`: 104 colors, wrong channel id, closed device → false and `operations` empty.
  - `constructionSendsNothing`: `modules.create(...)` alone → `operations` empty.
  - Component check: load `components/peripheral-ajazz-ak35i-v4.json` via `ComponentDefinition`, 105 positions, no two positions identical, mapping is 0..104.
- [ ] **Step 2:** Run `ctest -R "peripheral_reports|components"` → FAIL.
- [ ] **Step 3:** Implement profile, component, `acceptsProfile` clause (`protocol=="ajazz-ak35i-v4" && vid==0x0c45 && pid==0x8009 && iface==3 && usage==0xff13 && channel.id=="AjazzAk35iV4"`, count 105) and the driver branch. Add a `get(opcode)` helper next to `feature()`/`output()`.
- [ ] **Step 4:** Run `ctest --test-dir build/r36` → all suites PASS.
- [ ] **Step 5:** Commit `feat(peripherals): AJAZZ AK35I V4 Direct RGB driver (offline-tested, experimental)`.

### Task 4: Review gate dry run answers GETs

**Files:**
- Modify: `src/output/HardwareOutput.cpp:7-21` (`RecordingTransport`)
- Test: `tests/output/test_peripherals.cpp`

**Interfaces:**
- Produces: `RecordingTransport::getFeatureReport` answers with the last recorded SET report, byte `[4]` set to `0x01`, same length; records nothing. Comment: dry-run acknowledgement only, so drivers that read ACKs can be serialized for the review gate; real devices are never answered this way.

- [ ] **Step 1:** Test `previewAcceptsAjazzProfile`: `previewOutputReports` with the AJAZZ profile and 105 colors returns exactly 11 reports in this order: START `04 18`, `04 20`, 8 chunks, `04 02`; each 65 bytes. Existing `reports()` slot unchanged.
- [ ] **Step 2:** Run `ctest -R peripheral_reports` → FAIL (driver gets 0-byte GET).
- [ ] **Step 3:** Implement.
- [ ] **Step 4:** Run full `ctest --test-dir build/r36` → PASS.
- [ ] **Step 5:** Commit `fix(output): dry-run transport acknowledges feature reads for ACK-based drivers`.

### Task 5: Explicit model selection (after D1)

**Files (for the recommended D1 option):**
- Modify: `src/main.cpp:76-99` and `src/service/main.cpp:126` — additionally load `profiles/experimental/ajazz-ak35i-v4.json` only when the setting is on.
- Modify: the Settings dialog added in `c16cb36` (*Edit > Settings*): checkbox "AJAZZ AK35I V4 (experimental, I own this exact model)", default off, stored with the existing settings mechanism the service already reads (check how the service gets "Hide/Dim disconnected"; if it does not read editor settings, stop and report before inventing a channel).
- Modify: `scripts/package_release.py` only if the `profiles` glob does not already include the subfolder (it uses `rglob`, so it does — verify the ZIP lists the file).
- Test: `tests/output/test_peripherals.cpp` or the startup/service test that covers profile loading.

- [ ] **Step 1:** Tests `experimentalProfileInactiveByDefault` (setting off → loaded profile IDs do not contain `ajazz-ak35i-v4`; a fake 0C45:8009 FF13 descriptor yields no candidate) and `experimentalProfileLoadedWhenEnabled`.
- [ ] **Step 2:** Run → FAIL. **Step 3:** Implement. **Step 4:** Full CTest → PASS.
- [ ] **Step 5:** Commit `feat(editor): opt-in experimental AJAZZ AK35I V4 profile`.

### Task 6: Documentation and handoff

**Files:** `docs/AJAZZ_AK35I_V4_PROTOCOL.md` (status line: Phase A implemented offline, list of tests), `docs/HARDWARE_MODULES.md` (peripherals module: new protocol, API 3), `CONTINUE.md` (state + Phase B checklist from spec §9), `THIRD_PARTY_NOTICES.md` only if anything third-party was used (expected: nothing).

- [ ] **Step 1:** Update the docs; state explicitly "no hardware test yet".
- [ ] **Step 2:** `graphify update .`; full CTest → PASS.
- [ ] **Step 3:** Commit `docs: AJAZZ AK35I V4 Phase A status`.

## Not in Phase A (spec §5, §6, §9 Phase B/C, §10)

Hardware test (START, test pattern Esc red / F1 green / F2 blue), timing measurement and fps tuning, unplug/power-cycle behavior, persistent effects, F5 readback, TFT, clock, keymap, 2.4 GHz/Bluetooth.
