#include "device/SteelSeriesApex3Driver.h"

namespace {
const char* kChannelName = "SteelSeriesApex3";

// The brief (task-9-brief.md) pins the brightness *packet layout* but not
// which value the driver should send; "3" was only the example argument in
// its protocol-layout test, mistaken at the time for a plausible real value.
// IDriver exposes no brightness control, so this hardcodes a value (not
// user-configurable) for every push.
// Hardware-verified 2026-10-20 (Task 18, closing the Task 9 ponytail debt
// item in .superpowers/sdd/2026-10-19-lighting-studio/progress.md): the
// T-Zone brightness scale is 0-100, not 0-3. Confirmed both by live test
// against a real Apex 3 (kBrightness=3 was visually ~3% brightness, barely
// perceptible; 100 is full, clearly visible) and by OpenRGB's own source -
// STEELSERIES_TZ_BRIGHTNESS_MAX = 0x64 (100) in
// SteelSeriesApexTZoneController.h:22. The value is now confirmed-correct,
// not a guess; it just still can't be changed by the user.
constexpr uint8_t kSteelSeriesApex3BrightnessMax = 100;
} // namespace

SteelSeriesApex3Driver::SteelSeriesApex3Driver(IHidDevice& hid) : hid_(hid)
{
}

Channel SteelSeriesApex3Driver::createDefaultChannel()
{
    Channel channel(kChannelName, STEELSERIES_APEX3_MAX_ZONES);
    auto component = Component::makeGenericPlaceholder(STEELSERIES_APEX3_MAX_ZONES);
    component.productName = component.displayName = "SteelSeries Apex 3";
    component.type = "KeyboardZones";
    component.width = 10; component.height = 2;
    for (int i = 0; i < component.ledCount; ++i) component.ledNames[i] = "Zone " + std::to_string(i + 1);
    channel.addComponent(component);
    return channel;
}

bool SteelSeriesApex3Driver::supportsMode(const std::string& channelName, ChannelMode mode) const
{
    // Direct-mode only (Global Constraint): Canvas is the only mode this
    // peripheral ever accepts, so Native is never supported.
    return channelName == kChannelName && mode == ChannelMode::Canvas;
}

bool SteelSeriesApex3Driver::setChannelMode(const std::string& channelName, ChannelMode mode)
{
    return supportsMode(channelName, mode);
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
    const std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> brightnessPacket = BuildApex3BrightnessPacket(kSteelSeriesApex3BrightnessMax);
    const std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> colorsPacket = BuildApex3ColorsPacket(colors);
    const bool brightnessSent = hid_.write(brightnessPacket.data(), brightnessPacket.size());
    const bool colorsSent = hid_.write(colorsPacket.data(), colorsPacket.size());
    return brightnessSent && colorsSent;
}
