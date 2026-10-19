// Unit tests for the Razer Basilisk V3 protocol encoders. Ported from
// OpenRGB's RazerController
// (Controllers/RazerController/RazerController/RazerController.cpp).
// Pure data encoding, no HID/device access.
#include <QtTest/QtTest>

#include "protocol/RazerBasiliskProtocol.h"

class RazerBasiliskProtocolTest : public QObject
{
    Q_OBJECT

private slots:
    void testBuildBasiliskColorsReport_layout();
    void testBuildBasiliskColorsReport_tooManyLedsThrows();
    void testBuildBasiliskModeCustomReport_layout();
};

// Pinned to razer_create_custom_frame_extended_matrix_report
// (RazerController.cpp:886-903) as called from SetLEDs with row_index=0,
// start_col=0, stop_col=10 (RAZER_BASILISK_MAX_LEDS-1), and to
// razer_calculate_crc (.cpp:695-716, XOR of bytes 3..88). Two LEDs with
// distinct colors, rest zero-padded (9 of the 11 LEDs), as the task
// brief's representative example.
//
// CRC hand-computed (not via the production XOR loop, so the test
// actually checks something): bytes[3..88] are all zero except
// data_size=0x26, command_class=0x0F, command_id=0x03, stop_col=0x0A, and
// the 6 non-zero RGB bytes (0x11,0x22,0x33,0x44,0x55,0x66).
// 0x26^0x0F^0x03^0x0A = 0x20; 0x11^0x22^0x33^0x44^0x55^0x66 = 0x77;
// 0x20^0x77 = 0x57.
void RazerBasiliskProtocolTest::testBuildBasiliskColorsReport_layout()
{
    std::vector<MsiColor> colors(11, MsiColor{0, 0, 0});
    colors[0] = {0x11, 0x22, 0x33};
    colors[1] = {0x44, 0x55, 0x66};
    auto report = BuildBasiliskColorsReport(colors);
    QCOMPARE(static_cast<int>(report.size()), static_cast<int>(RAZER_BASILISK_REPORT_SIZE));

    std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> expected{};
    expected[2] = 0x1F;  // transaction_id
    expected[6] = 0x26;  // data_size = 33 + 5
    expected[7] = 0x0F;  // command_class
    expected[8] = 0x03;  // command_id
    // arguments[2]=row_index=0 -> expected[11], arguments[3]=start_col=0 -> expected[12]: stay 0
    expected[13] = 0x0A; // arguments[4] = stop_col = 10
    // LED 0 RGB at arguments[5..7] -> expected[14..16]
    expected[14] = 0x11; expected[15] = 0x22; expected[16] = 0x33;
    // LED 1 RGB at arguments[8..10] -> expected[17..19]
    expected[17] = 0x44; expected[18] = 0x55; expected[19] = 0x66;
    // LEDs 2..10 stay zero (expected[20..46])
    expected[89] = 0x57; // hand-computed crc, see comment above
    // expected[90] = 0 (reserved)

    for (size_t i = 0; i < expected.size(); ++i) {
        QCOMPARE(report[i], expected[i]);
    }
}

// RAZER_BASILISK_MAX_LEDS is 11 (cols, RazerDevices.cpp:4837); 12 LEDs must
// throw, matching BuildKoneColorsReport's established style.
void RazerBasiliskProtocolTest::testBuildBasiliskColorsReport_tooManyLedsThrows()
{
    std::vector<MsiColor> colors(12, MsiColor{1, 2, 3});
    QVERIFY_EXCEPTION_THROWN(BuildBasiliskColorsReport(colors), std::invalid_argument);
}

// Pinned to razer_create_mode_custom_extended_matrix_report
// (RazerController.cpp:1024-1033): constant report, no inputs.
// CRC hand-computed: bytes[3..88] are all zero except data_size=0x0C,
// command_class=0x0F, command_id=0x02, arguments[2]=0x08.
// 0x0C^0x0F^0x02^0x08 = 0x09.
void RazerBasiliskProtocolTest::testBuildBasiliskModeCustomReport_layout()
{
    auto report = BuildBasiliskModeCustomReport();
    QCOMPARE(static_cast<int>(report.size()), static_cast<int>(RAZER_BASILISK_REPORT_SIZE));

    std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> expected{};
    expected[2] = 0x1F;  // transaction_id
    expected[6] = 0x0C;  // data_size
    expected[7] = 0x0F;  // command_class
    expected[8] = 0x02;  // command_id
    expected[11] = 0x08; // arguments[2]
    expected[89] = 0x09; // hand-computed crc, see comment above

    for (size_t i = 0; i < expected.size(); ++i) {
        QCOMPARE(report[i], expected[i]);
    }
}

QTEST_MAIN(RazerBasiliskProtocolTest)
#include "test_razer_basilisk_protocol.moc"
