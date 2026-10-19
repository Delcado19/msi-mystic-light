#include "protocol/RoccatKoneProtocol.h"

#include <stdexcept>

// Byte layout pinned exactly to RoccatKoneAimoController::SendInit
// (RoccatKoneAimoController.cpp:60-83): a constant 6-byte buffer, no
// per-call inputs.
std::array<uint8_t, ROCCAT_KONE_INIT_REPORT_SIZE> BuildKoneInitReport()
{
    return {0x0E, 0x06, 0x01, 0x01, 0x00, 0xFF};
}

// Byte layout pinned exactly to RoccatKoneAimoController's constructor
// (.cpp:16-30, header bytes 0x0D/0x2E) and SetChannelColors (.cpp:85-100,
// per-LED RGB at 0x02 + ledIndex*4 + {R,G,B}_OFFSET). colors[i] is LED i
// (ROCCAT_KONE_AIMO_CHANNEL's SCROLL_WHEEL=0 .. LOWER_RIGHT=10, .h:30-37).
std::array<uint8_t, ROCCAT_KONE_COLOR_REPORT_SIZE> BuildKoneColorsReport(const std::vector<MsiColor>& colors)
{
    if (colors.size() > static_cast<size_t>(ROCCAT_KONE_MAX_LEDS)) {
        throw std::invalid_argument("BuildKoneColorsReport: colors.size() exceeds ROCCAT_KONE_MAX_LEDS");
    }

    std::array<uint8_t, ROCCAT_KONE_COLOR_REPORT_SIZE> report{};
    report[0] = 0x0D;
    report[1] = 0x2E;
    for (size_t i = 0; i < colors.size(); ++i) {
        const size_t usbIdx = 0x02 + (i * 4);
        report[usbIdx + 0] = colors[i].r;
        report[usbIdx + 1] = colors[i].g;
        report[usbIdx + 2] = colors[i].b;
    }
    return report;
}
