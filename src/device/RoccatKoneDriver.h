// Driver for the Roccat Kone Aimo mouse. Direct-mode only, per the
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
#include "protocol/RoccatKoneProtocol.h"

class RoccatKoneDriver : public IDriver {
public:
    explicit RoccatKoneDriver(IHidDevice& hid);

    // One Channel named "RoccatKone" with ROCCAT_KONE_MAX_LEDS LED slots,
    // one Component approximating the mouse's LED layout as a generic
    // placeholder.
    // ponytail: real per-LED mouse coordinates (scroll wheel, side strips,
    // logo) would make the canvas preview look more accurate; replace
    // Component::makeGenericPlaceholder with the actual layout if/when
    // that matters.
    static Channel createDefaultChannel();

    // True only for ChannelMode::Canvas on "RoccatKone"; Native always
    // returns false (Global Constraint: this peripheral is direct-mode
    // only, it has no native lighting mode of its own).
    bool setChannelMode(const std::string& channelName, ChannelMode mode) override;
    ChannelMode channelMode(const std::string& channelName) const override;

    // Capability query (Task 23): true only for ChannelMode::Canvas on
    // "RoccatKone" -- the same rule setChannelMode checks before sending
    // anything, exposed without the HID/isOpen side effects.
    bool supportsMode(const std::string& channelName, ChannelMode mode) const override;

    // Always returns false, no-op: this peripheral has no native mode.
    bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) override;

    // Sends the init report then the colors report via hid.sendFeatureReport(),
    // matching the order RoccatKoneAimoController sends them (SendInit from
    // its constructor, before any SetChannelColors/SendUpdate call).
    // OpenRGB sends the init report exactly once, at device construction;
    // IDriver has no separate "connect"/init hook, so this driver resends
    // it on every push instead. Whether the real mouse tolerates a
    // resent init report on every color update is unverified - confirm
    // against real hardware in Task 18.
    // ponytail: resending a constant 6-byte report on every push is wasted
    // I/O and unverified-safe; if Task 18 shows it needs to be sent only
    // once, add an `inited_` bool here and skip BuildKoneInitReport() after
    // the first successful send.
    // Returns false without sending if !hid.isOpen(), channelName isn't
    // "RoccatKone", or colors.size() exceeds ROCCAT_KONE_MAX_LEDS.
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) override;

private:
    IHidDevice& hid_;
};
