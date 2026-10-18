// Windows autostart registration via the per-user Run registry key
// (HKCU\Software\Microsoft\Windows\CurrentVersion\Run) — the standard way a
// non-elevated app registers to launch at login, no installer/service
// needed. Used by Task 12's application wiring to offer a "start with
// Windows" option.
#pragma once

#include <string>

#include "platform/win/IAutostart.h"

class AutostartWin : public IAutostart {
public:
    // valueName: registry value name under
    // HKCU\Software\Microsoft\Windows\CurrentVersion\Run. exePath: the
    // string written as that value's data when enabled.
    AutostartWin(std::wstring valueName, std::wstring exePath);

    // True iff valueName exists under Run and its string data equals exePath.
    bool isEnabled() const override;
    // true: writes exePath to valueName (REG_SZ). false: deletes valueName
    // if present. Returns true iff the registry operation succeeded
    // (deleting an already-absent value counts as success).
    bool setEnabled(bool enabled) override;

private:
    std::wstring valueName_, exePath_;
};
