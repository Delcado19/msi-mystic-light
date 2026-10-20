// Unit tests for RoccatKoneDriver (Task 10): direct-mode-only driver for
// the Roccat Kone Aimo mouse. Uses FakeHidDevice (tests/hid/FakeHidDevice.h)
// - no real hardware involved.
#include <QtTest/QtTest>

#include "device/RoccatKoneDriver.h"
#include "hid/FakeHidDevice.h"

namespace {
template <size_t N>
std::vector<uint8_t> toVec(const std::array<uint8_t, N>& report)
{
    return std::vector<uint8_t>(report.begin(), report.end());
}
} // namespace

class RoccatKoneDriverTest : public QObject
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

void RoccatKoneDriverTest::testPushCanvasColors_sendsBothReportsInOrder()
{
    FakeHidDevice hid; hid.open(0x1E7D, 0x2E27); // ROCCAT_VID/ROCCAT_KONE_AIMO_PID, RoccatControllerDetect.cpp:35/:50
    RoccatKoneDriver driver(hid);

    std::vector<MsiColor> colors = {{1, 2, 3}, {4, 5, 6}};
    QVERIFY(driver.pushCanvasColors("RoccatKone", colors));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 2);
    // Order pinned to RoccatKoneAimoController: SendInit's init report is
    // sent before the color report (SetChannelColors/SendUpdate).
    QCOMPARE(hid.sentReports[0], toVec(BuildKoneInitReport()));
    QCOMPARE(hid.sentReports[1], toVec(BuildKoneColorsReport(colors)));
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 0); // sendFeatureReport, never write()
}

void RoccatKoneDriverTest::testSetNativeMode_alwaysReturnsFalse()
{
    FakeHidDevice hid; hid.open(0x1E7D, 0x2E27);
    RoccatKoneDriver driver(hid);
    QVERIFY(!driver.setNativeMode("RoccatKone", NativeModeSettings{}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 0);
}

void RoccatKoneDriverTest::testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue()
{
    FakeHidDevice hid; hid.open(0x1E7D, 0x2E27);
    RoccatKoneDriver driver(hid);
    QVERIFY(!driver.setChannelMode("RoccatKone", ChannelMode::Native));
    QVERIFY(driver.setChannelMode("RoccatKone", ChannelMode::Canvas));
    QCOMPARE(driver.channelMode("RoccatKone"), ChannelMode::Canvas);
}

void RoccatKoneDriverTest::testPushCanvasColors_returnsFalseWhenHidNotOpen()
{
    FakeHidDevice hid; // never opened
    RoccatKoneDriver driver(hid);
    QVERIFY(!driver.pushCanvasColors("RoccatKone", {{1, 2, 3}}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

void RoccatKoneDriverTest::testPushCanvasColors_wrongChannelNameFailsGracefully()
{
    FakeHidDevice hid; hid.open(0x1E7D, 0x2E27);
    RoccatKoneDriver driver(hid);
    QVERIFY(!driver.pushCanvasColors("NoSuchChannel", {{1, 2, 3}}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

// ROCCAT_KONE_MAX_LEDS is 11; 12 LEDs must be rejected gracefully (never
// throw - BuildKoneColorsReport itself throws past this limit).
void RoccatKoneDriverTest::testPushCanvasColors_rejectsMoreLedsThanMax()
{
    FakeHidDevice hid; hid.open(0x1E7D, 0x2E27);
    RoccatKoneDriver driver(hid);
    std::vector<MsiColor> tooMany(12, MsiColor{1, 2, 3});
    QVERIFY(!driver.pushCanvasColors("RoccatKone", tooMany));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

void RoccatKoneDriverTest::testCreateDefaultChannel_hasDocumentedNameAndLedCount()
{
    Channel channel = RoccatKoneDriver::createDefaultChannel();
    QCOMPARE(channel.name(), std::string("RoccatKone"));
    QCOMPARE(channel.ledLimit(), ROCCAT_KONE_MAX_LEDS);
    QCOMPARE(channel.totalLedCount(), ROCCAT_KONE_MAX_LEDS);
}

// Task 23: non-mutating capability query LightingStudioWindow uses to pick
// which modes this channel's row offers; must agree with setChannelMode.
void RoccatKoneDriverTest::testSupportsMode_canvasOnlyNeverNative()
{
    FakeHidDevice hid; // deliberately not opened -- supportsMode needs no I/O
    RoccatKoneDriver driver(hid);
    QVERIFY(driver.supportsMode("RoccatKone", ChannelMode::Canvas));
    QVERIFY(!driver.supportsMode("RoccatKone", ChannelMode::Native));
    QVERIFY(!driver.supportsMode("NoSuchChannel", ChannelMode::Canvas));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

QTEST_MAIN(RoccatKoneDriverTest)
#include "test_roccat_kone_driver.moc"
