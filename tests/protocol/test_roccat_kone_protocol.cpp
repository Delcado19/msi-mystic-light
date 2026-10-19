// Unit tests for the Roccat Kone Aimo protocol encoders. Ported from
// OpenRGB's RoccatKoneAimoController
// (Controllers/RoccatController/RoccatKoneAimoController/RoccatKoneAimoController.cpp).
// Pure data encoding, no HID/device access.
#include <QtTest/QtTest>

#include "protocol/RoccatKoneProtocol.h"

class RoccatKoneProtocolTest : public QObject
{
    Q_OBJECT

private slots:
    void testBuildKoneInitReport_layout();
    void testBuildKoneColorsReport_layout();
    void testBuildKoneColorsReport_tooManyLedsThrows();
};

// Pinned to RoccatKoneAimoController::SendInit
// (RoccatKoneAimoController.cpp:72-77): constant 6-byte buffer.
void RoccatKoneProtocolTest::testBuildKoneInitReport_layout()
{
    auto report = BuildKoneInitReport();
    QCOMPARE(static_cast<int>(report.size()), static_cast<int>(ROCCAT_KONE_INIT_REPORT_SIZE));
    std::array<uint8_t, 6> expected = {0x0E, 0x06, 0x01, 0x01, 0x00, 0xFF};
    for (size_t i = 0; i < expected.size(); ++i) {
        QCOMPARE(report[i], expected[i]);
    }
}

// Pinned to the RoccatKoneAimoController constructor (.cpp:26-27, header
// bytes 0x0D/0x2E) and SetChannelColors (.cpp:94-98, usb_idx = 0x02 +
// ledIndex*4, R/G/B at sub-offsets 0/1/2). Two LEDs with distinct colors,
// as the task brief's representative example.
void RoccatKoneProtocolTest::testBuildKoneColorsReport_layout()
{
    std::vector<MsiColor> colors = {{0x11, 0x22, 0x33}, {0x44, 0x55, 0x66}};
    auto report = BuildKoneColorsReport(colors);
    QCOMPARE(static_cast<int>(report.size()), static_cast<int>(ROCCAT_KONE_COLOR_REPORT_SIZE));

    std::array<uint8_t, ROCCAT_KONE_COLOR_REPORT_SIZE> expected{};
    expected[0] = 0x0D;
    expected[1] = 0x2E;
    // LED 0: usb_idx = 0x02 + 0*4 = 2
    expected[2] = 0x11; expected[3] = 0x22; expected[4] = 0x33; // byte 5 stays 0 padding
    // LED 1: usb_idx = 0x02 + 1*4 = 6
    expected[6] = 0x44; expected[7] = 0x55; expected[8] = 0x66; // byte 9 stays 0 padding

    for (size_t i = 0; i < expected.size(); ++i) {
        QCOMPARE(report[i], expected[i]);
    }
}

// ROCCAT_KONE_MAX_LEDS is 11 (NUM_LEDS, RoccatKoneAimoController.h:22); 12
// LEDs must throw, matching BuildApex3ColorsPacket's established style.
void RoccatKoneProtocolTest::testBuildKoneColorsReport_tooManyLedsThrows()
{
    std::vector<MsiColor> colors(12, MsiColor{1, 2, 3});
    QVERIFY_EXCEPTION_THROWN(BuildKoneColorsReport(colors), std::invalid_argument);
}

QTEST_MAIN(RoccatKoneProtocolTest)
#include "test_roccat_kone_protocol.moc"
