// Driver for the Razer Basilisk V3 mouse. Direct-mode only, per the
// project's Global Constraint for this peripheral: Native mode is never
// supported, so setNativeMode is always a no-op and setChannelMode never
// accepts ChannelMode::Native.
#pragma once

#include <string>
#include <vector>

#include "device/Channel.h"
#include "device/DeviceTypes.h"
#include "device/IDriver.h"
#include "hid/IHidDevice.h"
#include "protocol/RazerBasiliskProtocol.h"

class RazerBasiliskDriver : public IDriver {
public:
    explicit RazerBasiliskDriver(IHidDevice& hid);

    // One Channel named "RazerBasiliskV3" with RAZER_BASILISK_MAX_LEDS LED
    // slots, one Component approximating the mouse's LED layout (Logo,
    // Scroll Wheel, 9-LED LED Strip - RazerDevices.cpp:4806-4828) as a
    // generic placeholder.
    // ponytail: real per-LED mouse coordinates would make the canvas
    // preview look more accurate; replace Component::makeGenericPlaceholder
    // with the actual layout if/when that matters.
    static Channel createDefaultChannel();

    // True only for ChannelMode::Canvas on "RazerBasiliskV3"; Native always
    // returns false (Global Constraint: this peripheral is direct-mode
    // only, it has no native lighting mode of its own).
    bool setChannelMode(const std::string& channelName, ChannelMode mode) override;
    ChannelMode channelMode(const std::string& channelName) const override;

    // Always returns false, no-op: this peripheral has no native mode.
    bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) override;

    // Sends the colors report then the mode-custom activation report via
    // hid.sendFeatureReport(), matching the order RazerController::SetLEDs
    // uses: the custom frame is written first, then razer_set_mode_custom()
    // is called once at the end to display it (RazerController.cpp:303-354,
    // activation call at line 348).
    // OpenRGB sends the mode-custom report once per SetLEDs() call, i.e.
    // once per full-frame push - not once at device init like the Roccat
    // Kone's init report. There is no separate "activate direct mode"
    // lifecycle hook in this app either, so resending it on every push here
    // is the natural match, not a deviation. Whether the real mouse
    // tolerates/needs this exact resend cadence is unverified - confirm
    // against real hardware in Task 18.
    // Returns false without sending if !hid.isOpen(), channelName isn't
    // "RazerBasiliskV3", or colors.size() exceeds RAZER_BASILISK_MAX_LEDS.
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) override;

private:
    IHidDevice& hid_;
};
