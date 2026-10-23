#include "MsiProtocol.h"

#include <stdexcept>

std::array<std::uint8_t, 290> BuildReport0x50(
    const std::array<MsiZoneSettings, 18>& zones, bool store)
{
    std::array<std::uint8_t, 290> report{};
    report[0] = 0x50;
    for (std::size_t i = 0; i < zones.size(); ++i) {
        const auto& zone = zones[i];
        if (static_cast<std::uint8_t>(zone.lightingMode) > 10 ||
            zone.colorCount < 1 || zone.colorCount > 4 ||
            zone.brightness > 5 || zone.speed > 2) {
            throw std::invalid_argument("Invalid MSI zone settings");
        }
        auto offset = 1 + i * 16;
        report[offset++] = static_cast<std::uint8_t>(zone.lightingMode);
        for (const auto color : {zone.color1, zone.color2, zone.color3, zone.color4}) {
            report[offset++] = color.r;
            report[offset++] = color.g;
            report[offset++] = color.b;
        }
        report[offset++] = zone.colorCount - 1;
        report[offset++] = static_cast<std::uint8_t>(
            (zone.selectAll << 7) | (zone.directionInward << 6) |
            (zone.colorSelectionUser << 5) | (zone.brightness << 2) | zone.speed);
        report[offset] = zone.cycleNum;
    }
    report.back() = store ? 1 : 0;
    return report;
}

std::array<std::uint8_t, 727> BuildReport0x51(
    MsiGen1AdvArea area, const std::vector<MsiColor>& colors)
{
    const auto address = static_cast<std::uint16_t>(area);
    const bool validArea = (address >= 0x0200 && address <= 0x0201) ||
        (address >= 0x0300 && address <= 0x0304) ||
        (address >= 0x0400 && address <= 0x0402) || address == 0x0600 ||
        (address >= 0x0700 && address <= 0x0705) || address == 0x0800;
    if (!validArea || colors.size() > 240) {
        throw std::invalid_argument("Invalid MSI area or LED count");
    }
    std::array<std::uint8_t, 727> report{};
    report[0] = 0x51;
    report[1] = 0x09;
    report[2] = static_cast<std::uint8_t>(address >> 8);
    report[3] = static_cast<std::uint8_t>(address & 0xff);
    report[6] = static_cast<std::uint8_t>(colors.size());
    std::size_t offset = 7;
    for (const auto color : colors) {
        report[offset++] = color.r;
        report[offset++] = color.g;
        report[offset++] = color.b;
    }
    return report;
}
