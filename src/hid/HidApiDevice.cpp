#include "hid/HidApiDevice.h"

// vcpkg's hidapi::hidapi target adds .../include/hidapi itself (not its
// parent) to the include path, so the header is <hidapi.h>, not
// <hidapi/hidapi.h>.
#include <hidapi.h>

namespace {
// hid_init()/hid_exit() are hidapi's process-wide lifecycle: hid_init() sets
// up global backend state and hid_exit() tears it down. Since more than one
// HidApiDevice can exist at a time (and the app is single-threaded for HID
// use), a simple instance counter calls hid_init() only for the first live
// instance and hid_exit() only once the last one is destroyed, so instances
// don't stomp on each other's global state.
int g_instanceCount = 0;
}

HidApiDevice::HidApiDevice() {
    if (g_instanceCount == 0) {
        hid_init();
    }
    ++g_instanceCount;
}

HidApiDevice::~HidApiDevice() {
    close();
    --g_instanceCount;
    if (g_instanceCount == 0) {
        hid_exit();
    }
}

bool HidApiDevice::open(uint16_t vendorId, uint16_t productId) {
    close();
    handle_ = hid_open(vendorId, productId, nullptr);
    return handle_ != nullptr;
}

void HidApiDevice::close() {
    if (handle_ != nullptr) {
        hid_close(handle_);
        handle_ = nullptr;
    }
}

bool HidApiDevice::isOpen() const {
    return handle_ != nullptr;
}

bool HidApiDevice::sendFeatureReport(const uint8_t* report, size_t length) {
    if (handle_ == nullptr) {
        return false;
    }
    int sent = hid_send_feature_report(handle_, report, length);
    return sent == static_cast<int>(length);
}
