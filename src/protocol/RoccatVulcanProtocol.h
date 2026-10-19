// Roccat Vulcan 100/120 Aimo wire-protocol encoder (per-key LED output
// reports). Ported from OpenRGB's RoccatVulcanKeyboardController
// (Controllers/RoccatController/RoccatVulcanKeyboardController/
// RoccatVulcanKeyboardController.cpp and .h). Pure data encoding: no
// HID/device I/O, no Qt dependency.
//
// Scope: this only covers RoccatVulcanKeyboardController::SendColors
// (.cpp:197-398), the hid_write(dev_led, ...) direct-mode color push. It
// does NOT cover SendMode/EnableDirect (those write to the *control*
// interface via feature reports to activate/configure a mode - out of
// scope per the task brief, which only asks for the per-key color push).
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
//             for the default case (which both target PIDs hit). This is
//             the keyboard's *firmware version*, read at runtime via
//             InitDeviceInfo()'s feature-report round-trip
//             (RoccatVulcanKeyboardController.cpp:72-ff) - a stateful
//             handshake this task does not implement (out of scope; see
//             the task brief, which only asks for the SendColors byte
//             packing). Since dev_info is a freshly-constructed
//             device_info with no explicit zero-initializer in this
//             class, and OpenRGB's RGBController_RoccatVulcanKeyboard
//             constructor (.cpp:37) calls InitDeviceInfo() before any
//             color push can happen, version_major/minor are *always*
//             populated by the time SendColors runs on a real device -
//             this is not actually a "sometimes true" runtime fork on
//               hardware, it is a fixed property of each physical unit's
//             flashed firmware, just one this driver has no way to query
//             without implementing the same handshake. We default to the
//             little-endian branch (`bufs[0][3]=length&0xFF,
//             bufs[0][4]=(length>>8)&0xFF`) because it is what
//             IsBigEndianDirectMode() returns for an unqueried/zero
//             dev_info, and because it is the only one of the two we can
//             pin down without that handshake.
//             ** CONCERN carried into the task report: this is the one
//             byte pair in this encoder that may be wrong for a specific
//             physical unit depending on its firmware version. It affects
//             only packet 0, bytes 3-4 (a constant length field, not any
//             per-key color byte) - confirm against real hardware in
//             Task 18, and if it turns out wrong, the fix is a single
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
