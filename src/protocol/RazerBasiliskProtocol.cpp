// SPDX-License-Identifier: GPL-2.0-or-later
//
// Derived from OpenRGB (https://openrgb.org), GPL-2.0-or-later: upstream
// source RazerController (Controllers/RazerController/RazerController/
// RazerController.cpp/.h), specifically SetLEDs, razer_create_custom_
// frame_extended_matrix_report, razer_create_mode_custom_extended_matrix_
// report, and razer_calculate_crc. This is a from-scratch reimplementation
// of that protocol, written by reading OpenRGB's source (not copied from
// it), and modified to this codebase's own style and IHidDevice
// abstraction; kept under GPL-2.0-or-later as a derivative work of
// GPL-licensed protocol logic, per GPLv2 Sections 1/2(a).

#include "protocol/RazerBasiliskProtocol.h"

#include <stdexcept>

namespace {

// Ported from RazerController::razer_calculate_crc (RazerController.cpp:
// 695-716): XOR of every byte from index 3 up to and including index 88
// (the loop is `for (i = 3; i < 89; i++)`), i.e. everything except
// report_id/status/transaction_id (0-2) and crc/reserved (89-90).
uint8_t CalculateCrc(const std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE>& report)
{
    uint8_t crc = 0;
    for (size_t i = 3; i < 89; ++i) {
        crc ^= report[i];
    }
    return crc;
}

} // namespace

// Byte layout pinned to razer_create_custom_frame_extended_matrix_report
// (RazerController.cpp:886-903) as called from SetLEDs (.cpp:303-341) with
// row_index=0, start_col=0, stop_col=RAZER_BASILISK_MAX_LEDS-1=10. See the
// header's top-of-file comment for the full byte-by-byte derivation.
std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> BuildBasiliskColorsReport(const std::vector<MsiColor>& colors)
{
    if (colors.size() > static_cast<size_t>(RAZER_BASILISK_MAX_LEDS)) {
        throw std::invalid_argument("BuildBasiliskColorsReport: colors.size() exceeds RAZER_BASILISK_MAX_LEDS");
    }

    std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> report{};
    report[2] = 0x1F;  // transaction_id, RazerDevices.cpp:4836
    report[6] = 0x26;  // data_size = row_length(33) + 5
    report[7] = 0x0F;  // command_class
    report[8] = 0x03;  // command_id
    // arguments[2] = row_index = 0x00 (report[11]) - stays 0, left implicit
    // arguments[3] = start_col = 0x00 (report[12]) - stays 0, left implicit
    report[13] = 0x0A; // arguments[4] = stop_col = cols-1 = 10

    for (size_t i = 0; i < colors.size(); ++i) {
        const size_t argIdx = 14 + (i * 3); // arguments[5 + i*3], report offset = 9 + 5 + i*3
        report[argIdx + 0] = colors[i].r;
        report[argIdx + 1] = colors[i].g;
        report[argIdx + 2] = colors[i].b;
    }

    report[89] = CalculateCrc(report);
    return report;
}

// Byte layout pinned to razer_create_mode_custom_extended_matrix_report
// (RazerController.cpp:1024-1033). Constant: every byte is fixed, no
// per-call inputs.
std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> BuildBasiliskModeCustomReport()
{
    std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> report{};
    report[2] = 0x1F;  // transaction_id
    report[6] = 0x0C;  // data_size
    report[7] = 0x0F;  // command_class
    report[8] = 0x02;  // command_id
    // arguments[0] = 0x00 (report[9]) - stays 0, left implicit
    // arguments[1] = 0x00 (report[10]) - stays 0, left implicit
    report[11] = 0x08; // arguments[2]

    report[89] = CalculateCrc(report);
    return report;
}
