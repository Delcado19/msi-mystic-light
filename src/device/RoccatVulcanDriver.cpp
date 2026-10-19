#include "device/RoccatVulcanDriver.h"

namespace {
const char* kChannelName = "RoccatVulcan";
} // namespace

RoccatVulcanDriver::RoccatVulcanDriver(IHidDevice& controlHid, IHidDevice& ledHid)
    : controlHid_(controlHid), ledHid_(ledHid)
{
}

Channel RoccatVulcanDriver::createDefaultChannel()
{
    Channel channel(kChannelName, VULCAN_MAX_LEDS);

    // 12x12 row/column grid (x=column, y=row), matching
    // BuildVulcanColorPackets' column/row derivation
    // (VULCAN_COLUMN_LENGTH=12 rows per column). Per the task brief's
    // ponytail note: an approximation of the physical key matrix, not
    // exact keycap pixel positions - protocol correctness doesn't depend
    // on this, only the canvas preview's visual layout.
    Component component;
    component.productName = "Roccat Vulcan 100/120 Aimo";
    component.displayName = "Roccat Vulcan 100/120 Aimo";
    component.brand = "Roccat";
    component.type = "Keyboard";
    component.ledCount = VULCAN_MAX_LEDS;
    component.width = VULCAN_COLUMN_LENGTH;
    component.height = VULCAN_COLUMN_LENGTH;
    for (int i = 0; i < VULCAN_MAX_LEDS; ++i) {
        const int column = i / VULCAN_COLUMN_LENGTH;
        const int row = i % VULCAN_COLUMN_LENGTH;
        component.ledMapping.push_back(i);
        component.ledCoordinates.emplace_back(column, row);
        component.ledNames.push_back("Key " + std::to_string(i + 1));
    }
    channel.addComponent(component);

    return channel;
}

bool RoccatVulcanDriver::setChannelMode(const std::string& channelName, ChannelMode mode)
{
    // Direct-mode only (Global Constraint): Canvas is the only mode this
    // peripheral ever accepts, so Native is always rejected here.
    if (channelName != kChannelName || mode != ChannelMode::Canvas) {
        return false;
    }
    return true;
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
    // control interface is never touched by a color push (this driver
    // doesn't implement SendMode/EnableDirect's control-interface
    // handshake - see RoccatVulcanDriver.h).
    const auto packets = BuildVulcanColorPackets(colors);
    bool allSent = true;
    for (const auto& packet : packets) {
        allSent = ledHid_.write(packet.data(), packet.size()) && allSent;
    }
    return allSent;
}
