#pragma once

#include <array>
#include <cstdint>
#include <vector>

struct MsiColor {
    std::uint8_t r = 0, g = 0, b = 0;
};

enum class MsiLightingMode : std::uint8_t {
    Off = 0, Wave = 1, Steady = 2, Flame = 3, Breathing = 4,
    ColorRing = 5, Lightning = 6, Recreation = 7, Meteor = 8,
    Advanced = 9, GodLike = 10
};

struct MsiZoneSettings {
    MsiLightingMode lightingMode = MsiLightingMode::Off;
    MsiColor color1{}, color2{}, color3{}, color4{};
    std::uint8_t colorCount = 1;
    bool selectAll = false;
    bool directionInward = false;
    bool colorSelectionUser = false;
    std::uint8_t brightness = 5;
    std::uint8_t speed = 0;
    std::uint8_t cycleNum = 30;
};

// docs/MB_800_HID_PROTOCOL.md: records start at byte 1, not after a header.
// Session-only by default: repeated Store writes would wear board NVRAM.
// Throws std::invalid_argument for unsupported modes or out-of-range settings.
std::array<std::uint8_t, 290> BuildReport0x50(
    const std::array<MsiZoneSettings, 18>& zones, bool store = false);

enum class MsiGen1AdvArea : std::uint16_t {
    JRGB1 = 0x0200, JRGB2 = 0x0201,
    JPIPE1 = 0x0300, JPIPE2 = 0x0301, JPIPE3 = 0x0302,
    JPIPE4 = 0x0303, JPIPE5 = 0x0304,
    JARGB1 = 0x0400, JARGB2 = 0x0401, JARGB3 = 0x0402,
    SelectAll = 0x0600,
    Onboard1 = 0x0700, Onboard2 = 0x0701, Onboard3 = 0x0702,
    Onboard4 = 0x0703, Onboard5 = 0x0704, Onboard6 = 0x0705,
    JAF = 0x0800
};

// Pure serialization, no HID I/O. Throws std::invalid_argument for unknown
// areas or more than 240 LEDs; board-specific limits belong to the driver.
std::array<std::uint8_t, 727> BuildReport0x51(
    MsiGen1AdvArea area, const std::vector<MsiColor>& colors);
