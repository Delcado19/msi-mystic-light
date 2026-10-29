// SPDX-License-Identifier: GPL-2.0-or-later
//
// Derived from OpenRGB (https://openrgb.org), GPL-2.0-or-later: upstream
// source PolychromeUSBController::WriteZone and RGBController_PolychromeUSB's
// mode table (Controllers/ASRockPolychromeUSBController/
// ASRockPolychromeUSBController.cpp:211-261,
// RGBController_ASRockPolychromeUSB.cpp:40-183). This is a from-scratch
// reimplementation of that protocol, written by reading OpenRGB's source
// (not copied from it), cross-checked against SignalRGB's
// Asrock_Motherboard_Controller.js plugin (same byte layout at the per-LED
// streaming level; see docs/OPENRGB_HARDWARE_PORT_PLAN.md), and modified to
// this codebase's own style and IHidDevice abstraction; kept under
// GPL-2.0-or-later as a derivative work of GPL-licensed protocol logic, per
// GPLv2 Sections 1/2(a).
//
// ASRock Polychrome USB (VID 0x26CE, PID 0x01A2) whole-zone wire-protocol
// encoder. No report ID byte (buf[0] stays 0x00). One 65-byte output report
// (hid_write) per zone update, byte 1 = command 0x10; the device answers
// with a 64-byte input report that upstream never inspects (the caller
// drains it, best-effort, and does not fail the write on a drain failure).
//
// Scope deliberately narrower than upstream: only the single-color
// whole-zone "native mode" path is implemented. RGSwap color-order config
// and the combined multi-zone per-LED "Direct"/Chroma streaming mode are
// not implemented (see docs/OPENRGB_HARDWARE_PORT_PLAN.md: Direct needs a
// cross-channel combined-buffer design this driver does not have, and its
// flash-persistence behavior under continuous real-time updates is
// unverified). Color-channel order is fixed non-swapped (G, R, B at offsets
// 5-7), matching upstream's own safe default
// (WriteRGSwap(0,0,0,0,0,0,0,0) — "Disable RGSwap as it causes flashing on
// each update in direct mode").
//
// Upstream's RGBController_ASRockPolychromeUSB.cpp has a copy-paste bug: its
// "Music" mode block reuses POLYCHROME_USB_MODE_RANDOM's value (0x05)
// instead of the header's own POLYCHROME_USB_MODE_MUSIC (0x06). This
// reimplementation uses the correct 0x06.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "protocol/MsiProtocol.h" // MsiColor

constexpr std::size_t ASROCK_POLYCHROME_PACKET_SIZE = 65;
constexpr int ASROCK_POLYCHROME_ZONE_COUNT = 8;

enum class AsrockPolychromeZone : std::uint8_t {
    Rgb1 = 0, Rgb2 = 1, Argb1 = 2, Argb2 = 3, Pch = 4, IoCover = 5, Pcb = 6, Audio = 7
};

enum class AsrockPolychromeMode : std::uint8_t {
    Off = 0x00, Static = 0x01, Breathing = 0x02, Strobe = 0x03, SpectrumCycle = 0x04,
    Random = 0x05, Music = 0x06, Wave = 0x07, Spring = 0x08, Stack = 0x09, Cram = 0x0A,
    Scan = 0x0B, Neon = 0x0C, Water = 0x0D, Rainbow = 0x0E
};

// Throws std::invalid_argument if zone is not one of the eight named zones.
std::array<std::uint8_t, ASROCK_POLYCHROME_PACKET_SIZE> BuildAsrockPolychromeZoneReport(
    AsrockPolychromeZone zone, AsrockPolychromeMode mode, std::uint8_t speed, MsiColor color);
