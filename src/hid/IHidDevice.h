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
    // Append transport methods to preserve API 1 feature-report vtable slots.
    // Modules using output reports require host API 2; old hosts reject their metadata.
    virtual bool writeOutputReport(const std::uint8_t*, std::size_t) { return false; }
    // Read-back methods, API 3. Discovery/config-readback modules require host
    // API 3; older hosts reject their metadata. Returns the number of bytes
    // actually read, or 0 on a closed device, invalid buffer or OS error.
    // buffer[0] on input selects the report ID to request (hidapi convention);
    // the call overwrites the buffer with the response.
    virtual std::size_t getFeatureReport(std::uint8_t*, std::size_t) { return 0; }
    // Interrupt input report read; no report-ID selection. Same API 3 gate.
    virtual std::size_t readInputReport(std::uint8_t*, std::size_t) { return 0; }
};
