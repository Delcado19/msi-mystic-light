// SteelSeries Apex 3 (T-Zone protocol) wire-protocol encoders. Ported from
// OpenRGB's SteelSeriesApexTZoneController::SetColor
// (Controllers/SteelSeriesController/SteelSeriesApexTZoneController/SteelSeriesApexTZoneController.cpp:56-80).
// Pure data encoding: no HID/device I/O, no Qt dependency. No report ID byte
// (buf[0] stays 0x00, unlike MsiProtocol's report[0]); two 33-byte output
// reports (hid_write, not a feature report) per update: a brightness packet
// (cmd 0x0A) then a colors packet (cmd 0x0B, up to
// STEELSERIES_APEX3_MAX_ZONES zones' RGB starting at offset 3).
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "protocol/MsiProtocol.h" // MsiColor

constexpr size_t STEELSERIES_APEX3_PACKET_SIZE = 33;
constexpr int STEELSERIES_APEX3_MAX_ZONES = 10; // (33-3)/3

std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> BuildApex3BrightnessPacket(uint8_t brightness);

// Throws std::invalid_argument if zoneColors.size() > STEELSERIES_APEX3_MAX_ZONES.
std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> BuildApex3ColorsPacket(const std::vector<MsiColor>& zoneColors);
