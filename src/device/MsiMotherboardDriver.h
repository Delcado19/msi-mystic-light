// MSI motherboard driver (MPG B850 Edge TI WiFi / MS-7E62): the integration
// point between the HID transport (IHidDevice), the MSI wire-protocol
// encoders (MsiZoneSettings/BuildReport0x50, MsiGen1AdvArea/BuildReport0x51)
// and per-channel LED-chain bookkeeping (Channel). Owns the fixed zone-index
// and Gen1Adv-area mapping for this exact board's channels (the lookup table
// lives in MsiMotherboardDriver.cpp); see docs/MB_800_HID_PROTOCOL.md for the
// pinned topology this mapping is taken from.
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "device/Channel.h"
#include "device/DeviceTypes.h"
#include "device/IDriver.h"
#include "hid/IHidDevice.h"
#include "protocol/MsiProtocol.h"

class MsiMotherboardDriver : public IDriver {
public:
    explicit MsiMotherboardDriver(IHidDevice& hid);

    // Fixed channel topology for the MPG B850 Edge TI WiFi (MS-7E62), per
    // docs/MB_800_HID_PROTOCOL.md: "JARGB1"/"JARGB2"/"JARGB3" (<=120 LEDs
    // each, zone indices 0/1/2, Gen1Adv areas JARGB1/JARGB2/JARGB3),
    // "EZConnector" (<=120 LEDs, zone index 3, Gen1Adv area JAF),
    // "IOShield" (11 LEDs, zone index 4, Gen1Adv area JPIPE1),
    // "JRGB1" (1 LED, zone index 9, native-mode only - not addressable
    // per-LED, so Canvas mode is unsupported for this one channel and
    // setChannelMode("JRGB1", Canvas) returns false).
    static std::vector<Channel> createDefaultChannels();

    // Returns false (no-op) for an unknown channel name, or for a channel
    // that has no Gen1Adv area (currently only "JRGB1") when mode is
    // Canvas: that channel isn't per-LED addressable.
    //
    // Entering Canvas mode also sends one 0x50 report arming that zone's
    // record to lightingMode=Advanced: live hardware testing (2026-10-20)
    // found the firmware only displays 0x51 per-LED data for a zone whose
    // 0x50 record is in Advanced mode (see MsiMotherboardDriver.cpp for the
    // full finding). This send is resent every call, same as
    // RoccatVulcanDriver's EnableDirect.
    bool setChannelMode(const std::string& channelName, ChannelMode mode) override;
    ChannelMode channelMode(const std::string& channelName) const override;

    // Capability query (Task 23): false for an unknown channel; for Canvas,
    // true only if the channel has a Gen1Adv area (every channel but
    // JRGB1); Native is always true for a known channel -- the exact same
    // rule setChannelMode's own early-return already encodes, read without
    // mutating anything.
    bool supportsMode(const std::string& channelName, ChannelMode mode) const override;

    // Native mode only. Updates this channel's record in the internal
    // 18-zone state (so other zones' last-sent settings are preserved)
    // and sends the full report 0x50. Returns false without sending if
    // !hid.isOpen(), channelName is unknown, or channelMode(channelName)
    // != Native.
    bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) override;

    // Canvas mode only. Sends report 0x51 for this channel's Gen1Adv area.
    // Returns false without sending if !hid.isOpen(), channelName is
    // unknown, channelMode(channelName) != Canvas, or colors.size()
    // exceeds this channel's LED limit (the last condition is a deliberate
    // addition beyond the brief: BuildReport0x51 throws past 240 colors,
    // and this driver must fail gracefully, never throw, per the Review
    // Focus item on a disconnected/misused board).
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) override;

private:
    IHidDevice& hid_;
    std::array<MsiZoneSettings, 18> zoneState_{}; // last-sent 0x50 state for ALL zones
    std::map<std::string, ChannelMode> channelModes_;
};
