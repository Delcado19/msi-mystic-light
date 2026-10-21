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
// dev_led). The control interface is used for exactly one thing here:
// EnableDirect's one-time "enter direct mode" report, sent from
// setChannelMode(Canvas) (mirroring RGBController_RoccatVulcanKeyboard::
// DeviceUpdateMode calling EnableDirect(true) once, not per push - see
// protocol/RoccatVulcanProtocol.h). SendMode (the much larger mode/speed/
// brightness/static-color control report, for non-Direct modes this
// Canvas-only driver never uses) is out of scope.
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

    // One channel with 144 hardware-addressed slots and the actual 24x6
    // ISO (DE/UK/FR) key positions. Unused wire slots share the origin;
    // ANSI/JIS layout detection is not implemented.
    static Channel createDefaultChannel();

    // True only for ChannelMode::Canvas on "RoccatVulcan"; Native always
    // returns false (Global Constraint: this peripheral is direct-mode
    // only, it has no native lighting mode of its own). On success for
    // Canvas, also sends BuildVulcanEnableDirectReport(true) to
    // controlHid_ (RoccatVulcanProtocol.h's EnableDirect port) - the
    // one-time "enter direct mode" report a real device needs before it
    // will honor color pushes. Returns false, without sending, if
    // controlHid_ is not open.
    bool setChannelMode(const std::string& channelName, ChannelMode mode) override;
    ChannelMode channelMode(const std::string& channelName) const override;

    // Capability query (Task 23): true only for ChannelMode::Canvas on
    // "RoccatVulcan" -- the same rule setChannelMode checks before sending
    // anything, exposed without the HID/isOpen side effects.
    bool supportsMode(const std::string& channelName, ChannelMode mode) const override;

    // Always returns false, no-op: this peripheral has no native mode.
    bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) override;

    // Writes BuildVulcanColorPackets' packets to ledHid_ only via write()
    // (output report - RoccatVulcanKeyboardController.cpp:388-394's
    // hid_write(dev_led, ...) loop). controlHid_ is never touched by a
    // color push: EnableDirect is sent once from setChannelMode(Canvas)
    // instead (see above), not resent on every push.
    // Returns false without sending if either HID is not open, channelName
    // isn't "RoccatVulcan", or colors.size() exceeds VULCAN_MAX_LEDS.
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) override;

private:
    IHidDevice& controlHid_;
    IHidDevice& ledHid_;
};
