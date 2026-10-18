// MSI wire-protocol encoders for the MB_800 HID family (reports 0x50/0x51).
// Pure data encoding: no HID/device I/O, no Qt dependency. Byte layout and
// bit-packing are pinned exactly to docs/MB_800_HID_PROTOCOL.md; see that
// doc before changing any formula here.
#pragma once

#include <array>
#include <cstdint>
#include <vector>

struct MsiColor { uint8_t r = 0, g = 0, b = 0; };

enum class MsiLightingMode : uint8_t {
    Off = 0, Wave = 1, Steady = 2, Flame = 3, Breathing = 4, ColorRing = 5,
    Lightning = 6, Recreation = 7, Meteor = 8, Advanced = 9, GodLike = 10
};

// One 16-byte record of report 0x50. Default-constructed = Off, all-black, matches an unused zone.
struct MsiZoneSettings {
    MsiLightingMode lightingMode = MsiLightingMode::Off;
    MsiColor color1{}, color2{}, color3{}, color4{};
    uint8_t colorCount = 1;          // 1-4 active colors (encoded as (colorCount-1) in Option1 bits 0-1)
    bool selectAll = false;          // Option2 bit 7
    bool directionInward = false;    // Option2 bit 6 (false=Out, true=In)
    bool colorSelectionUser = false; // Option2 bit 5 (false=Rainbow, true=User)
    uint8_t brightness = 5;          // Option2 bits 4-2, 0-5 -> 0/20/40/60/80/100%
    uint8_t speed = 0;               // Option2 bits 1-0, 0-2
    uint8_t cycleNum = 30;           // last byte of the record
};

// Builds the full 290-byte report 0x50 payload. zones[i] is the record for
// MsiZoneIndex i (JARGB1=0 ... SelectAll=17), written contiguously starting
// at byte 1 (NO separate header — see docs/MB_800_HID_PROTOCOL.md). Byte 0
// is the report ID (0x50). Byte 289 is the Store flag: `store` true persists
// to board NVRAM, false (the default, and v1's only use) is session-only.
std::array<uint8_t, 290> BuildReport0x50(const std::array<MsiZoneSettings, 18>& zones, bool store = false);

// Area addresses for report 0x51 (big-endian uint16 at bytes 2-3).
enum class MsiGen1AdvArea : uint16_t {
    JRGB1 = 0x0200, JRGB2 = 0x0201,
    JPIPE1 = 0x0300, JPIPE2 = 0x0301, JPIPE3 = 0x0302, JPIPE4 = 0x0303, JPIPE5 = 0x0304,
    JARGB1 = 0x0400, JARGB2 = 0x0401, JARGB3 = 0x0402,
    SelectAll = 0x0600,
    Onboard1 = 0x0700, Onboard2 = 0x0701, Onboard3 = 0x0702, Onboard4 = 0x0703, Onboard5 = 0x0704, Onboard6 = 0x0705,
    JAF = 0x0800
};

// Builds the 727-byte report 0x51 (Gen1_Adv) payload: byte0=0x51, byte1=0x09,
// bytes2-3=area (big-endian), bytes4-5=0x00, byte6=colors.size(), bytes7+ =
// RGB triplets, remainder zero-padded to 727. Throws std::invalid_argument
// if colors.size() > 240.
std::array<uint8_t, 727> BuildReport0x51(MsiGen1AdvArea area, const std::vector<MsiColor>& colors);
