#include "protocol/RoccatVulcanProtocol.h"

#include <stdexcept>

// Byte layout pinned to RoccatVulcanKeyboardController::SendColors
// (RoccatVulcanKeyboardController.cpp:197-398), default-case constants
// (packet_length=436, column_length=12, protocol_version=1 - .cpp:243-246)
// which both ROCCAT_VULCAN_100_AIMO_PID and ROCCAT_VULCAN_120_AIMO_PID hit.
// See the header's top-of-file comment for the full byte-by-byte
// derivation, including the one firmware-version-dependent byte order
// this cannot resolve without a runtime handshake (packet 0 bytes 3-4).
std::vector<std::array<uint8_t, VULCAN_PACKET_SIZE>> BuildVulcanColorPackets(
    const std::vector<MsiColor>& keyColors)
{
    if (keyColors.size() > static_cast<size_t>(VULCAN_MAX_LEDS)) {
        throw std::invalid_argument("BuildVulcanColorPackets: keyColors.size() exceeds VULCAN_MAX_LEDS");
    }

    std::vector<std::array<uint8_t, VULCAN_PACKET_SIZE>> packets(VULCAN_PACKET_COUNT);
    for (auto& packet : packets) {
        packet.fill(0x00);
    }

    // Packet 0 header only (protocol_version==1 branch only ever writes
    // bufs[0][1]/[2]/[3]/[4] - .cpp:266-269, 274-289). packets[1..6] stay
    // headerless, pure color data.
    packets[0][1] = 0xA1;
    packets[0][2] = 0x01;
    // header_length_first==4 branch (.cpp:278-289); little-endian default,
    // see header comment for the firmware-version caveat.
    packets[0][3] = VULCAN_PACKET_LENGTH & 0xFF;
    packets[0][4] = (VULCAN_PACKET_LENGTH >> 8) & 0xFF;

    for (size_t i = 0; i < keyColors.size(); ++i) {
        const int column = static_cast<int>(i) / VULCAN_COLUMN_LENGTH;
        const int row = static_cast<int>(i) % VULCAN_COLUMN_LENGTH;
        // Global linear offset across the whole conceptual packet_length
        // stream; +VULCAN_HEADER_LENGTH_FIRST shifts every key past packet
        // 0's 4-byte header (.cpp:366: `column * 3 * column_length + row +
        // header_length_first`).
        int offset = column * 3 * VULCAN_COLUMN_LENGTH + row + VULCAN_HEADER_LENGTH_FIRST;

        packets[offset / 64][offset % 64 + 1] = keyColors[i].r;
        offset += VULCAN_COLUMN_LENGTH;
        packets[offset / 64][offset % 64 + 1] = keyColors[i].g;
        offset += VULCAN_COLUMN_LENGTH;
        packets[offset / 64][offset % 64 + 1] = keyColors[i].b;
    }

    return packets;
}
