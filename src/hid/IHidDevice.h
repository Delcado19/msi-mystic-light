#pragma once

#include <cstddef>
#include <cstdint>

class IHidDevice {
public:
    virtual ~IHidDevice() = default;
    virtual bool open(std::uint16_t vendorId, std::uint16_t productId) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    // Byte 0 includes the report ID. True means the entire report was sent;
    // closed devices, invalid buffers, short writes and OS errors return false.
    virtual bool sendFeatureReport(const std::uint8_t* report, std::size_t length) = 0;
};
