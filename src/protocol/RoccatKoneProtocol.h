// Roccat Kone Aimo wire-protocol encoders. Ported from OpenRGB's
// RoccatKoneAimoController
// (Controllers/RoccatController/RoccatKoneAimoController/RoccatKoneAimoController.cpp
// and .h). Pure data encoding: no HID/device I/O, no Qt dependency.
//
// Two distinct feature reports, confirmed by RoccatKoneAimoController.cpp:
//  - A 6-byte init report, built once in SendInit() (.cpp:60-83) from a
//    LOCAL buffer unrelated to the color buffer: usb_buf[0..5] =
//    {0x0E, 0x06, 0x01, 0x01, 0x00, 0xFF} (.cpp:72-77), sent via
//    hid_send_feature_report(dev, usb_buf, 6) (.cpp:82).
//  - A 46-byte color report (usb_colors_buf, USB_COLOR_BUFF_LEN = 46,
//    .h:28): byte 0 = 0x0D, byte 1 = 0x2E, set once in the constructor
//    (.cpp:26-27). SetChannelColors (.cpp:85-100) writes each LED's RGB at
//    usb_idx = 0x02 + (ledIndex * 4) (.cpp:94), with R/G/B sub-offsets 0/1/2
//    (R_OFFSET/G_OFFSET/B_OFFSET, .h:24-26); the 4th byte of each 4-byte
//    slot is never written, so it stays 0 padding. SendUpdate (.cpp:102-112)
//    sends the whole 46-byte buffer via hid_send_feature_report every time,
//    "whole buffer needs to be sent everytime" (.cpp:105).
//    NUM_LEDS (.h:22) = 11, matching the ROCCAT_KONE_AIMO_CHANNEL enum
//    (.h:30-37: SCROLL_WHEEL=0 .. LOWER_RIGHT=10) and the buffer's capacity
//    (last LED's B byte lands at 0x02 + 10*4 + 2 = 44, fits in 46 bytes).
//
// SendInit's hid_get_feature_report (.cpp:67) reads into a separate local
// buffer, not usb_colors_buf, so it has no bearing on the color report.
// SendUpdate's hid_get_feature_report(dev, usb_colors_buf, 3) (.cpp:111)
// DOES read the device's reply back into the persistent color buffer, so
// bytes 0-2 of usb_colors_buf are overwritten after every real send: byte 0
// (report ID echo, likely stays 0x0D), byte 1 (0x2E = 46 = USB_COLOR_BUFF_LEN
// on the first send - an inference that it's an echoed size field, not
// confirmed from source), byte 2 (LED0.R, clobbered until the next
// SetChannelColors call rewrites it). BuildKoneColorsReport below only
// models the buffer OpenRGB *sends*, i.e. its state before that read-back,
// which is correct for a single push; this app also has no
// hid_get_feature_report equivalent (IHidDevice exposes no feature-report
// getter), so a byte-0/1/2 round-trip effect cannot be reproduced here even
// if it turns out to matter. Flagged for confirmation against real hardware
// in Task 18.
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "protocol/MsiProtocol.h" // MsiColor

constexpr size_t ROCCAT_KONE_INIT_REPORT_SIZE = 6;
constexpr size_t ROCCAT_KONE_COLOR_REPORT_SIZE = 46;
constexpr int ROCCAT_KONE_MAX_LEDS = 11; // NUM_LEDS, RoccatKoneAimoController.h:22

// Fixed 6-byte report sent once at device init (RoccatKoneAimoController.cpp
// SendInit, lines 72-77/82). No inputs: every byte is a constant.
std::array<uint8_t, ROCCAT_KONE_INIT_REPORT_SIZE> BuildKoneInitReport();

// Throws std::invalid_argument if colors.size() > ROCCAT_KONE_MAX_LEDS.
std::array<uint8_t, ROCCAT_KONE_COLOR_REPORT_SIZE> BuildKoneColorsReport(const std::vector<MsiColor>& colors);
