#include "device/RoccatVulcanDriver.h"

namespace {
const char* kChannelName = "RoccatVulcan";
// Physical key-matrix dimensions, RoccatVulcanKeyboardLayouts.h:45/55
// (`ROCCAT_VULCAN_120_AIMO_LAYOUT_KEYS_104/105[6][24]`) - 6 rows, 24
// columns. Deliberately distinct from VULCAN_COLUMN_LENGTH (12), which is
// the wire-protocol's addressing stride, not the keyboard's shape.
constexpr int kPhysicalRows = 6;
constexpr int kPhysicalCols = 24;
} // namespace

RoccatVulcanDriver::RoccatVulcanDriver(IHidDevice& controlHid, IHidDevice& ledHid)
    : controlHid_(controlHid), ledHid_(ledHid)
{
}

Channel RoccatVulcanDriver::createDefaultChannel()
{
    Channel channel(kChannelName, VULCAN_MAX_LEDS);

    // 24x6 row/column grid (x=column, y=row) matching the real physical
    // key-matrix dimensions (see kPhysicalRows/kPhysicalCols above), not
    // VULCAN_COLUMN_LENGTH's 12 (a wire-protocol stride, unrelated to key
    // placement). Column-major fill: see header doc comment for the
    // ponytail-scoped simplification this makes vs. the real per-key
    // lookup table.
    Component component;
    component.productName = "Roccat Vulcan 100/120 Aimo";
    component.displayName = "Roccat Vulcan 100/120 Aimo";
    component.brand = "Roccat";
    component.type = "Keyboard";
    component.ledCount = VULCAN_MAX_LEDS;
    component.width = kPhysicalCols;
    component.height = kPhysicalRows;
    for (int i = 0; i < VULCAN_MAX_LEDS; ++i) {
        const int column = i / kPhysicalRows;
        const int row = i % kPhysicalRows;
        component.ledMapping.push_back(i);
        component.ledCoordinates.emplace_back(column, row);
        component.ledNames.push_back("Key " + std::to_string(i + 1));
    }
    channel.addComponent(component);

    return channel;
}

bool RoccatVulcanDriver::supportsMode(const std::string& channelName, ChannelMode mode) const
{
    // Direct-mode only (Global Constraint): Canvas is the only mode this
    // peripheral ever accepts, so Native is never supported.
    return channelName == kChannelName && mode == ChannelMode::Canvas;
}

bool RoccatVulcanDriver::setChannelMode(const std::string& channelName, ChannelMode mode)
{
    if (!supportsMode(channelName, mode)) {
        return false;
    }
    if (!controlHid_.isOpen()) {
        return false;
    }

    // RGBController_RoccatVulcanKeyboard::DeviceUpdateMode (.cpp:314) only
    // calls EnableDirect(true) once, when entering Direct mode - not on
    // every color push (RoccatVulcanProtocol.h's BuildVulcanEnableDirectReport
    // doc comment). setChannelMode(Canvas) is this driver's "entering
    // direct mode" moment, so the report goes here, to controlHid_, never
    // to ledHid_. This keeps pushCanvasColors() LED-interface-only.
    const auto report = BuildVulcanEnableDirectReport(true);
    return controlHid_.sendFeatureReport(report.data(), report.size());
}

ChannelMode RoccatVulcanDriver::channelMode(const std::string& channelName) const
{
    // Canvas is this peripheral's only supported mode; an unknown channel
    // name falls back to Native, mirroring MsiMotherboardDriver's
    // unknown-channel default.
    return channelName == kChannelName ? ChannelMode::Canvas : ChannelMode::Native;
}

bool RoccatVulcanDriver::setNativeMode(const std::string&, const NativeModeSettings&)
{
    // Global Constraint: this peripheral is direct-mode only, it has no
    // native lighting mode of its own, so this is always a no-op.
    return false;
}

bool RoccatVulcanDriver::pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors)
{
    // Both HID interfaces must be open: the real device needs the LED
    // interface open to receive the color packets, and requiring the
    // control interface too (even though it's unused here) matches
    // "this driver owns both handles for this device" - a color push with
    // only half the device's interfaces open is not a state this driver
    // considers ready.
    if (!controlHid_.isOpen() || !ledHid_.isOpen() || channelName != kChannelName
        || colors.size() > static_cast<size_t>(VULCAN_MAX_LEDS)) {
        return false;
    }

    // RoccatVulcanKeyboardController.cpp:388-394: hid_write(dev_led, ...)
    // loop, one output-report write per packet, LED interface only. The
    // control interface is never touched by a color push - EnableDirect
    // is sent once from setChannelMode(Canvas) instead (see above), not
    // resent on every push.
    const auto packets = BuildVulcanColorPackets(colors);
    bool allSent = true;
    for (const auto& packet : packets) {
        allSent = ledHid_.write(packet.data(), packet.size()) && allSent;
    }
    return allSent;
}
