// AJAZZ AK35I V4 (ISO-DE, wired 0C45:8009, control collection FF13:0001) Direct RGB encoder.
// Independent implementation from the owner's vendor-driver captures; no third-party code. Wire
// format and evidence: docs/AJAZZ_AK35I_V4_PROTOCOL.md sections 3-4, fixture
// docs/hardware-inventory/ajazz-ak35i-v4-rgb-fixtures.json. Pure data encoding, no device I/O.
//
// Native hidapi feature buffers are 65 bytes: byte 0 = report ID 0, bytes 1..64 = payload.
// Payload offsets in the spec are therefore native offset - 1.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "protocol/MsiProtocol.h" // MsiColor

constexpr std::size_t AJAZZ_V4_PAYLOAD = 64;
constexpr std::size_t AJAZZ_V4_REPORT = 65;
constexpr int AJAZZ_V4_LEDS = 105;

// light_index of each physical key in manufacturer XML order (fixture led_map.entries[].order 0..104).
// The Direct RGB table must use this order, not numeric ID order.
extern const std::array<std::uint8_t, AJAZZ_V4_LEDS> AJAZZ_V4_LIGHT_IDS;

using AjazzReport = std::array<std::uint8_t, AJAZZ_V4_REPORT>;

// Command report: payload 04 <opcode> 00 00 00 00 00 00 <arg8>, zero padded.
AjazzReport BuildAjazzCommand(std::uint8_t opcode, std::uint8_t arg8 = 0);

// Eight data reports of 16 [light ID, R, G, B] records: 105 LEDs in XML order, then 23 zero records.
// The eighth report is all zero and is still sent (captured behavior). Throws std::invalid_argument
// unless colors.size() == AJAZZ_V4_LEDS.
std::array<AjazzReport, 8> BuildAjazzDirectChunks(const std::vector<MsiColor>& colors);

// Positive acknowledgement: full 65-byte read, report ID 0, payload 04 <opcode> xx 01. Other bytes
// are ignored (04 02 replies carry extra non-zero bytes).
bool AjazzAckOk(const std::uint8_t* report, std::size_t length, std::uint8_t opcode);
