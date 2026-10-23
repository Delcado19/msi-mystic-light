#include "HidApiDevice.h"

#include <hidapi.h>
#include <limits>

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
