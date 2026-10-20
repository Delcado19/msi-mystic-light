#include "device/RoccatKoneDriver.h"

namespace {
const char* kChannelName = "RoccatKone";
} // namespace

RoccatKoneDriver::RoccatKoneDriver(IHidDevice& hid) : hid_(hid)
{
}

Channel RoccatKoneDriver::createDefaultChannel()
{
    Channel channel(kChannelName, ROCCAT_KONE_MAX_LEDS);
    channel.addComponent(Component::makeGenericPlaceholder(ROCCAT_KONE_MAX_LEDS));
    return channel;
}

bool RoccatKoneDriver::supportsMode(const std::string& channelName, ChannelMode mode) const
{
    // Direct-mode only (Global Constraint): Canvas is the only mode this
    // peripheral ever accepts, so Native is never supported.
    return channelName == kChannelName && mode == ChannelMode::Canvas;
}

bool RoccatKoneDriver::setChannelMode(const std::string& channelName, ChannelMode mode)
{
    return supportsMode(channelName, mode);
}

ChannelMode RoccatKoneDriver::channelMode(const std::string& channelName) const
{
    // Canvas is this peripheral's only supported mode; an unknown channel
    // name falls back to Native, mirroring MsiMotherboardDriver's
    // unknown-channel default.
    return channelName == kChannelName ? ChannelMode::Canvas : ChannelMode::Native;
}

bool RoccatKoneDriver::setNativeMode(const std::string&, const NativeModeSettings&)
{
    // Global Constraint: this peripheral is direct-mode only, it has no
    // native lighting mode of its own, so this is always a no-op.
    return false;
}

bool RoccatKoneDriver::pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors)
{
    if (!hid_.isOpen() || channelName != kChannelName
        || colors.size() > static_cast<size_t>(ROCCAT_KONE_MAX_LEDS)) {
        return false;
    }

    // Order pinned to RoccatKoneAimoController: SendInit (init report) runs
    // before any SetChannelColors/SendUpdate (colors report). Both are
    // feature reports (hid_send_feature_report), not output reports.
    const std::array<uint8_t, ROCCAT_KONE_INIT_REPORT_SIZE> initReport = BuildKoneInitReport();
    const std::array<uint8_t, ROCCAT_KONE_COLOR_REPORT_SIZE> colorsReport = BuildKoneColorsReport(colors);
    const bool initSent = hid_.sendFeatureReport(initReport.data(), initReport.size());
    const bool colorsSent = hid_.sendFeatureReport(colorsReport.data(), colorsReport.size());
    return initSent && colorsSent;
}
