// SPDX-License-Identifier: GPL-2.0-or-later
//
// Derived from OpenRGB (https://openrgb.org), GPL-2.0-or-later: upstream
// source LenovoMotherboardController::SetMode and
// RGBController_LenovoMotherboard's mode/zone tables
// (Controllers/LenovoMotherboardController/LenovoMotherboardController.cpp:51-89,
// RGBController_LenovoMotherboard.cpp:38-166). This is a from-scratch
// reimplementation of that protocol, written by reading OpenRGB's source
// (not copied from it), modified to this codebase's own style and
// IHidDevice abstraction; kept under GPL-2.0-or-later as a derivative work
// of GPL-licensed protocol logic, per GPLv2 Sections 1/2(a).
//
// Lenovo motherboard RGB (VID 0x17EF, PID 0xC955, usage page 0xFF89, usage
// 0xCC) feature-report encoder. Two 64-byte feature reports
// (hid_send_feature_report) per update, both carrying the same
// zone/mode/speed/brightness/color fields at different offsets: upstream
// sends both unconditionally on every call and this reimplementation does
// the same, since the second report's purpose (a secondary/global register
// vs. a duplicate write) is not documented upstream and was not verified
// against real hardware. Upstream's own metadata marks this device
// `@save :x:` (no persisted state), so there is no flash-wear concern like
// ASRock's Polychrome controller.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "protocol/MsiProtocol.h" // MsiColor

constexpr std::size_t LENOVO_MOTHERBOARD_PACKET_SIZE = 64;

enum class LenovoMotherboardZone : std::uint8_t {
    RearFan = 0x11,
    CpuFan = 0x12,
};

enum class LenovoMotherboardMode : std::uint8_t {
    Static = 0x01, Sparkle = 0x02, Breathing = 0x03, Wave = 0x04,
    Specter = 0x06, RainbowWave = 0x09, Random = 0x0A
};

// speed and brightness are raw 1..4 values (upstream's documented range);
// callers map their own 0..2 / 0..5 scales down before calling this.
std::array<std::array<std::uint8_t, LENOVO_MOTHERBOARD_PACKET_SIZE>, 2> BuildLenovoMotherboardReports(
    LenovoMotherboardZone zone, LenovoMotherboardMode mode, std::uint8_t speed,
    std::uint8_t brightness, MsiColor color);
