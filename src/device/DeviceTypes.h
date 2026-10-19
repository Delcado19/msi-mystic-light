// Value types shared by every device driver (IDriver implementations) and by
// callers such as Canvas and Settings. Kept in their own header so that
// IDriver.h can name them without including a concrete driver header.
#pragma once

#include <cstdint>

#include "protocol/MsiProtocol.h"

enum class ChannelMode { Canvas, Native };

// Mirrors MsiZoneSettings; kept separate so callers of the driver don't
// need to know the Option1/Option2 bit-packing, only plain fields.
struct NativeModeSettings {
    MsiLightingMode lightingMode = MsiLightingMode::Steady;
    MsiColor color1{}, color2{}, color3{}, color4{};
    uint8_t colorCount = 1;
    bool selectAll = false;
    bool directionInward = false;
    bool colorSelectionUser = true;
    uint8_t brightness = 5;
    uint8_t speed = 0;
    uint8_t cycleNum = 30;
};
