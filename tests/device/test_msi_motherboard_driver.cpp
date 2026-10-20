// Unit tests for MsiMotherboardDriver (Task 6): the integration point
// between IHidDevice, the MSI report 0x50/0x51 encoders, and the per-channel
// zone-index/Gen1Adv-area mapping fixed for the MPG B850 Edge TI WiFi
// (MS-7E62). Uses FakeHidDevice (tests/hid/FakeHidDevice.h) - no real
// hardware involved.
#include <QtTest/QtTest>

#include <map>

#include "device/MsiMotherboardDriver.h"
#include "hid/FakeHidDevice.h"

class MsiMotherboardDriverTest : public QObject
{
    Q_OBJECT

private slots:
    void testSetNativeMode_sendsExpectedBytesAndPreservesOtherZones();
    void testSetNativeMode_deviceNotOpenFailsGracefully();
    void testPushCanvasColors_onlyWorksInCanvasMode();
    void testJRGB1_cannotBeSetToCanvasMode();
    void testPushCanvasColors_deviceNotOpenFailsGracefully();
    void testZoneIndexMapping_pinsEachChannelToItsDocumentedZone();
    void testGen1AdvAreaMapping_pinsEachChannelToItsDocumentedArea();
    void testPushCanvasColors_rejectsMoreColorsThanLedLimit();
    void testSetNativeMode_unknownChannelFailsGracefully();
    void testSetNativeMode_wrongModeFailsGracefully();
    void testSetNativeMode_jargb3UsesZoneTwoAndAreaFour02();
    void testCreateDefaultChannels_returnsAllSixChannelsWithDocumentedLedLimits();
    void testCreateDefaultChannels_channelsHavePlaceholderComponentsSoColorsAreNotEmpty();
    void testSetChannelModeCanvas_armsZoneToAdvancedBefore0x51Push();
    void testSetChannelModeNative_doesNotArmZone();
    void testJRGB1_cannotBeSetToCanvasMode_sendsNothing();
};

void MsiMotherboardDriverTest::testSetNativeMode_sendsExpectedBytesAndPreservesOtherZones()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto channels = MsiMotherboardDriver::createDefaultChannels();

    NativeModeSettings s1{MsiLightingMode::Wave, {255,0,0},{0,255,0},{0,0,255},{255,255,255}, 4, false,false,false,5,0,30};
    QVERIFY(driver.setNativeMode("JARGB1", s1));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    QCOMPARE(hid.sentReports[0][0], static_cast<uint8_t>(0x50));
    QCOMPARE(hid.sentReports[0][1], static_cast<uint8_t>(1)); // record 0 = JARGB1, lightingMode Wave

    NativeModeSettings s2{MsiLightingMode::Steady, {10,20,30},{},{},{}, 1, false,false,true,3,1,15};
    QVERIFY(driver.setNativeMode("JARGB2", s2));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 2);
    // JARGB1's record (bytes 1-16) must still reflect s1, unchanged by the JARGB2 update.
    QCOMPARE(hid.sentReports[1][1], static_cast<uint8_t>(1));
    QCOMPARE(hid.sentReports[1][2], static_cast<uint8_t>(255));
    // The above only proves zone 0 is untouched; it does NOT prove JARGB2
    // landed in zone 1 (bytes 17-32) rather than nowhere/elsewhere. Pin that
    // directly: zone 1's own record must reflect s2's values.
    QCOMPARE(hid.sentReports[1][17], static_cast<uint8_t>(2));  // record 1 lightingMode = Steady (2)
    QCOMPARE(hid.sentReports[1][18], static_cast<uint8_t>(10)); // record 1 color1.r
    QCOMPARE(hid.sentReports[1][19], static_cast<uint8_t>(20)); // record 1 color1.g
    QCOMPARE(hid.sentReports[1][20], static_cast<uint8_t>(30)); // record 1 color1.b
}

void MsiMotherboardDriverTest::testSetNativeMode_deviceNotOpenFailsGracefully()
{
    FakeHidDevice hid; // never opened
    MsiMotherboardDriver driver(hid);
    QVERIFY(!driver.setNativeMode("JARGB1", NativeModeSettings{}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

void MsiMotherboardDriverTest::testPushCanvasColors_onlyWorksInCanvasMode()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    MsiMotherboardDriver::createDefaultChannels();

    std::vector<MsiColor> colors = {{1,2,3},{4,5,6}};
    QVERIFY(!driver.pushCanvasColors("JARGB1", colors)); // default mode is Native
    QVERIFY(driver.setChannelMode("JARGB1", ChannelMode::Canvas));
    QVERIFY(driver.pushCanvasColors("JARGB1", colors));
    // setChannelMode(Canvas) also sends one arming 0x50 report (see
    // testSetChannelModeCanvas_armsZoneToAdvancedBefore0x51Push below for
    // the dedicated test), so two reports are sent in total: 0x50 then 0x51.
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 2);
    QCOMPARE(hid.sentReports[0][0], static_cast<uint8_t>(0x50));
    QCOMPARE(hid.sentReports[1][0], static_cast<uint8_t>(0x51));
}

void MsiMotherboardDriverTest::testJRGB1_cannotBeSetToCanvasMode()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    QVERIFY(!driver.setChannelMode("JRGB1", ChannelMode::Canvas));
    QCOMPARE(driver.channelMode("JRGB1"), ChannelMode::Native);
}

// Review Focus: "MSI board not connected/unplugged - driver calls must
// return failure gracefully, never throw or crash." The brief's given tests
// only cover this for setNativeMode; pushCanvasColors needs the same
// !hid.isOpen() guard and the same coverage.
void MsiMotherboardDriverTest::testPushCanvasColors_deviceNotOpenFailsGracefully()
{
    FakeHidDevice hid; // never opened
    MsiMotherboardDriver driver(hid);
    QVERIFY(driver.setChannelMode("JARGB1", ChannelMode::Canvas));
    QVERIFY(!driver.pushCanvasColors("JARGB1", {{1,2,3}}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

// Pins the name -> zone-index mapping from the brief's note ("JARGB_V2_1=0,
// JARGB_V2_2=1, JARGB_V2_3=2, EZ Conn=3, IO COVER=4, JRGB1=9"), the single
// riskiest fact in this driver. Without this, a swapped EZConnector/IOShield
// zone index would pass every other test in this file (they only touch
// JARGB1/JARGB2). report[1 + zoneIndex*16] is each zone record's
// lightingMode byte (see BuildReport0x50/MsiProtocol.cpp).
void MsiMotherboardDriverTest::testZoneIndexMapping_pinsEachChannelToItsDocumentedZone()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    NativeModeSettings wave; wave.lightingMode = MsiLightingMode::Wave;

    QVERIFY(driver.setNativeMode("EZConnector", wave));
    QCOMPARE(hid.sentReports.back()[1 + 3 * 16], static_cast<uint8_t>(1)); // zone 3
    QCOMPARE(hid.sentReports.back()[1 + 4 * 16], static_cast<uint8_t>(0)); // zone 4 untouched

    QVERIFY(driver.setNativeMode("IOShield", wave));
    QCOMPARE(hid.sentReports.back()[1 + 4 * 16], static_cast<uint8_t>(1)); // zone 4

    QVERIFY(driver.setNativeMode("JRGB1", wave));
    QCOMPARE(hid.sentReports.back()[1 + 9 * 16], static_cast<uint8_t>(1)); // zone 9
}

// Pins the name -> Gen1Adv-area mapping from the brief's note: EZConnector
// uses area JAF (0x0800), IOShield uses JPIPE1 (0x0300) - NOT the areas
// named JARGB1/2/3. Report 0x51 bytes 2-3 are the area, big-endian.
void MsiMotherboardDriverTest::testGen1AdvAreaMapping_pinsEachChannelToItsDocumentedArea()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);

    // JARGB1/JARGB2 use the areas of the same name (0x0400/0x0401) - unlike
    // EZConnector/IOShield below, which use a DIFFERENT area than their own
    // name would suggest. Neither was asserted anywhere before this test.
    QVERIFY(driver.setChannelMode("JARGB1", ChannelMode::Canvas));
    QVERIFY(driver.pushCanvasColors("JARGB1", {{1,2,3}}));
    QCOMPARE(hid.sentReports.back()[2], static_cast<uint8_t>(0x04));
    QCOMPARE(hid.sentReports.back()[3], static_cast<uint8_t>(0x00));

    QVERIFY(driver.setChannelMode("JARGB2", ChannelMode::Canvas));
    QVERIFY(driver.pushCanvasColors("JARGB2", {{1,2,3}}));
    QCOMPARE(hid.sentReports.back()[2], static_cast<uint8_t>(0x04));
    QCOMPARE(hid.sentReports.back()[3], static_cast<uint8_t>(0x01));

    QVERIFY(driver.setChannelMode("EZConnector", ChannelMode::Canvas));
    QVERIFY(driver.pushCanvasColors("EZConnector", {{1,2,3}}));
    QCOMPARE(hid.sentReports.back()[2], static_cast<uint8_t>(0x08)); // JAF high byte
    QCOMPARE(hid.sentReports.back()[3], static_cast<uint8_t>(0x00));

    QVERIFY(driver.setChannelMode("IOShield", ChannelMode::Canvas));
    QVERIFY(driver.pushCanvasColors("IOShield", {{1,2,3}}));
    QCOMPARE(hid.sentReports.back()[2], static_cast<uint8_t>(0x03)); // JPIPE1 high byte
    QCOMPARE(hid.sentReports.back()[3], static_cast<uint8_t>(0x00));
}

// Deliberate contract addition beyond the brief (see header comment on
// pushCanvasColors): BuildReport0x51 throws past 240 colors, which would
// violate the "never throw or crash" Review Focus item, so the driver must
// refuse anything over the channel's own LED limit before calling it.
// IOShield's limit is 11; 12 colors must be rejected with nothing sent.
void MsiMotherboardDriverTest::testPushCanvasColors_rejectsMoreColorsThanLedLimit()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    QVERIFY(driver.setChannelMode("IOShield", ChannelMode::Canvas));
    // setChannelMode(Canvas) itself sends one arming 0x50 report; only the
    // rejected pushCanvasColors call below must add nothing further.
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);

    std::vector<MsiColor> tooMany(12, MsiColor{1, 2, 3});
    QVERIFY(!driver.pushCanvasColors("IOShield", tooMany));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
}

void MsiMotherboardDriverTest::testSetNativeMode_unknownChannelFailsGracefully()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    QVERIFY(!driver.setNativeMode("NoSuchChannel", NativeModeSettings{}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

void MsiMotherboardDriverTest::testSetNativeMode_wrongModeFailsGracefully()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    QVERIFY(driver.setChannelMode("JARGB1", ChannelMode::Canvas));
    // setChannelMode(Canvas) itself sends one arming 0x50 report; the
    // rejected setNativeMode call below must add nothing further.
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    QVERIFY(!driver.setNativeMode("JARGB1", NativeModeSettings{}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
}

// Finding from review: JARGB3's zone index (2) and Gen1Adv area (0x0402)
// were asserted by zero tests before this one - every other test only ever
// exercised JARGB1/JARGB2/EZConnector/IOShield/JRGB1.
void MsiMotherboardDriverTest::testSetNativeMode_jargb3UsesZoneTwoAndAreaFour02()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);

    NativeModeSettings s{MsiLightingMode::Wave, {7,8,9},{},{},{}, 1, false,false,true,5,0,30};
    QVERIFY(driver.setNativeMode("JARGB3", s));
    const auto& report = hid.sentReports.back();
    QCOMPARE(report[1 + 2 * 16], static_cast<uint8_t>(1)); // zone 2 lightingMode = Wave (1)
    QCOMPARE(report[2 + 2 * 16], static_cast<uint8_t>(7));  // zone 2 color1.r
    QCOMPARE(report[3 + 2 * 16], static_cast<uint8_t>(8));  // zone 2 color1.g
    QCOMPARE(report[4 + 2 * 16], static_cast<uint8_t>(9));  // zone 2 color1.b

    QVERIFY(driver.setChannelMode("JARGB3", ChannelMode::Canvas));
    QVERIFY(driver.pushCanvasColors("JARGB3", {{1,2,3}}));
    QCOMPARE(hid.sentReports.back()[2], static_cast<uint8_t>(0x04));
    QCOMPARE(hid.sentReports.back()[3], static_cast<uint8_t>(0x02));
}

// Finding from review: createDefaultChannels()'s return value is an
// explicit brief requirement ("Fixed channel topology ... createDefaultChannels")
// but was only ever called and discarded in other tests - zero assertions
// on its result.
void MsiMotherboardDriverTest::testCreateDefaultChannels_returnsAllSixChannelsWithDocumentedLedLimits()
{
    auto channels = MsiMotherboardDriver::createDefaultChannels();
    QCOMPARE(static_cast<int>(channels.size()), 6);

    std::map<std::string, int> ledLimitByName;
    for (const auto& ch : channels) {
        ledLimitByName[ch.name()] = ch.ledLimit();
    }
    QCOMPARE(static_cast<int>(ledLimitByName.size()), 6); // no duplicate/missing names
    QCOMPARE(ledLimitByName.at("JARGB1"), 120);
    QCOMPARE(ledLimitByName.at("JARGB2"), 120);
    QCOMPARE(ledLimitByName.at("JARGB3"), 120);
    QCOMPARE(ledLimitByName.at("EZConnector"), 120);
    QCOMPARE(ledLimitByName.at("IOShield"), 11);
    QCOMPARE(ledLimitByName.at("JRGB1"), 1);
}

// HW verification finding: createDefaultChannels() built each Channel with
// the right ledLimit but never called addComponent() on it, so Channel::colors()
// (only populated for LEDs owned by an added Component) was always empty for
// every motherboard channel - Canvas's report 0x51 silently sent zero LEDs on
// real hardware (offset-6 LED-count byte 0x00 instead of e.g. 0x78/120), with
// no error anywhere. testCreateDefaultChannels_returnsAllSixChannelsWithDocumentedLedLimits
// above only checked ledLimit(), which addComponent() does not affect, so it
// never caught this. This test exercises colors()/totalLedCount() instead -
// the exact values Canvas::tick() depends on - so a regression (removing the
// addComponent calls again) fails here immediately.
void MsiMotherboardDriverTest::testCreateDefaultChannels_channelsHavePlaceholderComponentsSoColorsAreNotEmpty()
{
    auto channels = MsiMotherboardDriver::createDefaultChannels();

    std::map<std::string, int> expectedLedCount = {
        {"JARGB1", 120}, {"JARGB2", 120}, {"JARGB3", 120},
        {"EZConnector", 120}, {"IOShield", 11}, {"JRGB1", 1},
    };
    QCOMPARE(static_cast<int>(channels.size()), static_cast<int>(expectedLedCount.size()));
    for (const auto& ch : channels) {
        const int expected = expectedLedCount.at(ch.name());
        QCOMPARE(ch.totalLedCount(), expected);
        QCOMPARE(static_cast<int>(ch.colors().size()), expected);
    }
}

// Live-hardware finding (2026-10-20, see MsiMotherboardDriver.cpp's
// setChannelMode comment): the firmware only honors 0x51 per-LED data for a
// zone whose 0x50 record has lightingMode=Advanced (9). Pins that
// setChannelMode(Canvas) sends that arming 0x50 report itself, using the
// same record-layout offset (1 + zoneIndex*16) as
// testZoneIndexMapping_pinsEachChannelToItsDocumentedZone above.
void MsiMotherboardDriverTest::testSetChannelModeCanvas_armsZoneToAdvancedBefore0x51Push()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);

    QVERIFY(driver.setChannelMode("JARGB2", ChannelMode::Canvas));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    const auto& report = hid.sentReports.back();
    QCOMPARE(report[0], static_cast<uint8_t>(0x50));
    // JARGB2 is zone index 1: lightingMode byte is at 1 + 1*16 = 17.
    QCOMPARE(report[1 + 1 * 16], static_cast<uint8_t>(9)); // MsiLightingMode::Advanced
}

// Only Canvas arms a zone; Native must never trigger the 0x50 arming send
// (it has its own, separate setNativeMode path).
void MsiMotherboardDriverTest::testSetChannelModeNative_doesNotArmZone()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);

    QVERIFY(driver.setChannelMode("JARGB1", ChannelMode::Native));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

// JRGB1 has no Gen1Adv area, so setChannelMode(Canvas) must keep returning
// false and must not attempt the arming send either (it's gated behind the
// same early return as the Gen1Adv-area check).
void MsiMotherboardDriverTest::testJRGB1_cannotBeSetToCanvasMode_sendsNothing()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);

    QVERIFY(!driver.setChannelMode("JRGB1", ChannelMode::Canvas));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

QTEST_MAIN(MsiMotherboardDriverTest)
#include "test_msi_motherboard_driver.moc"
