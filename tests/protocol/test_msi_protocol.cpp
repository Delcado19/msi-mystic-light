// Unit tests for the MSI wire-protocol encoders (report 0x50 / 0x51).
// Pure data encoding, no HID/device access.
#include <QtTest/QtTest>

#include "protocol/MsiProtocol.h"

class MsiProtocolTest : public QObject
{
    Q_OBJECT

private slots:
    void testBuildReport0x50_defaultZonesAllZero();
    void testBuildReport0x50_knownZoneMatchesVendorDefault();
    void testBuildReport0x51_twoLeds();
    void testBuildReport0x51_tooManyLedsThrows();
};

void MsiProtocolTest::testBuildReport0x50_defaultZonesAllZero()
{
    std::array<MsiZoneSettings, 18> zones{}; // all default (Off, black, colorCount=1)
    auto report = BuildReport0x50(zones);
    QCOMPARE(report[0], static_cast<uint8_t>(0x50));
    QCOMPARE(report[1], static_cast<uint8_t>(0)); // lightingMode Off
    QCOMPARE(report[289], static_cast<uint8_t>(0)); // store=false default
}

void MsiProtocolTest::testBuildReport0x50_knownZoneMatchesVendorDefault()
{
    // This exact zone matches MSI's own DefaultSetting_Gen1 for JARGB1,
    // independently confirmed against the real vendor driver this session.
    std::array<MsiZoneSettings, 18> zones{};
    zones[0] = MsiZoneSettings{
        MsiLightingMode::Wave, {255,0,0}, {0,255,0}, {0,0,255}, {255,255,255},
        4, false, false, false, 5, 0, 30
    };
    auto report = BuildReport0x50(zones);
    std::array<uint8_t, 16> expectedRecord0 = {1,255,0,0,0,255,0,0,0,255,255,255,255,3,20,30};
    for (size_t i = 0; i < 16; ++i) {
        QCOMPARE(report[1 + i], expectedRecord0[i]);
    }
}

void MsiProtocolTest::testBuildReport0x51_twoLeds()
{
    std::vector<MsiColor> colors = {{255,0,0},{0,255,0}};
    auto report = BuildReport0x51(MsiGen1AdvArea::JARGB1, colors);
    QCOMPARE(report[0], static_cast<uint8_t>(0x51));
    QCOMPARE(report[1], static_cast<uint8_t>(0x09));
    QCOMPARE(report[2], static_cast<uint8_t>(0x04)); // JARGB1 = 0x0400, high byte
    QCOMPARE(report[3], static_cast<uint8_t>(0x00)); // low byte
    QCOMPARE(report[6], static_cast<uint8_t>(2));    // LED count
    std::array<uint8_t, 6> expectedColors = {255,0,0, 0,255,0};
    for (size_t i = 0; i < 6; ++i) {
        QCOMPARE(report[7 + i], expectedColors[i]);
    }
    QCOMPARE(report[726], static_cast<uint8_t>(0)); // tail padding
}

void MsiProtocolTest::testBuildReport0x51_tooManyLedsThrows()
{
    std::vector<MsiColor> colors(241);
    QVERIFY_EXCEPTION_THROWN(BuildReport0x51(MsiGen1AdvArea::JARGB1, colors), std::invalid_argument);
}

QTEST_MAIN(MsiProtocolTest)
#include "test_msi_protocol.moc"
