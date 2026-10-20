// Roccat Vulcan 100/120 Aimo wire-protocol encoder (per-key LED output
// reports). Ported from OpenRGB's RoccatVulcanKeyboardController
// (Controllers/RoccatController/RoccatVulcanKeyboardController/
// RoccatVulcanKeyboardController.cpp and .h). Pure data encoding: no
// HID/device I/O, no Qt dependency.
//
// Scope: covers RoccatVulcanKeyboardController::SendColors (.cpp:197-398,
// the hid_write(dev_led, ...) direct-mode color push - the task brief's
// primary target) AND EnableDirect (.cpp:166-195, the one-time "enter
// direct mode" control-report, needed so a real device actually honors
// the color packets - RGBController_RoccatVulcanKeyboard.cpp:314 only
// calls EnableDirect(true) once, from DeviceUpdateMode(), not on every
// color push). It does NOT cover SendMode (.cpp:400-ff, the much larger
// mode/speed/brightness/static-color control report) - that configures
// non-Direct modes (Wave, Static, Default) this app's Canvas-only driver
// never uses, so porting it is out of scope here.
//
// PID identity (RoccatVulcanKeyboardController.h:23-24):
//   ROCCAT_VULCAN_100_AIMO_PID = 0x307A, ROCCAT_VULCAN_120_AIMO_PID = 0x3098
// Neither PID appears anywhere in RoccatVulcanKeyboardController.cpp
// (verified by search) - both fall through every switch in that file
// (SendColors, SendMode, InitDeviceInfo, IsBigEndianDirectMode) to the
// `default` case. So, unlike most of this file's other model families,
// the two target PIDs are NOT just "similar" - they are byte-for-byte
// identical in every packing constant below. This is independently
// confirmed by RGBController_RoccatVulcanKeyboard.cpp:112-115, which maps
// both PIDs to the same `RoccatVulcan120AimoLayouts` key-matrix table, and
// by lines 53-59 of that file, which group both PIDs together as the one
// exception that does NOT get MODE_FLAG_HAS_BRIGHTNESS on Direct mode.
//
// SendColors default-case constants (RoccatVulcanKeyboardController.cpp:
// 243-246): packet_length=436, column_length=12, protocol_version=1.
// Derived from those:
//   packet_num = ceil(436/64) = 7                              (.cpp:249)
//   header_length_first = (436>255) ? 4 : 3 = 4                (.cpp:272)
//   data_length_packet (version>1 only, unused here) = 60      (.cpp:292)
//   max LED slots = (packet_length - header_length_first)/3 = 144
//
// Packet shape: packet_num=7 buffers of 65 bytes each
// (bufs[p].resize(65), .cpp:254), sent individually via
// hid_write(dev_led, &bufs[p][0], 65) (.cpp:388-394). Byte 0 of every
// packet is never written (always 0x00) - it is the output-report's
// report-ID byte, consistent with IHidDevice::write()'s "report must
// already include the report ID as report[0]" contract.
//
// Packet 0 header, protocol_version==1 branch (.cpp:266-269, 274-289):
//   [1] = 0xA1
//   [2] = 0x01
//   [3],[4] = packet_length (436 = 0x01B4) as a 16-bit value, byte order
//             picked by IsBigEndianDirectMode() (.cpp:155-164):
//             `dev_info.version_major >= 1 && dev_info.version_minor >= 16`
//             for the default case (which both target PIDs hit), i.e.
//             big-endian only for firmware >= v1.16 (version parsed from
//             a single byte as major=buf[2]/100, minor=buf[2]%100 -
//             InitDeviceInfo, .cpp:110-111). This is a genuine per-unit
//             fact (each physical keyboard's flashed firmware either is
//             or isn't >= v1.16), queried at runtime via a feature-report
//             round-trip (InitDeviceInfo, .cpp:72-147) that this task does
//             not implement (out of scope; the brief asks only for the
//             SendColors byte packing). Without that handshake this
//             encoder cannot know a specific unit's firmware version, so
//             it hardcodes one branch - an arbitrary but necessary choice
//             per unit, not a derived default.
//             ** Live-hardware-tested 2026-10-20 against a real Roccat
//             Vulcan 100 Aimo (full-size, numpad, 144 individually
//             addressable keys): the little-endian branch (`bufs[0][3]=
//             length&0xFF, bufs[0][4]=(length>>8)&0xFF`) produced
//             corrupted/shifted colors - all 144 keys lit in scattered
//             wrong hues (yellow/green/cyan, no red), the classic symptom
//             of the firmware misparsing the declared length and reading
//             color bytes from a shifted offset. The big-endian branch
//             (`bufs[0][3]=(length>>8)&0xFF, bufs[0][4]=length&0xFF`,
//             now hardcoded in BuildVulcanColorPackets) produced solid,
//             correct red across all keys, confirming this tested unit's
//             firmware is >= v1.16 per IsBigEndianDirectMode() above. So
//             big-endian is now the confirmed default for at least this
//             one real unit - but the underlying caveat still applies: a
//             *different* physical unit on older firmware (pre-v1.16)
//             could still need little-endian, and this encoder still has
//             no way to tell without performing the real
//             InitDeviceInfo/IsBigEndianDirectMode handshake. If a future
//             unit reports wrong colors again, the fix is the same single
//             two-byte swap, not a repacking. **
//   [5..64] = color data starts here (byte index 5 = header_length_first+1)
//
// Packets 1..6: no header at all - bytes [1..64] are color data (.cpp:
// 364-371 computes a *global* linear offset starting at
// `column*3*column_length + row + header_length_first`, so the +4 shift
// only matters for data landing in packet 0; packets 1+ get the full 64
// usable bytes since `offset/64`/`offset%64` naturally rolls over).
//
// Per-key offset (protocol_version==1 branch, .cpp:361-371), re-derived
// as a closed form for key index `idx` in [0, VULCAN_MAX_LEDS):
//   column = idx / VULCAN_COLUMN_LENGTH
//   row    = idx % VULCAN_COLUMN_LENGTH
//   base   = column * 3 * VULCAN_COLUMN_LENGTH + row + header_length_first
//   R at global offset base,    G at base+VULCAN_COLUMN_LENGTH,
//   B at base+2*VULCAN_COLUMN_LENGTH
//   packet index  = globalOffset / 64
//   byte in array = globalOffset % 64 + 1   (the "+1" skips the unwritten
//                                             report-ID byte 0)
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "protocol/MsiProtocol.h" // MsiColor

constexpr size_t VULCAN_PACKET_SIZE = 65;             // bufs[p].resize(65), .cpp:254
constexpr int VULCAN_PACKET_COUNT = 7;                // ceil(436/64), .cpp:249
constexpr int VULCAN_COLUMN_LENGTH = 12;              // column_length, default case, .cpp:245
constexpr int VULCAN_HEADER_LENGTH_FIRST = 4;          // header_length_first, .cpp:272 (436>255)
constexpr uint16_t VULCAN_PACKET_LENGTH = 436;         // packet_length, default case, .cpp:244
// Max addressable LED slots: (packet_length - header_length_first) / 3 RGB
// bytes = (436-4)/3 = 144 = VULCAN_COLUMN_LENGTH * VULCAN_COLUMN_LENGTH
// (12 columns x 12 rows in the column-major packing above).
constexpr int VULCAN_MAX_LEDS = 144;

// Builds the VULCAN_PACKET_COUNT output-report packets for a direct-mode
// per-key color push (RoccatVulcanKeyboardController::SendColors,
// .cpp:197-398, default-case constants - see header comment above).
// keyColors[i] is key index i's color (same convention as
// BuildBasiliskColorsReport/BuildKoneColorsReport: vector position IS the
// LED/key index, there is no separate index field). Keys beyond
// keyColors.size() stay black (zero-initialized), matching the real
// device always receiving all VULCAN_PACKET_COUNT packets regardless of
// how many keys the caller actually painted.
// Throws std::invalid_argument if keyColors.size() > VULCAN_MAX_LEDS.
std::vector<std::array<uint8_t, VULCAN_PACKET_SIZE>> BuildVulcanColorPackets(
    const std::vector<MsiColor>& keyColors);

constexpr size_t VULCAN_ENABLE_DIRECT_REPORT_SIZE = 3;

// RoccatVulcanKeyboardController::EnableDirect's default-case report
// (.cpp:166-195: `buf = {0x15, 0x00, on_off_switch}`, sent via
// hid_send_feature_report(dev_ctrl, buf, 3) to the CONTROL interface, NOT
// the LED interface SendColors/BuildVulcanColorPackets uses). Both target
// PIDs hit this default case (neither is named in EnableDirect's switch
// either). RGBController_RoccatVulcanKeyboard::DeviceUpdateMode (.cpp:
// 314) calls `EnableDirect(modes[active_mode].value ==
// ROCCAT_VULCAN_MODE_DIRECT)` - i.e. enable=true - once when entering
// Direct mode, not on every color push; RoccatVulcanDriver mirrors that
// by sending this from setChannelMode(Canvas), not from
// pushCanvasColors().
std::array<uint8_t, VULCAN_ENABLE_DIRECT_REPORT_SIZE> BuildVulcanEnableDirectReport(bool enable);
