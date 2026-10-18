// Transport abstraction for a USB HID device. Isolates the MSI motherboard
// driver (Task 6) from the concrete hidapi backend so it can be unit-tested
// against FakeHidDevice (tests/hid/FakeHidDevice.h) without real hardware.
#pragma once

#include <cstddef>
#include <cstdint>

class IHidDevice {
public:
    virtual ~IHidDevice() = default;
    virtual bool open(uint16_t vendorId, uint16_t productId) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    // report must already include the report ID as report[0].
    // Returns true iff the OS call reported success (sent length == declared length).
    virtual bool sendFeatureReport(const uint8_t* report, size_t length) = 0;
};
