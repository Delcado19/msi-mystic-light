// Unit tests for MsiMotherboardDriver (Task 6): the integration point
// between IHidDevice, the MSI report 0x50/0x51 encoders, and the per-channel
// zone-index/Gen1Adv-area mapping fixed for the MPG B850 Edge TI WiFi
// (MS-7E62). Uses FakeHidDevice (tests/hid/FakeHidDevice.h) - no real
// hardware involved.
#include <QtTest/QtTest>

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
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    QCOMPARE(hid.sentReports[0][0], static_cast<uint8_t>(0x51));
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

    std::vector<MsiColor> tooMany(12, MsiColor{1, 2, 3});
    QVERIFY(!driver.pushCanvasColors("IOShield", tooMany));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
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
    QVERIFY(!driver.setNativeMode("JARGB1", NativeModeSettings{}));
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0);
}

QTEST_MAIN(MsiMotherboardDriverTest)
#include "test_msi_motherboard_driver.moc"
