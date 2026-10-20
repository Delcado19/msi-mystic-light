// Unit tests for RazerBasiliskDriver (Task 11): direct-mode-only driver for
// the Razer Basilisk V3 mouse. Uses FakeHidDevice (tests/hid/FakeHidDevice.h)
// - no real hardware involved.
#include <QtTest/QtTest>

#include "device/RazerBasiliskDriver.h"
#include "hid/FakeHidDevice.h"

namespace {
template <size_t N>
std::vector<uint8_t> toVec(const std::array<uint8_t, N>& report)
{
    return std::vector<uint8_t>(report.begin(), report.end());
}
} // namespace

class RazerBasiliskDriverTest : public QObject
{
    Q_OBJECT

private slots:
    void testPushCanvasColors_sendsBothReportsInOrder();
    void testSetNativeMode_alwaysReturnsFalse();
    void testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue();
    void testPushCanvasColors_returnsFalseWhenHidNotOpen();
    void testPushCanvasColors_wrongChannelNameFailsGracefully();
    void testPushCanvasColors_rejectsMoreLedsThanMax();
    void testCreateDefaultChannel_hasDocumentedNameAndLedCount();
    void testSupportsMode_canvasOnlyNeverNative();
};

void RazerBasiliskDriverTest::testPushCanvasColors_sendsBothReportsInOrder()
{
    FakeHidDevice hid; hid.open(0x1532, 0x0099); // RAZER_VID/RAZER_BASILISK_V3_PID, RazerDevices.h:23/:157
    RazerBasiliskDriver driver(hid);

    std::vector<MsiColor> colors = {{1, 2, 3}, {4, 5, 6}};
    QVERIFY(driver.pushCanvasColors("RazerBasiliskV3", colors));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 2);
    // Order pinned to RazerController::SetLEDs: the custom frame (colors
    // report) is sent before razer_set_mode_custom()'s activation report.
    QCOMPARE(hid.sentReports[0], toVec(BuildBasiliskColorsReport(colors)));
    QCOMPARE(hid.sentReports[1], toVec(BuildBasiliskModeCustomReport()));
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 0); // sendFeatureReport, never write()
}

void RazerBasiliskDriverTest::testSetNativeMode_alwaysReturnsFalse()
{
    FakeHidDevice hid; hid.open(0x1532, 0x0099);
    RazerBasiliskDriver driver(hid);
    QVERIFY(!driver.setNativeMode("RazerBasiliskV3", NativeModeSettings{}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 0);
}

void RazerBasiliskDriverTest::testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue()
{
    FakeHidDevice hid; hid.open(0x1532, 0x0099);
    RazerBasiliskDriver driver(hid);
    QVERIFY(!driver.setChannelMode("RazerBasiliskV3", ChannelMode::Native));
    QVERIFY(driver.setChannelMode("RazerBasiliskV3", ChannelMode::Canvas));
    QCOMPARE(driver.channelMode("RazerBasiliskV3"), ChannelMode::Canvas);
}

void RazerBasiliskDriverTest::testPushCanvasColors_returnsFalseWhenHidNotOpen()
{
    FakeHidDevice hid; // never opened
    RazerBasiliskDriver driver(hid);
    QVERIFY(!driver.pushCanvasColors("RazerBasiliskV3", {{1, 2, 3}}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

void RazerBasiliskDriverTest::testPushCanvasColors_wrongChannelNameFailsGracefully()
{
    FakeHidDevice hid; hid.open(0x1532, 0x0099);
    RazerBasiliskDriver driver(hid);
    QVERIFY(!driver.pushCanvasColors("NoSuchChannel", {{1, 2, 3}}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

// RAZER_BASILISK_MAX_LEDS is 11; 12 LEDs must be rejected gracefully (never
// throw - BuildBasiliskColorsReport itself throws past this limit).
void RazerBasiliskDriverTest::testPushCanvasColors_rejectsMoreLedsThanMax()
{
    FakeHidDevice hid; hid.open(0x1532, 0x0099);
    RazerBasiliskDriver driver(hid);
    std::vector<MsiColor> tooMany(12, MsiColor{1, 2, 3});
    QVERIFY(!driver.pushCanvasColors("RazerBasiliskV3", tooMany));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

void RazerBasiliskDriverTest::testCreateDefaultChannel_hasDocumentedNameAndLedCount()
{
    Channel channel = RazerBasiliskDriver::createDefaultChannel();
    QCOMPARE(channel.name(), std::string("RazerBasiliskV3"));
    QCOMPARE(channel.ledLimit(), RAZER_BASILISK_MAX_LEDS);
    QCOMPARE(channel.totalLedCount(), RAZER_BASILISK_MAX_LEDS);
}

// Task 23: non-mutating capability query LightingStudioWindow uses to pick
// which modes this channel's row offers; must agree with setChannelMode.
void RazerBasiliskDriverTest::testSupportsMode_canvasOnlyNeverNative()
{
    FakeHidDevice hid; // deliberately not opened -- supportsMode needs no I/O
    RazerBasiliskDriver driver(hid);
    QVERIFY(driver.supportsMode("RazerBasiliskV3", ChannelMode::Canvas));
    QVERIFY(!driver.supportsMode("RazerBasiliskV3", ChannelMode::Native));
    QVERIFY(!driver.supportsMode("NoSuchChannel", ChannelMode::Canvas));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

QTEST_MAIN(RazerBasiliskDriverTest)
#include "test_razer_basilisk_driver.moc"
