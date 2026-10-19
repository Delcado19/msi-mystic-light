// Unit tests for the SteelSeries Apex 3 T-Zone protocol encoders. Ported
// from OpenRGB's SteelSeriesApexTZoneController::SetColor
// (Controllers/SteelSeriesController/SteelSeriesApexTZoneController/SteelSeriesApexTZoneController.cpp:56-80).
// Pure data encoding, no HID/device access.
#include <QtTest/QtTest>

#include "protocol/SteelSeriesApex3Protocol.h"

class SteelSeriesApex3ProtocolTest : public QObject
{
    Q_OBJECT

private slots:
    void testBuildApex3BrightnessPacket_layout();
    void testBuildApex3ColorsPacket_layout();
    void testBuildApex3ColorsPacket_tooManyZonesThrows();
};

void SteelSeriesApex3ProtocolTest::testBuildApex3BrightnessPacket_layout()
{
    auto packet = BuildApex3BrightnessPacket(3);
    QCOMPARE(static_cast<int>(packet.size()), static_cast<int>(STEELSERIES_APEX3_PACKET_SIZE));
    QCOMPARE(packet[0], static_cast<uint8_t>(0x00));
    QCOMPARE(packet[1], static_cast<uint8_t>(0x0A));
    QCOMPARE(packet[3], static_cast<uint8_t>(3));
    for (size_t i = 0; i < packet.size(); ++i) {
        if (i == 1 || i == 3) continue;
        QCOMPARE(packet[i], static_cast<uint8_t>(0x00));
    }
}

void SteelSeriesApex3ProtocolTest::testBuildApex3ColorsPacket_layout()
{
    std::vector<MsiColor> zones = {{1, 2, 3}, {4, 5, 6}};
    auto packet = BuildApex3ColorsPacket(zones);
    QCOMPARE(packet[0], static_cast<uint8_t>(0x00));
    QCOMPARE(packet[1], static_cast<uint8_t>(0x0B));
    std::array<uint8_t, 6> expected = {1, 2, 3, 4, 5, 6};
    for (size_t i = 0; i < 6; ++i) {
        QCOMPARE(packet[3 + i], expected[i]);
    }
    for (size_t i = 9; i < packet.size(); ++i) {
        QCOMPARE(packet[i], static_cast<uint8_t>(0x00));
    }
}

void SteelSeriesApex3ProtocolTest::testBuildApex3ColorsPacket_tooManyZonesThrows()
{
    std::vector<MsiColor> zones(11, MsiColor{1, 2, 3}); // STEELSERIES_APEX3_MAX_ZONES is 10
    QVERIFY_EXCEPTION_THROWN(BuildApex3ColorsPacket(zones), std::invalid_argument);
}

QTEST_MAIN(SteelSeriesApex3ProtocolTest)
#include "test_steelseries_apex3_protocol.moc"
