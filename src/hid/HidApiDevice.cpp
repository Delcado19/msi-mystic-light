#include "hid/HidApiDevice.h"

// vcpkg's hidapi::hidapi target adds .../include/hidapi itself (not its
// parent) to the include path, so the header is <hidapi.h>, not
// <hidapi/hidapi.h>.
#include <hidapi.h>

namespace {
// hid_init()/hid_exit() are hidapi's process-wide lifecycle: hid_init() sets
// up global backend state and hid_exit() tears it down. Handles are constructed
// before the hardware worker starts and destroyed after it joins; lifecycle
// operations remain on the main thread. The counter initializes the first live
// instance and calls hid_exit() only after the last is destroyed, so instances
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

bool HidApiDevice::open(uint16_t vendorId, uint16_t productId, int interfaceNumber, int usagePage) {
    close();
    if (interfaceNumber == -1 && usagePage == -1) {
        // Preserves v1's exact existing behavior: hid_open opens the first
        // VID/PID match, with no way to target a specific interface.
        handle_ = hid_open(vendorId, productId, nullptr);
        return handle_ != nullptr;
    }

    // A device exposing several HID interfaces (or several collections
    // sharing one interface_number) under one VID/PID needs a specific
    // interface_number and/or usage_page, which plain hid_open can't
    // target. Enumerate and match, then open that exact path.
    hid_device_info* devs = hid_enumerate(vendorId, productId);
    hid_device_info* match = nullptr;
    for (hid_device_info* cur = devs; cur != nullptr; cur = cur->next) {
        if ((interfaceNumber == -1 || cur->interface_number == interfaceNumber)
            && (usagePage == -1 || cur->usage_page == usagePage)) {
            match = cur;
            break;
        }
    }
    if (match != nullptr) {
        handle_ = hid_open_path(match->path);
    }
    hid_free_enumeration(devs);
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
    // Task 21: sent < 0 is hidapi's hard-error return (as opposed to a
    // merely short count, which can't happen for a feature report's
    // strict == check below), typically meaning the device went away
    // (unplugged, or the handle was invalidated by a sleep/resume). Close
    // the handle so isOpen() reflects that -- see setCloseOnIoError()'s
    // doc comment in HidApiDevice.h for why this is opt-out-able and why
    // the default is to close.
    if (sent < 0 && closeOnIoError_) {
        close();
    }
    return sent == static_cast<int>(length);
}

bool HidApiDevice::write(const uint8_t* report, size_t length) {
    if (handle_ == nullptr) {
        return false;
    }
    int sent = hid_write(handle_, report, length);
    // Task 21: same hard-error close as sendFeatureReport() above. Only
    // sent < 0 (an actual error) closes the handle -- a non-negative but
    // short count is the padding behavior the comment below already
    // tolerates, not a disconnect signal.
    if (sent < 0 && closeOnIoError_) {
        close();
    }
    // Unlike hid_send_feature_report (which echoes back the caller's exact
    // length), hidapi's Windows backend pads a report shorter than the
    // device's output_report_length and returns the padded byte count
    // (see windows/hid.c hid_write()). A strict == check would therefore
    // report failure for a correctly-sent short report, so accept >=.
    return sent >= static_cast<int>(length);
}
