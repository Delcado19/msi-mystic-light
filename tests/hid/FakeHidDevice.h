// Test double for IHidDevice. Header-only, test-only infrastructure: no
// production code depends on this. Lets Task 6 and Task 8 test real driver
// logic without a physical device.
#pragma once

#include "hid/IHidDevice.h"

#include <cstdint>
#include <vector>

class FakeHidDevice : public IHidDevice {
public:
    bool open(uint16_t, uint16_t, int interfaceNumber = -1, int usagePage = -1) override {
        // Task 21: opened_ tracks openSucceeds, matching HidApiDevice's real
        // semantics (handle_ is only non-null when hid_open()/hid_open_path()
        // actually succeeded) -- a failed open() must leave isOpen() false,
        // otherwise a reconnect-retry loop gated on isOpen() could never be
        // exercised against a simulated failed reopen.
        opened_ = openSucceeds;
        openInterfaceNumber = interfaceNumber;
        openUsagePage = usagePage;
        return openSucceeds;
    }
    void close() override { opened_ = false; }
    bool isOpen() const override { return opened_; }
    bool sendFeatureReport(const uint8_t* report, size_t length) override {
        sentReports.emplace_back(report, report + length);
        return sendSucceeds;
    }
    bool write(const uint8_t* report, size_t length) override {
        writtenReports.emplace_back(report, report + length);
        return writeSucceeds;
    }
    bool openSucceeds = true;
    bool sendSucceeds = true;
    bool writeSucceeds = true;
    int openInterfaceNumber = -1; // interfaceNumber last passed to open()
    int openUsagePage = -1; // usagePage last passed to open()
    std::vector<std::vector<uint8_t>> sentReports;   // every report sent via sendFeatureReport, in order
    std::vector<std::vector<uint8_t>> writtenReports; // every report sent via write, in order
private:
    bool opened_ = false;
};
