#include "HidApiDevice.h"

#include <hidapi.h>
#include <limits>
#include <memory>
#include <stdexcept>

namespace {
struct HidRuntime {
    bool ready = hid_init() == 0;
    ~HidRuntime() { if (ready) hid_exit(); }
};

bool initializeHid()
{
    // One process-wide lifetime, established before any wrapper completes
    // construction: destroying one device cannot shut down another's HIDAPI.
    static HidRuntime runtime;
    return runtime.ready;
}
}

HidApiDevice::HidApiDevice() : initialized_(initializeHid()) {}

HidApiDevice::~HidApiDevice()
{
    close();
}

bool HidApiDevice::open(std::uint16_t vendorId, std::uint16_t productId)
{
    close();
    if (!initialized_) return false;
    handle_ = hid_open(vendorId, productId, nullptr);
    return isOpen();
}

std::vector<HidDescriptor> HidApiDevice::enumerate()
{
    if (!initializeHid()) throw std::runtime_error("HIDAPI initialization failed");
    std::unique_ptr<hid_device_info, decltype(&hid_free_enumeration)> list(
        hid_enumerate(0, 0), hid_free_enumeration);
    if (!list) {
        const auto error = hid_error(nullptr);
        if (error && *error) throw std::runtime_error("HID enumeration failed");
        return {};
    }
    std::vector<HidDescriptor> result;
    for (auto* item = list.get(); item; item = item->next) {
        // Unaddressable entries cannot become selectable endpoints.
        if (!item->path || !*item->path) continue;
        HidDescriptor descriptor;
        descriptor.path = item->path;
        descriptor.vendorId = item->vendor_id;
        descriptor.productId = item->product_id;
        descriptor.usagePage = item->usage_page;
        descriptor.usage = item->usage;
        descriptor.interfaceNumber = item->interface_number;
        if (item->manufacturer_string) descriptor.manufacturer = item->manufacturer_string;
        if (item->product_string) descriptor.product = item->product_string;
        result.push_back(std::move(descriptor));
    }
    return result;
}

bool HidApiDevice::openPath(const std::string& path)
{
    close();
    if (!initialized_ || path.empty() || path.find('\0') != std::string::npos) return false;
    handle_ = hid_open_path(path.c_str());
    return isOpen();
}

void HidApiDevice::close()
{
    if (handle_) {
        hid_close(handle_);
        handle_ = nullptr;
    }
}

bool HidApiDevice::isOpen() const
{
    return handle_ != nullptr;
}

bool HidApiDevice::sendFeatureReport(const std::uint8_t* report, std::size_t length)
{
    if (!handle_ || !report || length == 0 ||
        length > static_cast<std::size_t>(std::numeric_limits<int>::max())) return false;
    return hid_send_feature_report(handle_, report, length) == static_cast<int>(length);
}

bool HidApiDevice::writeOutputReport(const std::uint8_t* report, std::size_t length)
{
    if (!handle_ || !report || length == 0 || length > static_cast<std::size_t>(std::numeric_limits<int>::max())) return false;
    return hid_write(handle_, report, length) == static_cast<int>(length);
}
