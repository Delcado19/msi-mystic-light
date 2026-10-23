#pragma once

#include "IHidDevice.h"

struct hid_device_;

class HidApiDevice final : public IHidDevice {
public:
    HidApiDevice();
    ~HidApiDevice() override;
    // Each wrapper owns one handle; copying would close it twice.
    HidApiDevice(const HidApiDevice&) = delete;
    HidApiDevice& operator=(const HidApiDevice&) = delete;

    bool open(std::uint16_t vendorId, std::uint16_t productId) override;
    void close() override;
    bool isOpen() const override;
    bool sendFeatureReport(const std::uint8_t* report, std::size_t length) override;

private:
    hid_device_* handle_ = nullptr;
    bool initialized_;
};
