#include "protocol/MsiProtocol.h"

#include <QtTest>
#include <algorithm>
#include <stdexcept>

class MsiProtocolTest : public QObject {
    Q_OBJECT
private slots:
    void nativeRecords()
    {
        std::array<MsiZoneSettings, 18> zones{};
        auto& zone = zones[17];
        zone.lightingMode = MsiLightingMode::Breathing;
        zone.color1 = {1, 2, 3};
        zone.color2 = {4, 5, 6};
        zone.color3 = {7, 8, 9};
        zone.color4 = {10, 11, 12};
        zone.colorCount = 4;
        zone.selectAll = true;
        zone.directionInward = true;
        zone.colorSelectionUser = true;
        zone.speed = 2;
        zone.cycleNum = 42;
        const auto report = BuildReport0x50(zones);
        QCOMPARE(report[0], 0x50);
        const std::array<std::uint8_t, 16> expected{
            4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 3, 0xf6, 42};
        QVERIFY(std::equal(expected.begin(), expected.end(), report.begin() + 273));
        const std::array<std::uint8_t, 16> off{
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x14, 30};
        for (std::size_t i = 0; i < 17; ++i) {
            QVERIFY(std::equal(off.begin(), off.end(), report.begin() + 1 + i * 16));
        }
        QCOMPARE(report[289], 0);
        QCOMPARE(BuildReport0x50(zones, true)[289], 1);
    }

    void optionBits()
    {
        std::array<MsiZoneSettings, 18> zones{};
        auto& zone = zones[0];
        zone.selectAll = true;
        zone.colorSelectionUser = true;
        zone.speed = 1;
        // Captured Option2 0xB5 differs from 0x35 only in SelectAll.
        QCOMPARE(BuildReport0x50(zones)[15], 0xb5);
        zone.selectAll = false;
        QCOMPARE(BuildReport0x50(zones)[15], 0x35);
        for (std::uint8_t count = 1; count <= 4; ++count) {
            zone.colorCount = count;
            QCOMPARE(BuildReport0x50(zones)[14], count - 1);
        }
    }

    void invalidNativeSettings()
    {
        std::array<MsiZoneSettings, 18> zones{};
        for (auto count : {0, 5, 255}) {
            zones[0].colorCount = static_cast<std::uint8_t>(count);
            QVERIFY_EXCEPTION_THROWN(BuildReport0x50(zones), std::invalid_argument);
        }
        zones[0] = {};
        zones[0].brightness = 6;
        QVERIFY_EXCEPTION_THROWN(BuildReport0x50(zones), std::invalid_argument);
        zones[0] = {};
        zones[0].speed = 3;
        QVERIFY_EXCEPTION_THROWN(BuildReport0x50(zones), std::invalid_argument);
        zones[0] = {};
        zones[0].lightingMode = static_cast<MsiLightingMode>(11);
        QVERIFY_EXCEPTION_THROWN(BuildReport0x50(zones), std::invalid_argument);
    }

    void perLedLayout()
    {
        const auto report = BuildReport0x51(MsiGen1AdvArea::JARGB2,
            {{0x11, 0x22, 0x33}, {0xaa, 0xbb, 0xcc}});
        const std::array<std::uint8_t, 13> expected{
            0x51, 9, 4, 1, 0, 0, 2, 0x11, 0x22, 0x33, 0xaa, 0xbb, 0xcc};
        QVERIFY(std::equal(expected.begin(), expected.end(), report.begin()));
        QVERIFY(std::all_of(report.begin() + 13, report.end(), [](auto b) { return b == 0; }));
        const auto ez = BuildReport0x51(MsiGen1AdvArea::JAF, {});
        QCOMPARE(ez[2], 8);
        QCOMPARE(ez[3], 0);
        QCOMPARE(ez[6], 0);
        QVERIFY(std::all_of(ez.begin() + 7, ez.end(), [](auto b) { return b == 0; }));
    }

    void perLedBoundaries()
    {
        const auto report = BuildReport0x51(MsiGen1AdvArea::JPIPE1,
            std::vector<MsiColor>(240, {1, 2, 3}));
        QCOMPARE(report[6], 240);
        QCOMPARE(report[724], 1);
        QCOMPARE(report[725], 2);
        QCOMPARE(report[726], 3);
        QVERIFY_EXCEPTION_THROWN(BuildReport0x51(MsiGen1AdvArea::JARGB1,
            std::vector<MsiColor>(241)), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(BuildReport0x51(static_cast<MsiGen1AdvArea>(0x0403), {}),
            std::invalid_argument);
    }
};

QTEST_APPLESS_MAIN(MsiProtocolTest)
#include "test_msi_protocol.moc"
