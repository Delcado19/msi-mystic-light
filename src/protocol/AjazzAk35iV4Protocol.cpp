// AJAZZ AK35I V4 Direct RGB encoder; see the header for provenance (owner captures, no third-party code).
#include "protocol/AjazzAk35iV4Protocol.h"

#include <stdexcept>

// From rgb-keyboard.xml via the fixture's led_map (SHA-256 493bb118...b48), order 0..104.
const std::array<std::uint8_t, AJAZZ_V4_LEDS> AJAZZ_V4_LIGHT_IDS{
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 112, 113, 115, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 103, 116, 117, 118, 32, 33, 34, 122, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    85, 119, 120, 121, 50, 51, 52, 123, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 110, 68, 69, 70,
    73, 111, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 101, 86, 87, 88, 106, 91, 92, 93, 94, 95, 96,
    97, 98, 99, 100, 102, 104, 105};

AjazzReport BuildAjazzCommand(std::uint8_t opcode, std::uint8_t arg8)
{
    AjazzReport report{};
    report[1] = 0x04; report[2] = opcode; report[9] = arg8;
    return report;
}

std::array<AjazzReport, 8> BuildAjazzDirectChunks(const std::vector<MsiColor>& colors)
{
    if (colors.size() != static_cast<std::size_t>(AJAZZ_V4_LEDS))
        throw std::invalid_argument("BuildAjazzDirectChunks: exactly 105 colors required");
    std::array<AjazzReport, 8> chunks{};
    for (int k = 0; k < AJAZZ_V4_LEDS; ++k) {
        auto& chunk = chunks[k / 16];
        const int offset = 1 + 4 * (k % 16);
        chunk[offset] = AJAZZ_V4_LIGHT_IDS[k];
        chunk[offset + 1] = colors[k].r; chunk[offset + 2] = colors[k].g; chunk[offset + 3] = colors[k].b;
    }
    return chunks;
}

bool AjazzAckOk(const std::uint8_t* report, std::size_t length, std::uint8_t opcode)
{
    return report && length == AJAZZ_V4_REPORT && report[0] == 0 && report[1] == 0x04 && report[2] == opcode && report[4] == 0x01;
}
