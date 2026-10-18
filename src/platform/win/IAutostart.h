// Abstraction over "launch this app at Windows login", so Task 12's
// application wiring and tests can depend on an interface rather than the
// concrete Win32-registry-backed implementation (AutostartWin.h).
#pragma once

class IAutostart {
public:
    virtual ~IAutostart() = default;
    virtual bool isEnabled() const = 0;
    virtual bool setEnabled(bool enabled) = 0;
};
