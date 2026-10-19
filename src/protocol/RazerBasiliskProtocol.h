// Razer Basilisk V3 wire-protocol encoders. Ported from OpenRGB's
// RazerController
// (Controllers/RazerController/RazerController/RazerController.cpp/.h).
// Pure data encoding: no HID/device I/O, no Qt dependency.
//
// IMPORTANT CORRECTION vs. the task brief: the brief pointed at
// razer_argb_report / razer_usb_send_argb (RazerController.h:254-263,
// RazerController.cpp:2224-2228) as the Basilisk V3's ARGB path. That
// struct/function exist, but every Basilisk V3 device-table entry
// (RazerDevices.cpp, e.g. basilisk_v3_device at lines 4830-4848) sets
// matrix_type = RAZER_MATRIX_TYPE_EXTENDED, never
// RAZER_MATRIX_TYPE_EXTENDED_ARGB. razer_set_custom_frame's switch
// (RazerController.cpp:1567-1592) only reaches razer_create_custom_frame_
// argb_report/razer_usb_send_argb for RAZER_MATRIX_TYPE_EXTENDED_ARGB, so
// that path never runs for this device. The Basilisk V3 actually uses the
// plain razer_report struct (RazerController.h:236-249) sent via
// razer_usb_send -> hid_send_feature_report (RazerController.cpp:2184-2190),
// same transport style (feature report) the brief expected, just a
// different struct/builder. Function names below avoid "Argb" accordingly.
//
// Device geometry (RazerDevices.cpp:4830-4848, basilisk_v3_device):
// rows=1, cols=11 -> 11 LEDs total, zones in order Logo(1) [index 0],
// Scroll Wheel(1) [index 1], LED Strip(9, linear) [indices 2-10].
// RazerController::SetLEDs (.cpp:303-354) builds one row (row=0, since
// rows=1) covering the full width (start_col=0, stop_col=cols-1=10), then
// calls razer_set_mode_custom() once *after* the frame to put the device
// into custom-frame display mode (.cpp:343-348). For RAZER_MATRIX_TYPE_
// EXTENDED that resolves to razer_create_mode_custom_extended_matrix_report
// (.cpp:1024-1033) via razer_usb_send (.cpp:1840-1842).
//
// Both reports are the full 91-byte razer_report struct
// (sizeof: 1+1+1+2+1+1+1+1+80+1+1 = 91; PACK'd, no padding):
//   [0]      report_id        = report_index, which is 0 for the Basilisk
//                                V3 (default branch of the constructor's
//                                switch, .cpp:50-61 - only the Leviathan
//                                V2/V2X use 0x07).
//   [1]      status           = 0x00 (razer_create_report, .cpp:722-744)
//   [2]      transaction_id   = dev_transaction_id = device_list[...]
//                                ->transaction_id = 0x1F for
//                                basilisk_v3_device (RazerDevices.cpp:4836,
//                                read at RazerController.cpp:66; the union
//                                is a single byte, so .id is the whole byte)
//   [3-4]    remaining_packets = 0x0000 (razer_create_report)
//   [5]      protocol_type    = 0x00 (razer_create_report)
//   [6]      data_size        = varies per report, see below
//   [7]      command_class    = varies per report, see below
//   [8]      command_id       = varies per report, see below
//   [9-88]   arguments[80]    = varies per report, see below
//   [89]     crc              = XOR of bytes[3..88] inclusive
//                                (razer_calculate_crc, .cpp:695-716; the
//                                loop bound is `i < 89`, i.e. up to and
//                                including index 88)
//   [90]     reserved         = 0x00
//
// Colors report (razer_create_custom_frame_extended_matrix_report,
// .cpp:886-903, called from SetLEDs with row_index=0, start_col=0,
// stop_col=10):
//   data_size = row_length + 5 = (10+1-0)*3 + 5 = 38 = 0x26
//   command_class = 0x0F, command_id = 0x03
//   arguments[0..1] = 0 (never written by the OpenRGB builder)
//   arguments[2] = row_index = 0x00
//   arguments[3] = start_col = 0x00
//   arguments[4] = stop_col = 0x0A
//   arguments[5..37] = 33 bytes of RGB data, 3 bytes/LED x 11 LEDs, in
//                       zone order (Logo, Scroll Wheel, LED Strip x9)
//   arguments[38..79] = 0 (unused tail of the fixed 80-byte argument array)
//
// Mode-custom activation report (razer_create_mode_custom_extended_matrix_
// report, .cpp:1024-1033), constant, no per-call inputs:
//   data_size = 0x0C, command_class = 0x0F, command_id = 0x02
//   arguments[0] = 0x00, arguments[1] = 0x00, arguments[2] = 0x08
//   arguments[3..79] = 0
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "protocol/MsiProtocol.h" // MsiColor

constexpr size_t RAZER_BASILISK_REPORT_SIZE = 91; // sizeof(razer_report), RazerController.h:236-249
constexpr int RAZER_BASILISK_MAX_LEDS = 11;        // cols, RazerDevices.cpp:4837 (basilisk_v3_device)

// Throws std::invalid_argument if colors.size() > RAZER_BASILISK_MAX_LEDS.
// Short input is zero-padded (unwritten LED slots stay black), matching
// BuildKoneColorsReport's precedent.
std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> BuildBasiliskColorsReport(const std::vector<MsiColor>& colors);

// Fixed report sent once after every colors report to put the device into
// custom-frame display mode (razer_set_mode_custom ->
// razer_create_mode_custom_extended_matrix_report). No inputs: every byte
// is a constant.
std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> BuildBasiliskModeCustomReport();
