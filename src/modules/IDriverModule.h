#pragma once

#include "DeviceProfile.h"
#include "hid/IHidDevice.h"
#include <QMap>
#include <QtPlugin>
#include <memory>

struct RgbColor { quint8 r = 0, g = 0, b = 0; };

struct NativeEffect {
    QString id = "steady";
    QVector<RgbColor> colors{{255, 255, 255}};
    int brightness = 5;
    int speed = 0;
    bool inward = false;
};

enum class DiscoveryStatus { Unknown, Unsupported, Available, Failed };
struct LedTopology {
    DiscoveryStatus status = DiscoveryStatus::Unknown;
    QMap<QString, int> ledCounts;
    QString error;
};

class IDriver {
public:
    virtual ~IDriver() = default;
    virtual bool setNativeMode(const QString& channel, const NativeEffect& effect) = 0;
    virtual bool pushColors(const QString& channel, const QVector<RgbColor>& colors) = 0;
    // Unknown is distinct from Unsupported: absence of research is not a
    // hardware limitation. Discovery is explicit, never a constructor side effect.
    virtual LedTopology discoverLeds() { return {}; }
};

#define DRIVER_MODULE_IID "org.msi-mystic-light.DriverModule/1.0"

class IDriverModule {
public:
    virtual ~IDriverModule() = default;
    // Validate module-specific addresses/capabilities without any device I/O.
    virtual bool acceptsProfile(const DeviceProfile& profile, QString* error) const = 0;
    // The host owns and opens the transport after conflict resolution. It must
    // outlive the returned driver. Creating a driver must not send reports.
    virtual std::unique_ptr<IDriver> create(IHidDevice& transport, const DeviceProfile& profile) = 0;
};

Q_DECLARE_INTERFACE(IDriverModule, DRIVER_MODULE_IID)
