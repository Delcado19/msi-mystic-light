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

    // Task 21 (USB peripheral reconnect/retry): when enabled (the default),
    // a hard I/O error from sendFeatureReport()/write() -- hidapi reporting
    // -1, as opposed to merely a short write -- closes the handle so
    // isOpen() flips to false. hidapi never nulls the handle itself when a
    // device is unplugged/sleeps; without this, isOpen() would stay true
    // forever for a device that was open at some point, and main.cpp's
    // reconnect timer (gated on isOpen() == false, matching every driver's
    // existing isOpen() guard) would never notice a later disconnect, only
    // a device that was already absent at startup. main.cpp disables this
    // for the motherboard's HidApiDevice instance only: that device has no
    // reconnect timer of its own (out of this task's scope, see main.cpp),
    // so closing its handle on a transient I/O blip would strand it
    // disconnected for the rest of the app's lifetime with nothing left to
    // reopen it -- worse than today's behavior of just retrying the same
    // handle again next frame.
    void setCloseOnIoError(bool enabled) { closeOnIoError_ = enabled; }

private:
    hid_device* handle_ = nullptr;
    bool closeOnIoError_ = true;
};
