// SPDX-License-Identifier: GPL-2.0-or-later
//
// Derived from OpenRGB (https://openrgb.org), GPL-2.0-or-later: see
// LenovoMotherboardProtocol.h for the exact upstream source.

#include "protocol/LenovoMotherboardProtocol.h"

std::array<std::array<std::uint8_t, LENOVO_MOTHERBOARD_PACKET_SIZE>, 2> BuildLenovoMotherboardReports(
    LenovoMotherboardZone zone, LenovoMotherboardMode mode, std::uint8_t speed,
    std::uint8_t brightness, MsiColor color)
{
    const auto zoneValue = static_cast<std::uint8_t>(zone);
    const auto modeValue = static_cast<std::uint8_t>(mode);

    std::array<std::uint8_t, LENOVO_MOTHERBOARD_PACKET_SIZE> first{};
    first[0] = 0xCC; // LENOVO_MB_REPORT_ID
    first[1] = zoneValue;
    first[2] = modeValue;
    first[3] = speed;
    first[4] = brightness;
    first[5] = color.r;
    first[6] = color.g;
    first[7] = color.b;

    std::array<std::uint8_t, LENOVO_MOTHERBOARD_PACKET_SIZE> second{};
    second[0] = 0xCC;
    second[1] = 0x28;
    second[2] = 0x06;
    second[33] = zoneValue;
    second[34] = modeValue;
    second[35] = speed;
    second[36] = brightness;
    second[37] = color.r;
    second[38] = color.g;
    second[39] = color.b;

    return {first, second};
}
