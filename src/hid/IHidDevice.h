// Transport abstraction for a USB HID device. Isolates device drivers (the
// MSI motherboard driver from Task 6, and the Roccat/Razer/SteelSeries
// peripheral drivers added on top of this in later tasks) from the concrete
// hidapi backend, so each can be unit-tested against FakeHidDevice
// (tests/hid/FakeHidDevice.h) without real hardware.
#pragma once

#include <cstddef>
#include <cstdint>

class IHidDevice {
public:
    virtual ~IHidDevice() = default;
    // interfaceNumber == -1 (default) opens the first match for
    // vendorId/productId, same as v1's behavior. A device exposing
    // several HID interfaces under one VID/PID (e.g. Roccat Vulcan's
    // separate control and LED interfaces) is opened once per interface
    // by constructing a separate IHidDevice per interfaceNumber.
    virtual bool open(uint16_t vendorId, uint16_t productId, int interfaceNumber = -1) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    // report must already include the report ID as report[0].
    // Returns true iff the OS call reported success (sent length == declared length).
    virtual bool sendFeatureReport(const uint8_t* report, size_t length) = 0;
    // Output report (hid_write), as opposed to sendFeatureReport's feature
    // report (hid_send_feature_report). Some peripherals (e.g. the Roccat
    // Vulcan's LED interface) send color data via output report instead.
    // report must already include the report ID as report[0].
    // Returns true iff the OS call reported success. Unlike
    // sendFeatureReport, a successful call may report more bytes sent than
    // length (hidapi can pad a short report up to the device's max output
    // report size on some platforms), so implementations should treat
    // "sent >= length" as success, not strict equality.
    virtual bool write(const uint8_t* report, size_t length) = 0;
};
