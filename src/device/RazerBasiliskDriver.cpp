#include "device/RazerBasiliskDriver.h"

namespace {
const char* kChannelName = "RazerBasiliskV3";
} // namespace

RazerBasiliskDriver::RazerBasiliskDriver(IHidDevice& hid) : hid_(hid)
{
}

Channel RazerBasiliskDriver::createDefaultChannel()
{
    Channel channel(kChannelName, RAZER_BASILISK_MAX_LEDS);
    channel.addComponent(Component::makeGenericPlaceholder(RAZER_BASILISK_MAX_LEDS));
    return channel;
}

bool RazerBasiliskDriver::supportsMode(const std::string& channelName, ChannelMode mode) const
{
    // Direct-mode only (Global Constraint): Canvas is the only mode this
    // peripheral ever accepts, so Native is never supported.
    return channelName == kChannelName && mode == ChannelMode::Canvas;
}

bool RazerBasiliskDriver::setChannelMode(const std::string& channelName, ChannelMode mode)
{
    return supportsMode(channelName, mode);
}

ChannelMode RazerBasiliskDriver::channelMode(const std::string& channelName) const
{
    // Canvas is this peripheral's only supported mode; an unknown channel
    // name falls back to Native, mirroring MsiMotherboardDriver's
    // unknown-channel default.
    return channelName == kChannelName ? ChannelMode::Canvas : ChannelMode::Native;
}

bool RazerBasiliskDriver::setNativeMode(const std::string&, const NativeModeSettings&)
{
    // Global Constraint: this peripheral is direct-mode only, it has no
    // native lighting mode of its own, so this is always a no-op.
    return false;
}

bool RazerBasiliskDriver::pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors)
{
    if (!hid_.isOpen() || channelName != kChannelName
        || colors.size() > static_cast<size_t>(RAZER_BASILISK_MAX_LEDS)) {
        return false;
    }

    // Order pinned to RazerController::SetLEDs (RazerController.cpp:303-354):
    // the custom frame (colors report) is written first, then
    // razer_set_mode_custom() is sent once at the end to display it. Both
    // are feature reports (hid_send_feature_report), not output reports.
    const std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> colorsReport = BuildBasiliskColorsReport(colors);
    const std::array<uint8_t, RAZER_BASILISK_REPORT_SIZE> modeCustomReport = BuildBasiliskModeCustomReport();
    const bool colorsSent = hid_.sendFeatureReport(colorsReport.data(), colorsReport.size());
    const bool modeCustomSent = hid_.sendFeatureReport(modeCustomReport.data(), modeCustomReport.size());
    return colorsSent && modeCustomSent;
}
