#pragma once

#include "hid/IHidDevice.h"
#include <algorithm>
#include <deque>
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
        record('S', report, length);
        return sendSucceeds;
    }
    bool writeOutputReport(const std::uint8_t* report, std::size_t length) override
    {
        if (!opened_ || !report || length == 0) return false;
        sentReports.emplace_back(report, report + length);
        return sendSucceeds;
    }
    std::size_t getFeatureReport(std::uint8_t* buffer, std::size_t length) override
    {
        if (!opened_ || !buffer || length == 0) return 0;
        ++getFeatureReportCalls;
        record('G', buffer, 0);
        // Queued replies are returned in order and may be shorter than requested.
        auto response = featureReportResponse;
        if (!featureReportQueue.empty()) { response = featureReportQueue.front(); featureReportQueue.pop_front(); }
        const auto copyLength = std::min(length, response.size());
        std::copy_n(response.begin(), copyLength, buffer);
        return readSucceeds ? copyLength : 0;
    }
    std::size_t readInputReport(std::uint8_t* buffer, std::size_t length) override
    {
        if (!opened_ || !buffer || length == 0) return 0;
        ++readInputReportCalls;
        const auto copyLength = std::min(length, inputReportResponse.size());
        std::copy_n(inputReportResponse.begin(), copyLength, buffer);
        return readSucceeds ? copyLength : 0;
    }
    bool openSucceeds = true;
    bool sendSucceeds = true;
    bool readSucceeds = true;
    std::vector<std::vector<std::uint8_t>> sentReports;
    std::vector<std::uint8_t> featureReportResponse;
    std::deque<std::vector<std::uint8_t>> featureReportQueue;
    // Every feature SET ('S' + bytes) and GET ('G') in call order.
    std::vector<std::vector<std::uint8_t>> operations;
    std::vector<std::uint8_t> inputReportResponse;
    int getFeatureReportCalls = 0;
    int readInputReportCalls = 0;
private:
    void record(std::uint8_t tag, const std::uint8_t* bytes, std::size_t length)
    {
        std::vector<std::uint8_t> entry{tag}; entry.insert(entry.end(), bytes, bytes + length); operations.push_back(entry);
    }
    bool opened_ = false;
};
