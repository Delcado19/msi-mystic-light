// Test double for IHidDevice. Header-only, test-only infrastructure: no
// production code depends on this. Lets Task 6 and Task 8 test real driver
// logic without a physical device.
#pragma once

#include "hid/IHidDevice.h"

#include <cstdint>
#include <vector>

class FakeHidDevice : public IHidDevice {
public:
    bool open(uint16_t, uint16_t) override { opened_ = true; return openSucceeds; }
    void close() override { opened_ = false; }
    bool isOpen() const override { return opened_; }
    bool sendFeatureReport(const uint8_t* report, size_t length) override {
        sentReports.emplace_back(report, report + length);
        return sendSucceeds;
    }
    bool openSucceeds = true;
    bool sendSucceeds = true;
    std::vector<std::vector<uint8_t>> sentReports; // every report sent, in order
private:
    bool opened_ = false;
};
