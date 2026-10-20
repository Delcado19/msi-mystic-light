// Unit tests for the Roccat Vulcan 100/120 Aimo protocol encoder. Ported
// from OpenRGB's RoccatVulcanKeyboardController::SendColors
// (Controllers/RoccatController/RoccatVulcanKeyboardController/
// RoccatVulcanKeyboardController.cpp:197-398, default-case constants -
// see src/protocol/RoccatVulcanProtocol.h for the full derivation).
// Pure data encoding, no HID/device access.
#include <QtTest/QtTest>

#include "protocol/RoccatVulcanProtocol.h"

class RoccatVulcanProtocolTest : public QObject
{
    Q_OBJECT

private slots:
    void testBuildVulcanColorPackets_packetCountAndHeader();
    void testBuildVulcanColorPackets_layout();
    void testBuildVulcanColorPackets_lastKeyLandsInLastPacket();
    void testBuildVulcanColorPackets_tooManyKeysThrows();
    void testBuildVulcanEnableDirectReport_layout();
};

// Always builds exactly VULCAN_PACKET_COUNT (7) 65-byte packets, even for
// an empty input - the real device always receives all 7 packets
// (RoccatVulcanKeyboardController.cpp:249-256 sizes `bufs` from the fixed
// packet_length, not from colors.size()). Packet 0's header is pinned to
// .cpp:266-269 (0xA1/0x01 markers) and .cpp:280-289's big-endian branch of
// IsBigEndianDirectMode() - confirmed by live-hardware test 2026-10-20
// against a real Vulcan 100 Aimo; see header comment for the firmware-
// version caveat that still applies to other physical units:
// VULCAN_PACKET_LENGTH=436=0x01B4 -> high byte 0x01 at [3], low byte 0xB4
// at [4]. Packets 1..6 have no header at all (.cpp's protocol_version==1
// branch only ever writes bufs[0][1]/[2], never bufs[i>0][1]/[2]).
void RoccatVulcanProtocolTest::testBuildVulcanColorPackets_packetCountAndHeader()
{
    auto packets = BuildVulcanColorPackets({});
    QCOMPARE(static_cast<int>(packets.size()), VULCAN_PACKET_COUNT);
    for (const auto& packet : packets) {
        QCOMPARE(static_cast<int>(packet.size()), static_cast<int>(VULCAN_PACKET_SIZE));
    }

    QCOMPARE(packets[0][0], static_cast<uint8_t>(0x00)); // report-ID byte, never written
    QCOMPARE(packets[0][1], static_cast<uint8_t>(0xA1));
    QCOMPARE(packets[0][2], static_cast<uint8_t>(0x01));
    QCOMPARE(packets[0][3], static_cast<uint8_t>(0x01)); // 436 >> 8
    QCOMPARE(packets[0][4], static_cast<uint8_t>(0xB4)); // 436 & 0xFF

    for (int p = 1; p < VULCAN_PACKET_COUNT; ++p) {
        QCOMPARE(packets[p][0], static_cast<uint8_t>(0x00));
        QCOMPARE(packets[p][1], static_cast<uint8_t>(0x00)); // no per-packet header for protocol_version==1
        QCOMPARE(packets[p][2], static_cast<uint8_t>(0x00));
    }
}

// Per-key byte offsets hand-traced from RoccatVulcanKeyboardController.cpp:
// 361-371 (protocol_version==1 branch) for VULCAN_COLUMN_LENGTH=12,
// header_length_first=4. Three keys exercise: key 0 (column 0, row 0 -
// the very first slot after the header), key 11 (column 0, row 11 - last
// row of the first column) and key 12 (column 1, row 0 - whose blue byte
// crosses the packet-0/packet-1 boundary, the packing's trickiest case).
//
// key 0:  column=0,row=0  -> base=0*36+0+4=4   -> R@4,G@16,B@28
//         all < 64 -> packet 0, byte index = offset%64+1
//         R: packets[0][5], G: packets[0][17], B: packets[0][29]
// key 11: column=0,row=11 -> base=0*36+11+4=15 -> R@15,G@27,B@39
//         R: packets[0][16], G: packets[0][28], B: packets[0][40]
// key 12: column=1,row=0  -> base=1*36+0+4=40  -> R@40,G@52,B@64
//         R: packets[0][41], G: packets[0][53]
//         B@64 -> packet=64/64=1, byte=64%64=0 -> packets[1][1]
void RoccatVulcanProtocolTest::testBuildVulcanColorPackets_layout()
{
    std::vector<MsiColor> colors(13, MsiColor{0, 0, 0});
    colors[0] = {0x11, 0x22, 0x33};
    colors[11] = {0x44, 0x55, 0x66};
    colors[12] = {0x77, 0x88, 0x99};

    auto packets = BuildVulcanColorPackets(colors);
    QCOMPARE(static_cast<int>(packets.size()), VULCAN_PACKET_COUNT);

    QCOMPARE(packets[0][5], static_cast<uint8_t>(0x11));
    QCOMPARE(packets[0][17], static_cast<uint8_t>(0x22));
    QCOMPARE(packets[0][29], static_cast<uint8_t>(0x33));

    QCOMPARE(packets[0][16], static_cast<uint8_t>(0x44));
    QCOMPARE(packets[0][28], static_cast<uint8_t>(0x55));
    QCOMPARE(packets[0][40], static_cast<uint8_t>(0x66));

    QCOMPARE(packets[0][41], static_cast<uint8_t>(0x77));
    QCOMPARE(packets[0][53], static_cast<uint8_t>(0x88));
    QCOMPARE(packets[1][1], static_cast<uint8_t>(0x99)); // crosses into packet 1
}

// The very last addressable key (idx=143, the top of VULCAN_MAX_LEDS)
// must land fully inside packet 6 (the last of the VULCAN_PACKET_COUNT=7
// packets), confirming the 144-key cap doesn't overflow BuildVulcanColorPackets'
// fixed-size packet vector. Hand-traced from .cpp:361-371 same as the
// other cases: column=143/12=11, row=143%12=11 ->
// base=11*36+11+4=411 -> R@411,G@423,B@435.
// 411/64=6 rem 27 -> packets[6][28]; 423/64=6 rem 39 -> packets[6][40];
// 435/64=6 rem 51 -> packets[6][52].
void RoccatVulcanProtocolTest::testBuildVulcanColorPackets_lastKeyLandsInLastPacket()
{
    std::vector<MsiColor> colors(VULCAN_MAX_LEDS, MsiColor{0, 0, 0});
    colors[143] = {0xAA, 0xBB, 0xCC};

    auto packets = BuildVulcanColorPackets(colors);
    QCOMPARE(static_cast<int>(packets.size()), VULCAN_PACKET_COUNT);
    QCOMPARE(packets[6][28], static_cast<uint8_t>(0xAA));
    QCOMPARE(packets[6][40], static_cast<uint8_t>(0xBB));
    QCOMPARE(packets[6][52], static_cast<uint8_t>(0xCC));
}

// VULCAN_MAX_LEDS is 144 ((436-4)/3); 145 keys must throw, matching
// BuildBasiliskColorsReport/BuildKoneColorsReport's established style.
void RoccatVulcanProtocolTest::testBuildVulcanColorPackets_tooManyKeysThrows()
{
    std::vector<MsiColor> tooMany(VULCAN_MAX_LEDS + 1, MsiColor{1, 2, 3});
    QVERIFY_EXCEPTION_THROWN(BuildVulcanColorPackets(tooMany), std::invalid_argument);
}

// Pinned to RoccatVulcanKeyboardController::EnableDirect's default case
// (.cpp:185-187): buf = {0x15, 0x00, on_off_switch}.
void RoccatVulcanProtocolTest::testBuildVulcanEnableDirectReport_layout()
{
    auto enableReport = BuildVulcanEnableDirectReport(true);
    QCOMPARE(static_cast<int>(enableReport.size()), static_cast<int>(VULCAN_ENABLE_DIRECT_REPORT_SIZE));
    QCOMPARE(enableReport[0], static_cast<uint8_t>(0x15));
    QCOMPARE(enableReport[1], static_cast<uint8_t>(0x00));
    QCOMPARE(enableReport[2], static_cast<uint8_t>(0x01));

    auto disableReport = BuildVulcanEnableDirectReport(false);
    QCOMPARE(disableReport[0], static_cast<uint8_t>(0x15));
    QCOMPARE(disableReport[1], static_cast<uint8_t>(0x00));
    QCOMPARE(disableReport[2], static_cast<uint8_t>(0x00));
}

QTEST_MAIN(RoccatVulcanProtocolTest)
#include "test_roccat_vulcan_protocol.moc"
