#pragma once

#include "hid/IHidDevice.h"
#include <vector>

class FakeHidDevice : public IHidDevice {
public:
    bool open(std::uint16_t, std::uint16_t) override { opened_ = openSucceeds; return opened_; }
    void close() override { opened_ = false; }
    bool isOpen() const override { return opened_; }
    bool sendFeatureReport(const std::uint8_t* report, std::size_t length) override
    {
        if (!opened_ || !report || length == 0) return false;
        // Record attempted writes even when the simulated OS call fails.
        sentReports.emplace_back(report, report + length);
        return sendSucceeds;
    }
    bool openSucceeds = true;
    bool sendSucceeds = true;
    std::vector<std::vector<std::uint8_t>> sentReports;
private:
    bool opened_ = false;
};
