#include "device/RoccatVulcanDriver.h"

namespace {
const char* kChannelName = "RoccatVulcan";
// ISO (DE/UK/FR) positions translated from OpenRGB's
// RoccatVulcan120AimoLayouts: matrix indices reference led_names, whose
// IDs are reference hardware addresses, not sequential key indices. This is
// the initial fallback only, not proof of unit-specific physical mapping.
// Saved wire calibration can correct this template, including Win/Right Ctrl.
constexpr int kPhysicalRows = 6;
constexpr int kPhysicalCols = 24;
constexpr int kIsoWireIds[kPhysicalRows][kPhysicalCols] = {
    { 0x00, -1, 0x0B, 0x11, 0x17, 0x1C, -1, 0x30, 0x35, 0x3B, 0x41, 0x4E, 0x54, 0x55, 0x56, -1, 0x63, 0x67, 0x6C, -1, -1, -1, -1, -1 },
    { 0x01, 0x06, 0x0C, 0x12, 0x18, 0x1D, 0x21, 0x31, 0x36, 0x3C, 0x42, 0x48, 0x4F, 0x57, -1, -1, 0x64, 0x68, 0x6D, -1, 0x71, 0x77, 0x7C, 0x81 },
    { 0x02, -1, 0x07, 0x0D, 0x13, 0x19, 0x1E, 0x22, 0x32, 0x37, 0x3D, 0x43, 0x49, 0x50, -1, -1, 0x65, 0x69, 0x6E, -1, 0x72, 0x78, 0x7D, 0x82 },
    { 0x03, -1, 0x08, 0x0E, 0x14, 0x1A, 0x1F, 0x23, 0x33, 0x38, 0x3E, 0x44, 0x4A, 0x60, 0x58, -1, -1, -1, -1, -1, 0x73, 0x79, 0x7E, -1 },
    { 0x04, 0x09, 0x0F, 0x15, 0x1B, 0x20, 0x24, 0x34, 0x39, 0x3F, 0x45, 0x4B, -1, 0x52, -1, -1, -1, 0x6A, -1, -1, 0x74, 0x7A, 0x7F, 0x83 },
    { 0x05, 0x0A, 0x10, -1, -1, -1, 0x25, -1, -1, -1, 0x46, 0x4C, 0x53, -1, 0x59, -1, 0x66, 0x6B, 0x6F, -1, 0x75, -1, 0x80, -1 },
};
} // namespace

RoccatVulcanDriver::RoccatVulcanDriver(IHidDevice& controlHid, IHidDevice& ledHid)
    : controlHid_(controlHid), ledHid_(ledHid)
{
}

Channel RoccatVulcanDriver::createDefaultChannel()
{
    Channel channel(kChannelName, VULCAN_MAX_LEDS);

    Component component;
    component.productName = "Roccat Vulcan 100/120 Aimo";
    component.displayName = "Roccat Vulcan 100/120 Aimo";
    component.brand = "Roccat";
    component.type = "Keyboard";
    component.ledCount = VULCAN_MAX_LEDS;
    component.width = kPhysicalCols;
    component.height = kPhysicalRows;
    // Keep all 144 packet slots. Unpopulated slots are excluded from geometry
    // and remain zero in normal rendering, matching the reference packet.
    component.ledCoordinates.resize(VULCAN_MAX_LEDS, {0, 0});
    for (int i = 0; i < VULCAN_MAX_LEDS; ++i) {
        component.ledMapping.push_back(i);
        component.ledNames.push_back("Wire slot " + std::to_string(i));
    }
    for (int row = 0; row < kPhysicalRows; ++row) {
        for (int column = 0; column < kPhysicalCols; ++column) {
            const int id = kIsoWireIds[row][column];
            if (id >= 0) {
                component.ledCoordinates[id] = {column, row};
            }
        }
    }
    // ISO key legends are keyed by wire address; unnamed slots are not keys.
    component.ledNames[0x00] = "Esc";
    component.ledNames[0x01] = "^";
    component.ledNames[0x02] = "Tab";
    component.ledNames[0x03] = "Caps";
    component.ledNames[0x04] = "Shift";
    component.ledNames[0x05] = "Ctrl";
    component.ledNames[0x06] = "1";
    component.ledNames[0x09] = "< >";
    component.ledNames[0x0A] = "Win";
    component.ledNames[0x0B] = "F1";
    component.ledNames[0x0C] = "2";
    component.ledNames[0x07] = "Q";
    component.ledNames[0x08] = "A";
    component.ledNames[0x0F] = "Y";
    component.ledNames[0x10] = "Alt";
    component.ledNames[0x11] = "F2";
    component.ledNames[0x12] = "3";
    component.ledNames[0x0D] = "W";
    component.ledNames[0x0E] = "S";
    component.ledNames[0x15] = "X";
    component.ledNames[0x17] = "F3";
    component.ledNames[0x18] = "4";
    component.ledNames[0x13] = "E";
    component.ledNames[0x14] = "D";
    component.ledNames[0x1B] = "C";
    component.ledNames[0x1C] = "F4";
    component.ledNames[0x1D] = "5";
    component.ledNames[0x19] = "R";
    component.ledNames[0x1A] = "F";
    component.ledNames[0x20] = "V";
    component.ledNames[0x21] = "6";
    component.ledNames[0x1E] = "T";
    component.ledNames[0x1F] = "G";
    component.ledNames[0x24] = "B";
    component.ledNames[0x25] = "Space";
    component.ledNames[0x30] = "F5";
    component.ledNames[0x31] = "7";
    component.ledNames[0x22] = "Z";
    component.ledNames[0x23] = "H";
    component.ledNames[0x34] = "N";
    component.ledNames[0x35] = "F6";
    component.ledNames[0x36] = "8";
    component.ledNames[0x32] = "U";
    component.ledNames[0x33] = "J";
    component.ledNames[0x39] = "M";
    component.ledNames[0x3B] = "F7";
    component.ledNames[0x3C] = "9";
    component.ledNames[0x37] = "I";
    component.ledNames[0x38] = "K";
    component.ledNames[0x3F] = ",";
    component.ledNames[0x41] = "F8";
    component.ledNames[0x42] = "0";
    component.ledNames[0x3D] = "O";
    component.ledNames[0x3E] = "L";
    component.ledNames[0x45] = ".";
    component.ledNames[0x46] = "AltGr";
    component.ledNames[0x4E] = "F9";
    component.ledNames[0x48] = "\xC3\x9F";
    component.ledNames[0x43] = "P";
    component.ledNames[0x44] = "\xC3\x96";
    component.ledNames[0x4B] = "-";
    component.ledNames[0x4C] = "Fn";
    component.ledNames[0x54] = "F10";
    component.ledNames[0x4F] = "'";
    component.ledNames[0x49] = "\xC3\x9C";
    component.ledNames[0x4A] = "\xC3\x84";
    component.ledNames[0x53] = "Menu";
    component.ledNames[0x55] = "F11";
    component.ledNames[0x57] = "Back";
    component.ledNames[0x50] = "+";
    component.ledNames[0x60] = "#";
    component.ledNames[0x52] = "Shift";
    component.ledNames[0x56] = "F12";
    component.ledNames[0x58] = "Enter";
    component.ledNames[0x59] = "Ctrl";
    component.ledNames[0x63] = "Print";
    component.ledNames[0x64] = "Insert";
    component.ledNames[0x65] = "Delete";
    component.ledNames[0x66] = "Left";
    component.ledNames[0x67] = "Scroll";
    component.ledNames[0x68] = "Home";
    component.ledNames[0x69] = "End";
    component.ledNames[0x6A] = "Up";
    component.ledNames[0x6B] = "Down";
    component.ledNames[0x6C] = "Pause";
    component.ledNames[0x6D] = "PgUp";
    component.ledNames[0x6E] = "PgDn";
    component.ledNames[0x6F] = "Right";
    component.ledNames[0x71] = "Num";
    component.ledNames[0x72] = "7";
    component.ledNames[0x73] = "4";
    component.ledNames[0x74] = "1";
    component.ledNames[0x75] = "0";
    component.ledNames[0x77] = "/";
    component.ledNames[0x78] = "8";
    component.ledNames[0x79] = "5";
    component.ledNames[0x7A] = "2";
    component.ledNames[0x7C] = "*";
    component.ledNames[0x7D] = "9";
    component.ledNames[0x7E] = "6";
    component.ledNames[0x7F] = "3";
    component.ledNames[0x80] = ",";
    component.ledNames[0x81] = "-";
    component.ledNames[0x82] = "+";
    component.ledNames[0x83] = "Enter";
    component.ledNames[0x09] = "< >";
    component.ledNames[0x0A] = "Win";
    component.ledNames[0x59] = "Ctrl";
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
