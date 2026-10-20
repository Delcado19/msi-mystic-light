// Real-hardware IHidDevice backend, wrapping hidapi. No hardware-independent
// behavior to unit test; exercised manually against real devices in Task 12.
#pragma once

#include "hid/IHidDevice.h"

struct hid_device_;
typedef struct hid_device_ hid_device;

class HidApiDevice : public IHidDevice {
public:
    HidApiDevice();
    ~HidApiDevice() override;

    // Copying/moving would share handle_ across instances without running
    // the ctor body (the dtor still runs per copy), breaking both the
    // hid_init()/hid_exit() reference count and ownership of the hidapi
    // handle (double hid_close()).
    HidApiDevice(const HidApiDevice&) = delete;
    HidApiDevice& operator=(const HidApiDevice&) = delete;

    bool open(uint16_t vendorId, uint16_t productId, int interfaceNumber = -1, int usagePage = -1) override;
    void close() override;
    bool isOpen() const override;
    bool sendFeatureReport(const uint8_t* report, size_t length) override;
    bool write(const uint8_t* report, size_t length) override;

private:
    hid_device* handle_ = nullptr;
};
