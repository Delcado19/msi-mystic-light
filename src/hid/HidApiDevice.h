#pragma once

#include "IHidDevice.h"
#include <string>
#include <vector>

struct HidDescriptor {
    std::string path;
    std::uint16_t vendorId = 0, productId = 0;
    int usagePage = -1, usage = -1, interfaceNumber = -1;
    std::wstring manufacturer, product;
};

struct hid_device_;

class HidApiDevice final : public IHidDevice {
public:
    HidApiDevice();
    ~HidApiDevice() override;
    // Each wrapper owns one handle; copying would close it twice.
    HidApiDevice(const HidApiDevice&) = delete;
    HidApiDevice& operator=(const HidApiDevice&) = delete;

    bool open(std::uint16_t vendorId, std::uint16_t productId) override;
    // Discovery copies descriptors and never opens a device or sends reports.
    static std::vector<HidDescriptor> enumerate();
    // Use the exact enumerated path after interface/profile selection.
    bool openPath(const std::string& path);
    void close() override;
    bool isOpen() const override;
    bool writeOutputReport(const std::uint8_t* report, std::size_t length) override;
    bool sendFeatureReport(const std::uint8_t* report, std::size_t length) override;

private:
    hid_device_* handle_ = nullptr;
    bool initialized_;
};
