#include "device/SteelSeriesApex3Driver.h"

namespace {
const char* kChannelName = "SteelSeriesApex3";

// The brief (task-9-brief.md) pins the brightness *packet layout* but not
// which value the driver should send; "3" is only the example argument in
// its protocol-layout test. IDriver exposes no brightness control, so this
// hardcodes a value for every push.
// ponytail: hardcoded brightness, unverified against real hardware's actual
// scale (could be 0-3, could be 0-100 - unknown without a device or the
// cited OpenRGB source). Expose a real brightness setting, or confirm and
// pin the correct max value, once hardware is available to test against.
constexpr uint8_t kBrightness = 3;
} // namespace

SteelSeriesApex3Driver::SteelSeriesApex3Driver(IHidDevice& hid) : hid_(hid)
{
}

Channel SteelSeriesApex3Driver::createDefaultChannel()
{
    Channel channel(kChannelName, STEELSERIES_APEX3_MAX_ZONES);
    channel.addComponent(Component::makeGenericPlaceholder(STEELSERIES_APEX3_MAX_ZONES));
    return channel;
}

bool SteelSeriesApex3Driver::setChannelMode(const std::string& channelName, ChannelMode mode)
{
    // Direct-mode only (Global Constraint): Canvas is the only mode this
    // peripheral ever accepts, so Native is always rejected here.
    if (channelName != kChannelName || mode != ChannelMode::Canvas) {
        return false;
    }
    return true;
}

ChannelMode SteelSeriesApex3Driver::channelMode(const std::string& channelName) const
{
    // Canvas is this peripheral's only supported mode; an unknown channel
    // name falls back to Native, mirroring MsiMotherboardDriver's
    // unknown-channel default.
    return channelName == kChannelName ? ChannelMode::Canvas : ChannelMode::Native;
}

bool SteelSeriesApex3Driver::setNativeMode(const std::string&, const NativeModeSettings&)
{
    // Global Constraint: this peripheral is direct-mode only, it has no
    // native lighting mode of its own, so this is always a no-op.
    return false;
}

bool SteelSeriesApex3Driver::pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors)
{
    if (!hid_.isOpen() || channelName != kChannelName
        || colors.size() > static_cast<size_t>(STEELSERIES_APEX3_MAX_ZONES)) {
        return false;
    }

    // Order pinned to SteelSeriesApexTZoneController::SetColor: the
    // brightness packet (cmd 0x0A) is sent before the colors packet (cmd
    // 0x0B). Both are output reports (hid_write), not feature reports.
    const std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> brightnessPacket = BuildApex3BrightnessPacket(kBrightness);
    const std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> colorsPacket = BuildApex3ColorsPacket(colors);
    const bool brightnessSent = hid_.write(brightnessPacket.data(), brightnessPacket.size());
    const bool colorsSent = hid_.write(colorsPacket.data(), colorsPacket.size());
    return brightnessSent && colorsSent;
}
