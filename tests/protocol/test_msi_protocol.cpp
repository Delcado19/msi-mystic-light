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
    void testBuildReport0x50_option2MatchesRealCapture0xB5();
    void testBuildReport0x50_directionInwardSetsBit6();
    void testBuildReport0x50_recordStrideHoldsForLastZone();
    void testBuildReport0x50_storeTrueSetsLastByte();
    void testBuildReport0x51_twoLeds();
    void testBuildReport0x51_tooManyLedsThrows();
    void testBuildReport0x51_exactly240LedsDoesNotThrow();
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

void MsiProtocolTest::testBuildReport0x50_option2MatchesRealCapture0xB5()
{
    // Real capture from MSI290_TEMPLATE (docs/MB_800_HID_PROTOCOL.md, "Verification
    // against the existing capture template"): Option2 = 0xB5 decodes to
    // SelectAll=true, ColorSelection=User, Brightness=100% (5), Speed=Medium (1),
    // Direction=Out (false, i.e. bit 6 unset). Exercises bits 7/5/4-2/1-0 together
    // against an independently-verified real byte, not just hand-derived expectations.
    std::array<MsiZoneSettings, 18> zones{};
    zones[0] = MsiZoneSettings{
        MsiLightingMode::Off, {}, {}, {}, {},
        1, /*selectAll=*/true, /*directionInward=*/false, /*colorSelectionUser=*/true,
        /*brightness=*/5, /*speed=*/1, 30
    };
    auto report = BuildReport0x50(zones);
    QCOMPARE(report[1 + 14], static_cast<uint8_t>(0xB5));
}

void MsiProtocolTest::testBuildReport0x50_directionInwardSetsBit6()
{
    // Isolates bit 6 (directionInward): with every other Option2 bit false,
    // Option2 must be exactly 0x40. Catches a shift-amount transposition
    // between bits 6 and 7 that testBuildReport0x50_option2MatchesRealCapture0xB5
    // (which only sets bit 7, not bit 6) would not catch.
    std::array<MsiZoneSettings, 18> zones{};
    zones[0] = MsiZoneSettings{
        MsiLightingMode::Off, {}, {}, {}, {},
        1, /*selectAll=*/false, /*directionInward=*/true, /*colorSelectionUser=*/false,
        /*brightness=*/0, /*speed=*/0, 30
    };
    auto report = BuildReport0x50(zones);
    QCOMPARE(report[1 + 14], static_cast<uint8_t>(0x40));
}

void MsiProtocolTest::testBuildReport0x50_recordStrideHoldsForLastZone()
{
    // All-default zones (brightness=5 -> Option2=20, cycleNum=30). Checks the
    // last record (index 17) lands at its predicted absolute offset, catching a
    // stride bug (e.g. i*15 instead of i*16) that corrupts every zone after the
    // first while leaving record-0-only assertions green.
    std::array<MsiZoneSettings, 18> zones{};
    auto report = BuildReport0x50(zones);
    QCOMPARE(report[287], static_cast<uint8_t>(20)); // record 17's Option2
    QCOMPARE(report[288], static_cast<uint8_t>(30)); // record 17's cycleNum
}

void MsiProtocolTest::testBuildReport0x50_storeTrueSetsLastByte()
{
    std::array<MsiZoneSettings, 18> zones{};
    auto report = BuildReport0x50(zones, /*store=*/true);
    QCOMPARE(report[289], static_cast<uint8_t>(1));
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

void MsiProtocolTest::testBuildReport0x51_exactly240LedsDoesNotThrow()
{
    // Boundary case: 240 is the max that fits (7 header bytes + 240*3 = 727),
    // so it must NOT throw, unlike the 241 case above.
    std::vector<MsiColor> colors(240);
    auto report = BuildReport0x51(MsiGen1AdvArea::JARGB1, colors);
    QCOMPARE(report[6], static_cast<uint8_t>(240));
}

QTEST_MAIN(MsiProtocolTest)
#include "test_msi_protocol.moc"
