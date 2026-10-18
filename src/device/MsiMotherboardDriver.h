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
#include "hid/IHidDevice.h"
#include "protocol/MsiProtocol.h"

enum class ChannelMode { Canvas, Native };

// Mirrors MsiZoneSettings; kept separate so callers of the driver don't
// need to know the Option1/Option2 bit-packing, only plain fields.
struct NativeModeSettings {
    MsiLightingMode lightingMode = MsiLightingMode::Steady;
    MsiColor color1{}, color2{}, color3{}, color4{};
    uint8_t colorCount = 1;
    bool selectAll = false;
    bool directionInward = false;
    bool colorSelectionUser = true;
    uint8_t brightness = 5;
    uint8_t speed = 0;
    uint8_t cycleNum = 30;
};

class MsiMotherboardDriver {
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
    bool setChannelMode(const std::string& channelName, ChannelMode mode);
    ChannelMode channelMode(const std::string& channelName) const;

    // Native mode only. Updates this channel's record in the internal
    // 18-zone state (so other zones' last-sent settings are preserved)
    // and sends the full report 0x50. Returns false without sending if
    // !hid.isOpen(), channelName is unknown, or channelMode(channelName)
    // != Native.
    bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings);

    // Canvas mode only. Sends report 0x51 for this channel's Gen1Adv area.
    // Returns false without sending if !hid.isOpen(), channelName is
    // unknown, channelMode(channelName) != Canvas, or colors.size()
    // exceeds this channel's LED limit (the last condition is a deliberate
    // addition beyond the brief: BuildReport0x51 throws past 240 colors,
    // and this driver must fail gracefully, never throw, per the Review
    // Focus item on a disconnected/misused board).
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors);

private:
    IHidDevice& hid_;
    std::array<MsiZoneSettings, 18> zoneState_{}; // last-sent 0x50 state for ALL zones
    std::map<std::string, ChannelMode> channelModes_;
};
