// Driver for the SteelSeries Apex 3 keyboard (T-Zone protocol). Direct-mode
// only, per the project's Global Constraint for this peripheral: Native
// mode is never supported, so setNativeMode is always a no-op and
// setChannelMode never accepts ChannelMode::Native.
#pragma once

#include <string>
#include <vector>

#include "device/Channel.h"
#include "device/DeviceTypes.h"
#include "device/IDriver.h"
#include "hid/IHidDevice.h"
#include "protocol/SteelSeriesApex3Protocol.h"

class SteelSeriesApex3Driver : public IDriver {
public:
    explicit SteelSeriesApex3Driver(IHidDevice& hid);

    // One Channel named "SteelSeriesApex3" with STEELSERIES_APEX3_MAX_ZONES
    // zone slots, one Component approximating the T-Zone layout as a
    // left-to-right row.
    // ponytail: real per-zone keyboard coordinates would make the canvas
    // preview look more accurate; replace Component::makeGenericPlaceholder
    // with the actual T-Zone layout if/when that matters.
    static Channel createDefaultChannel();

    // True only for ChannelMode::Canvas on "SteelSeriesApex3"; Native always
    // returns false (Global Constraint: this peripheral is direct-mode
    // only, it has no native lighting mode of its own).
    bool setChannelMode(const std::string& channelName, ChannelMode mode) override;
    ChannelMode channelMode(const std::string& channelName) const override;

    // Capability query (Task 23): true only for ChannelMode::Canvas on
    // "SteelSeriesApex3" -- the same rule setChannelMode checks before
    // sending anything, exposed without the HID/isOpen side effects.
    bool supportsMode(const std::string& channelName, ChannelMode mode) const override;

    // Always returns false, no-op: this peripheral has no native mode.
    bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) override;

    // Sends the brightness packet (cmd 0x0A) then the colors packet (cmd
    // 0x0B) via hid.write(), matching the order in OpenRGB's
    // SteelSeriesApexTZoneController::SetColor. Returns false without
    // sending if !hid.isOpen(), channelName isn't "SteelSeriesApex3", or
    // colors.size() exceeds STEELSERIES_APEX3_MAX_ZONES.
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) override;

private:
    IHidDevice& hid_;
};
