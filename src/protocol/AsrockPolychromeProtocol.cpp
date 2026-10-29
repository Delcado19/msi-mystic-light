// SPDX-License-Identifier: GPL-2.0-or-later
//
// Derived from OpenRGB (https://openrgb.org), GPL-2.0-or-later: see
// AsrockPolychromeProtocol.h for the exact upstream source and the scope
// this reimplementation deliberately narrows.

#include "protocol/AsrockPolychromeProtocol.h"

#include <stdexcept>

std::array<std::uint8_t, ASROCK_POLYCHROME_PACKET_SIZE> BuildAsrockPolychromeZoneReport(
    AsrockPolychromeZone zone, AsrockPolychromeMode mode, std::uint8_t speed, MsiColor color)
{
    const auto zoneValue = static_cast<std::uint8_t>(zone);
    if (zoneValue >= ASROCK_POLYCHROME_ZONE_COUNT) {
        throw std::invalid_argument("BuildAsrockPolychromeZoneReport: zone out of range");
    }

    std::array<std::uint8_t, ASROCK_POLYCHROME_PACKET_SIZE> packet{};
    packet[1] = 0x10; // POLYCHROME_USB_SET_ZONE
    packet[3] = zoneValue;
    packet[4] = static_cast<std::uint8_t>(mode);
    packet[5] = color.g; // Non-swapped order: G, R, B (RGSwap always disabled).
    packet[6] = color.r;
    packet[7] = color.b;
    packet[8] = speed;
    packet[9] = 0xFF;
    return packet;
}
