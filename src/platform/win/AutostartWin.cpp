#include "platform/win/AutostartWin.h"

#include <windows.h>

namespace {
const wchar_t* kRunKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
}

AutostartWin::AutostartWin(std::wstring valueName, std::wstring exePath)
    : valueName_(std::move(valueName)), exePath_(std::move(exePath))
{
}

bool AutostartWin::isEnabled() const
{
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }

    // Query the required buffer size first, then read the string into it;
    // this handles values of any length without guessing a fixed buffer.
    DWORD type = 0;
    DWORD byteSize = 0;
    LONG status = RegQueryValueExW(key, valueName_.c_str(), nullptr, &type, nullptr, &byteSize);
    if (status != ERROR_SUCCESS || type != REG_SZ || byteSize == 0) {
        RegCloseKey(key);
        return false;
    }

    std::wstring data(byteSize / sizeof(wchar_t), L'\0');
    status = RegQueryValueExW(key, valueName_.c_str(), nullptr, &type,
                               reinterpret_cast<BYTE*>(data.data()), &byteSize);
    RegCloseKey(key);
    if (status != ERROR_SUCCESS) {
        return false;
    }

    // Strip the trailing NUL(s) RegQueryValueExW includes in byteSize so the
    // comparison against exePath_ (which has none) is exact.
    size_t nulPos = data.find(L'\0');
    if (nulPos != std::wstring::npos) {
        data.resize(nulPos);
    }
    return data == exePath_;
}

bool AutostartWin::setEnabled(bool enabled)
{
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_WRITE, &key) != ERROR_SUCCESS) {
        return false;
    }

    LONG status;
    if (enabled) {
        // +1 to include the null terminator in the written REG_SZ data, as
        // the Win32 registry API requires.
        DWORD byteSize = static_cast<DWORD>((exePath_.size() + 1) * sizeof(wchar_t));
        status = RegSetValueExW(key, valueName_.c_str(), 0, REG_SZ,
                                 reinterpret_cast<const BYTE*>(exePath_.c_str()), byteSize);
    } else {
        status = RegDeleteValueW(key, valueName_.c_str());
        // Deleting a value that was never set isn't a failure for this
        // API's contract (setEnabled(false) just means "not registered").
        if (status == ERROR_FILE_NOT_FOUND) {
            status = ERROR_SUCCESS;
        }
    }

    RegCloseKey(key);
    return status == ERROR_SUCCESS;
}
