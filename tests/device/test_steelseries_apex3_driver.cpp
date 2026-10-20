// Unit tests for SteelSeriesApex3Driver (Task 9): direct-mode-only driver
// for the SteelSeries Apex 3 keyboard's T-Zone protocol. Uses FakeHidDevice
// (tests/hid/FakeHidDevice.h) - no real hardware involved.
#include <QtTest/QtTest>

#include "device/SteelSeriesApex3Driver.h"
#include "hid/FakeHidDevice.h"

namespace {
std::vector<uint8_t> toVec(const std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE>& packet)
{
    return std::vector<uint8_t>(packet.begin(), packet.end());
}
} // namespace

class SteelSeriesApex3DriverTest : public QObject
{
    Q_OBJECT

private slots:
    void testPushCanvasColors_sendsBothPacketsInOrder();
    void testSetNativeMode_alwaysReturnsFalse();
    void testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue();
    void testPushCanvasColors_returnsFalseWhenHidNotOpen();
    void testPushCanvasColors_wrongChannelNameFailsGracefully();
    void testPushCanvasColors_rejectsMoreZonesThanMax();
    void testCreateDefaultChannel_hasDocumentedNameAndZoneCount();
    void testSupportsMode_canvasOnlyNeverNative();
};

void SteelSeriesApex3DriverTest::testPushCanvasColors_sendsBothPacketsInOrder()
{
    FakeHidDevice hid; hid.open(0x1038, 0x1612); // SteelSeries VID, Apex 3 PID
    SteelSeriesApex3Driver driver(hid);

    std::vector<MsiColor> colors = {{1, 2, 3}, {4, 5, 6}};
    QVERIFY(driver.pushCanvasColors("SteelSeriesApex3", colors));
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 2);
    // Order pinned to SteelSeriesApexTZoneController::SetColor: brightness
    // packet (cmd 0x0A) is sent before the colors packet (cmd 0x0B).
    // 100 (not 3) is the driver's hardcoded default - see kSteelSeriesApex3BrightnessMax
    // in SteelSeriesApex3Driver.cpp, hardware-confirmed against OpenRGB's
    // STEELSERIES_TZ_BRIGHTNESS_MAX (0x64).
    QCOMPARE(hid.writtenReports[0], toVec(BuildApex3BrightnessPacket(100)));
    QCOMPARE(hid.writtenReports[1], toVec(BuildApex3ColorsPacket(colors)));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0); // write(), never sendFeatureReport
}

void SteelSeriesApex3DriverTest::testSetNativeMode_alwaysReturnsFalse()
{
    FakeHidDevice hid; hid.open(0x1038, 0x1612);
    SteelSeriesApex3Driver driver(hid);
    QVERIFY(!driver.setNativeMode("SteelSeriesApex3", NativeModeSettings{}));
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 0);
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

void SteelSeriesApex3DriverTest::testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue()
{
    FakeHidDevice hid; hid.open(0x1038, 0x1612);
    SteelSeriesApex3Driver driver(hid);
    QVERIFY(!driver.setChannelMode("SteelSeriesApex3", ChannelMode::Native));
    QVERIFY(driver.setChannelMode("SteelSeriesApex3", ChannelMode::Canvas));
    QCOMPARE(driver.channelMode("SteelSeriesApex3"), ChannelMode::Canvas);
}

void SteelSeriesApex3DriverTest::testPushCanvasColors_returnsFalseWhenHidNotOpen()
{
    FakeHidDevice hid; // never opened
    SteelSeriesApex3Driver driver(hid);
    QVERIFY(!driver.pushCanvasColors("SteelSeriesApex3", {{1, 2, 3}}));
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 0);
}

void SteelSeriesApex3DriverTest::testPushCanvasColors_wrongChannelNameFailsGracefully()
{
    FakeHidDevice hid; hid.open(0x1038, 0x1612);
    SteelSeriesApex3Driver driver(hid);
    QVERIFY(!driver.pushCanvasColors("NoSuchChannel", {{1, 2, 3}}));
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 0);
}

// STEELSERIES_APEX3_MAX_ZONES is 10; 11 zones must be rejected gracefully
// (never throw - BuildApex3ColorsPacket itself throws past this limit).
void SteelSeriesApex3DriverTest::testPushCanvasColors_rejectsMoreZonesThanMax()
{
    FakeHidDevice hid; hid.open(0x1038, 0x1612);
    SteelSeriesApex3Driver driver(hid);
    std::vector<MsiColor> tooMany(11, MsiColor{1, 2, 3});
    QVERIFY(!driver.pushCanvasColors("SteelSeriesApex3", tooMany));
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 0);
}

void SteelSeriesApex3DriverTest::testCreateDefaultChannel_hasDocumentedNameAndZoneCount()
{
    Channel channel = SteelSeriesApex3Driver::createDefaultChannel();
    QCOMPARE(channel.name(), std::string("SteelSeriesApex3"));
    QCOMPARE(channel.ledLimit(), STEELSERIES_APEX3_MAX_ZONES);
    QCOMPARE(channel.totalLedCount(), STEELSERIES_APEX3_MAX_ZONES);
}

// Task 23: non-mutating capability query LightingStudioWindow uses to pick
// which modes this channel's row offers; must agree with setChannelMode.
void SteelSeriesApex3DriverTest::testSupportsMode_canvasOnlyNeverNative()
{
    FakeHidDevice hid; // deliberately not opened -- supportsMode needs no I/O
    SteelSeriesApex3Driver driver(hid);
    QVERIFY(driver.supportsMode("SteelSeriesApex3", ChannelMode::Canvas));
    QVERIFY(!driver.supportsMode("SteelSeriesApex3", ChannelMode::Native));
    QVERIFY(!driver.supportsMode("NoSuchChannel", ChannelMode::Canvas));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

QTEST_MAIN(SteelSeriesApex3DriverTest)
#include "test_steelseries_apex3_driver.moc"
