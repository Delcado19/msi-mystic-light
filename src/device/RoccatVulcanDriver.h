// Driver for the Roccat Vulcan 100/120 Aimo keyboard. Direct-mode only, per
// the project's Global Constraint for peripherals: Native mode is never
// supported, so setNativeMode is always a no-op and setChannelMode never
// accepts ChannelMode::Native.
//
// Unlike the other three peripheral drivers (SteelSeries Apex 3, Roccat
// Kone Aimo, Razer Basilisk V3), this device exposes TWO HID interfaces
// under one VID/PID: a control interface (mode/brightness/init, feature
// reports - RoccatVulcanKeyboardController.cpp's dev_ctrl) and a separate
// LED interface (per-key color data, output reports via hid_write -
// dev_led). Only the LED interface is used here: this task ports
// SendColors (the per-key direct-mode color push) only, not
// SendMode/EnableDirect (which write to the control interface to
// activate/configure a device-side mode - out of scope per the task
// brief).
#pragma once

#include <string>
#include <vector>

#include "device/Channel.h"
#include "device/DeviceTypes.h"
#include "device/IDriver.h"
#include "hid/IHidDevice.h"
#include "protocol/RoccatVulcanProtocol.h"

class RoccatVulcanDriver : public IDriver {
public:
    RoccatVulcanDriver(IHidDevice& controlHid, IHidDevice& ledHid);

    // One Channel named "RoccatVulcan" with VULCAN_MAX_LEDS (144) LED
    // slots, one Component approximating the key matrix as a 12x12
    // row/column grid (x=column, y=row - matching BuildVulcanColorPackets'
    // column/row derivation, VULCAN_COLUMN_LENGTH=12).
    // ponytail: this is the physical key-matrix shape, not exact keycap
    // pixel positions - refine later only if the canvas preview's visual
    // accuracy demands it, per the task brief's note; protocol correctness
    // does not depend on this.
    static Channel createDefaultChannel();

    // True only for ChannelMode::Canvas on "RoccatVulcan"; Native always
    // returns false (Global Constraint: this peripheral is direct-mode
    // only, it has no native lighting mode of its own).
    bool setChannelMode(const std::string& channelName, ChannelMode mode) override;
    ChannelMode channelMode(const std::string& channelName) const override;

    // Always returns false, no-op: this peripheral has no native mode.
    bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) override;

    // Writes BuildVulcanColorPackets' packets to ledHid_ only via write()
    // (output report - RoccatVulcanKeyboardController.cpp:388-394's
    // hid_write(dev_led, ...) loop). controlHid_ is never touched by a
    // color push: this driver does not implement the control-interface
    // mode-activation handshake (SendMode/EnableDirect), see header note.
    // Returns false without sending if either HID is not open, channelName
    // isn't "RoccatVulcan", or colors.size() exceeds VULCAN_MAX_LEDS.
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) override;

private:
    IHidDevice& controlHid_;
    IHidDevice& ledHid_;
};
