// SPDX-License-Identifier: GPL-2.0-or-later
//
// Derived from OpenRGB (https://openrgb.org), GPL-2.0-or-later: upstream
// source SteelSeriesApexTZoneController::SetColor (Controllers/
// SteelSeriesController/SteelSeriesApexTZoneController/
// SteelSeriesApexTZoneController.cpp:56-80). This is a from-scratch
// reimplementation of that protocol, written by reading OpenRGB's source
// (not copied from it), and modified to this codebase's own style and
// IHidDevice abstraction; kept under GPL-2.0-or-later as a derivative
// work of GPL-licensed protocol logic, per GPLv2 Sections 1/2(a).

#include "protocol/SteelSeriesApex3Protocol.h"

#include <stdexcept>

// Byte layout pinned exactly to OpenRGB's SteelSeriesApexTZoneController::
// SetColor. No report ID byte (byte 0 stays 0x00, unlike MsiProtocol's
// reports); byte 1 is the command (0x0A = set brightness); byte 3 is the
// brightness value. Do not approximate.
std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> BuildApex3BrightnessPacket(uint8_t brightness)
{
    std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> packet{};
    packet[1] = 0x0A;
    packet[3] = brightness;
    return packet;
}

// Byte layout pinned exactly to OpenRGB's SteelSeriesApexTZoneController::
// SetColor: byte 1 = 0x0B (set colors), RGB triplets starting at byte 3, one
// per zone, up to STEELSERIES_APEX3_MAX_ZONES ((33-3)/3 = 10).
std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> BuildApex3ColorsPacket(const std::vector<MsiColor>& zoneColors)
{
    if (zoneColors.size() > static_cast<size_t>(STEELSERIES_APEX3_MAX_ZONES)) {
        throw std::invalid_argument("BuildApex3ColorsPacket: zoneColors.size() exceeds STEELSERIES_APEX3_MAX_ZONES");
    }

    std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> packet{};
    packet[1] = 0x0B;
    for (size_t i = 0; i < zoneColors.size(); ++i) {
        packet[3 + i * 3] = zoneColors[i].r;
        packet[4 + i * 3] = zoneColors[i].g;
        packet[5 + i * 3] = zoneColors[i].b;
    }
    return packet;
}
