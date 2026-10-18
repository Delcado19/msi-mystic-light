#include "protocol/MsiProtocol.h"

#include <stdexcept>

// Byte layout and bit-packing formulas are pinned exactly to
// docs/MB_800_HID_PROTOCOL.md (report 0x50, "Gen1"). Do not approximate.
std::array<uint8_t, 290> BuildReport0x50(const std::array<MsiZoneSettings, 18>& zones, bool store)
{
    std::array<uint8_t, 290> report{};
    report[0] = 0x50;

    for (size_t i = 0; i < zones.size(); ++i) {
        const MsiZoneSettings& z = zones[i];
        const uint8_t option1 = (z.colorCount - 1) & 0x03;
        const uint8_t option2 = (static_cast<uint8_t>(z.selectAll) << 7)
            | (static_cast<uint8_t>(z.directionInward) << 6)
            | (static_cast<uint8_t>(z.colorSelectionUser) << 5)
            | ((z.brightness & 0x07) << 2)
            | (z.speed & 0x03);

        uint8_t* record = &report[1 + i * 16];
        record[0] = static_cast<uint8_t>(z.lightingMode);
        record[1] = z.color1.r; record[2] = z.color1.g; record[3] = z.color1.b;
        record[4] = z.color2.r; record[5] = z.color2.g; record[6] = z.color2.b;
        record[7] = z.color3.r; record[8] = z.color3.g; record[9] = z.color3.b;
        record[10] = z.color4.r; record[11] = z.color4.g; record[12] = z.color4.b;
        record[13] = option1;
        record[14] = option2;
        record[15] = z.cycleNum;
    }

    report[289] = store ? 1 : 0;
    return report;
}

// Byte layout pinned exactly to docs/MB_800_HID_PROTOCOL.md (report 0x51,
// "Gen1_Adv", per-LED addressing). 240 is the max LED count that fits in the
// 727-byte report (7 header bytes + 240*3 = 727).
std::array<uint8_t, 727> BuildReport0x51(MsiGen1AdvArea area, const std::vector<MsiColor>& colors)
{
    if (colors.size() > 240) {
        throw std::invalid_argument("BuildReport0x51: colors.size() exceeds 240 LEDs");
    }

    std::array<uint8_t, 727> report{};
    report[0] = 0x51;
    report[1] = 0x09;
    const uint16_t areaValue = static_cast<uint16_t>(area);
    report[2] = static_cast<uint8_t>(areaValue >> 8);   // big-endian high byte
    report[3] = static_cast<uint8_t>(areaValue & 0xFF); // big-endian low byte
    report[4] = 0x00;
    report[5] = 0x00;
    report[6] = static_cast<uint8_t>(colors.size());

    for (size_t i = 0; i < colors.size(); ++i) {
        report[7 + i * 3] = colors[i].r;
        report[8 + i * 3] = colors[i].g;
        report[9 + i * 3] = colors[i].b;
    }

    return report;
}
