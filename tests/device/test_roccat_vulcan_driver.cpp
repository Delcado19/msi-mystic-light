// Unit tests for RoccatVulcanDriver (Task 12): direct-mode-only driver for
// the Roccat Vulcan 100/120 Aimo keyboard. Uses FakeHidDevice
// (tests/hid/FakeHidDevice.h) for BOTH the control and LED HID interfaces -
// no real hardware involved.
#include <QtTest/QtTest>

#include "device/RoccatVulcanDriver.h"
#include "hid/FakeHidDevice.h"

namespace {
template <size_t N>
std::vector<uint8_t> toVec(const std::array<uint8_t, N>& report)
{
    return std::vector<uint8_t>(report.begin(), report.end());
}
} // namespace

class RoccatVulcanDriverTest : public QObject
{
    Q_OBJECT

private slots:
    void testPushCanvasColors_sendsPacketsToLedHidNotControlHid();
    void testSetNativeMode_alwaysReturnsFalse();
    void testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue();
    void testSetChannelMode_canvasReturnsFalseWhenControlHidNotOpen();
    void testPushCanvasColors_returnsFalseWhenEitherHidNotOpen();
    void testPushCanvasColors_wrongChannelNameFailsGracefully();
    void testPushCanvasColors_rejectsMoreLedsThanMax();
    void testCreateDefaultChannel_hasDocumentedNameAndLedCount();
    void testSupportsMode_canvasOnlyNeverNative();
};

// Color pushes go to the LED interface only (hid_write(dev_led, ...),
// RoccatVulcanKeyboardController.cpp:388-394), never the control
// interface - this driver does not implement the control-interface mode
// handshake (SendMode/EnableDirect), see RoccatVulcanDriver.h.
void RoccatVulcanDriverTest::testPushCanvasColors_sendsPacketsToLedHidNotControlHid()
{
    FakeHidDevice controlHid; controlHid.open(0x1E7D, 0x307A, 0); // placeholder VID/PID/interface, wired for real in Task 18
    FakeHidDevice ledHid; ledHid.open(0x1E7D, 0x307A, 1);
    RoccatVulcanDriver driver(controlHid, ledHid);

    std::vector<MsiColor> colors = {{1, 2, 3}, {4, 5, 6}};
    QVERIFY(driver.pushCanvasColors("RoccatVulcan", colors));

    QCOMPARE(static_cast<int>(ledHid.writtenReports.size()), VULCAN_PACKET_COUNT);
    auto packets = BuildVulcanColorPackets(colors);
    std::vector<std::vector<uint8_t>> expected;
    for (const auto& packet : packets) expected.push_back(toVec(packet));
    QCOMPARE(ledHid.writtenReports, expected);

    QCOMPARE(static_cast<int>(controlHid.writtenReports.size()), 0);
    QCOMPARE(static_cast<int>(controlHid.sentReports.size()), 0);
}

void RoccatVulcanDriverTest::testSetNativeMode_alwaysReturnsFalse()
{
    FakeHidDevice controlHid; controlHid.open(0x1E7D, 0x307A);
    FakeHidDevice ledHid; ledHid.open(0x1E7D, 0x307A);
    RoccatVulcanDriver driver(controlHid, ledHid);
    QVERIFY(!driver.setNativeMode("RoccatVulcan", NativeModeSettings{}));
    QCOMPARE(static_cast<int>(controlHid.sentReports.size()), 0);
    QCOMPARE(static_cast<int>(ledHid.writtenReports.size()), 0);
}

// setChannelMode(Canvas) also sends EnableDirect(true)
// (RoccatVulcanKeyboardController.cpp:314's one-time "entering Direct
// mode" call) to controlHid_ as a feature report - never to ledHid_.
void RoccatVulcanDriverTest::testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue()
{
    FakeHidDevice controlHid; controlHid.open(0x1E7D, 0x307A);
    FakeHidDevice ledHid; ledHid.open(0x1E7D, 0x307A);
    RoccatVulcanDriver driver(controlHid, ledHid);
    QVERIFY(!driver.setChannelMode("RoccatVulcan", ChannelMode::Native));
    QCOMPARE(static_cast<int>(controlHid.sentReports.size()), 0);

    QVERIFY(driver.setChannelMode("RoccatVulcan", ChannelMode::Canvas));
    QCOMPARE(driver.channelMode("RoccatVulcan"), ChannelMode::Canvas);
    QCOMPARE(static_cast<int>(controlHid.sentReports.size()), 1);
    QCOMPARE(controlHid.sentReports[0], toVec(BuildVulcanEnableDirectReport(true)));
    QCOMPARE(static_cast<int>(ledHid.writtenReports.size()), 0);
}

void RoccatVulcanDriverTest::testSetChannelMode_canvasReturnsFalseWhenControlHidNotOpen()
{
    FakeHidDevice controlHid; // not opened
    FakeHidDevice ledHid; ledHid.open(0x1E7D, 0x307A);
    RoccatVulcanDriver driver(controlHid, ledHid);
    QVERIFY(!driver.setChannelMode("RoccatVulcan", ChannelMode::Canvas));
    QCOMPARE(static_cast<int>(controlHid.sentReports.size()), 0);
}

// Both HIDs must be open for a color push to succeed (brief Step 6):
// control-only open, LED-only open, and neither open must all fail.
void RoccatVulcanDriverTest::testPushCanvasColors_returnsFalseWhenEitherHidNotOpen()
{
    {
        FakeHidDevice controlHid; // not opened
        FakeHidDevice ledHid; ledHid.open(0x1E7D, 0x307A);
        RoccatVulcanDriver driver(controlHid, ledHid);
        QVERIFY(!driver.pushCanvasColors("RoccatVulcan", {{1, 2, 3}}));
        QCOMPARE(static_cast<int>(ledHid.writtenReports.size()), 0);
    }
    {
        FakeHidDevice controlHid; controlHid.open(0x1E7D, 0x307A);
        FakeHidDevice ledHid; // not opened
        RoccatVulcanDriver driver(controlHid, ledHid);
        QVERIFY(!driver.pushCanvasColors("RoccatVulcan", {{1, 2, 3}}));
        QCOMPARE(static_cast<int>(ledHid.writtenReports.size()), 0);
    }
    {
        FakeHidDevice controlHid; // not opened
        FakeHidDevice ledHid; // not opened
        RoccatVulcanDriver driver(controlHid, ledHid);
        QVERIFY(!driver.pushCanvasColors("RoccatVulcan", {{1, 2, 3}}));
    }
}

void RoccatVulcanDriverTest::testPushCanvasColors_wrongChannelNameFailsGracefully()
{
    FakeHidDevice controlHid; controlHid.open(0x1E7D, 0x307A);
    FakeHidDevice ledHid; ledHid.open(0x1E7D, 0x307A);
    RoccatVulcanDriver driver(controlHid, ledHid);
    QVERIFY(!driver.pushCanvasColors("NoSuchChannel", {{1, 2, 3}}));
    QCOMPARE(static_cast<int>(ledHid.writtenReports.size()), 0);
}

// VULCAN_MAX_LEDS is 144; 145 keys must be rejected gracefully (never
// throw - BuildVulcanColorPackets itself throws past this limit).
void RoccatVulcanDriverTest::testPushCanvasColors_rejectsMoreLedsThanMax()
{
    FakeHidDevice controlHid; controlHid.open(0x1E7D, 0x307A);
    FakeHidDevice ledHid; ledHid.open(0x1E7D, 0x307A);
    RoccatVulcanDriver driver(controlHid, ledHid);
    std::vector<MsiColor> tooMany(VULCAN_MAX_LEDS + 1, MsiColor{1, 2, 3});
    QVERIFY(!driver.pushCanvasColors("RoccatVulcan", tooMany));
    QCOMPARE(static_cast<int>(ledHid.writtenReports.size()), 0);
}

void RoccatVulcanDriverTest::testCreateDefaultChannel_hasDocumentedNameAndLedCount()
{
    Channel channel = RoccatVulcanDriver::createDefaultChannel();
    QCOMPARE(channel.name(), std::string("RoccatVulcan"));
    QCOMPARE(channel.ledLimit(), VULCAN_MAX_LEDS);
    QCOMPARE(channel.totalLedCount(), VULCAN_MAX_LEDS);
    // Reference fallback from OpenRGB, not physical proof for every unit.
    QVERIFY(channel.ledPosition(0x09) == std::make_pair(1, 4)); // < >
    QVERIFY(channel.ledPosition(0x0A) == std::make_pair(1, 5)); // Win
    QVERIFY(channel.ledPosition(0x59) == std::make_pair(14, 5)); // Right Ctrl
    QVERIFY(channel.ledPosition(0x83) == std::make_pair(23, 4)); // Numpad Enter
    int active=0;
    for (int i = 0; i < VULCAN_MAX_LEDS; ++i) {
        const auto pos = channel.ledPosition(i);
        if(channel.components()[0].ledNames[i].find("Wire slot")==0){QVERIFY(!pos);continue;}
        QVERIFY(pos && pos->first >= 0 && pos->first < 24);
        QVERIFY(pos->second >= 0 && pos->second < 6);++active;
    }
    QCOMPARE(active,105);
    std::vector<MsiColor> colors(VULCAN_MAX_LEDS);
    colors[0x09] = {11, 22, 33};
    const auto packets = BuildVulcanColorPackets(colors);
    QCOMPARE(packets[0][14], uint8_t(11));
    QCOMPARE(packets[0][26], uint8_t(22));
    QCOMPARE(packets[0][38], uint8_t(33));

}

// Task 23: supportsMode() is a non-mutating query (no HID I/O, works even
// with both handles closed) that LightingStudioWindow uses to decide which
// modes to offer in this channel's mode combo -- must agree with
// setChannelMode's own accept/reject rule above.
void RoccatVulcanDriverTest::testSupportsMode_canvasOnlyNeverNative()
{
    FakeHidDevice controlHid; // deliberately not opened
    FakeHidDevice ledHid;     // deliberately not opened
    RoccatVulcanDriver driver(controlHid, ledHid);

    QVERIFY(driver.supportsMode("RoccatVulcan", ChannelMode::Canvas));
    QVERIFY(!driver.supportsMode("RoccatVulcan", ChannelMode::Native));
    QVERIFY(!driver.supportsMode("NoSuchChannel", ChannelMode::Canvas));
    QCOMPARE(static_cast<int>(controlHid.sentReports.size()), 0); // purely a query, nothing sent
}

QTEST_MAIN(RoccatVulcanDriverTest)
#include "test_roccat_vulcan_driver.moc"
