#pragma once

#include "IDriverModule.h"
#include <QPluginLoader>
#include <vector>

class DriverInstance {
public:
    DriverInstance() = default;
    DriverInstance(const DriverInstance&) = delete;
    DriverInstance& operator=(const DriverInstance&) = delete;
    // Move the unique_ptr handle, not the object: loader ownership must not be
    // reassigned before the previous driver's destructor has finished.
    IDriver& driver() const { return *driver_; }
private:
    friend class ModuleRegistry;
    // Reverse destruction order keeps plugin code loaded through driver teardown.
    std::shared_ptr<QPluginLoader> loader_;
    std::unique_ptr<IDriver> driver_;
};

class ModuleRegistry {
public:
    bool load(const QString& fileName, QString* error);
    QStringList loadDirectory(const QString& directory);
    QStringList moduleIds() const;
    bool acceptsProfile(const DeviceProfile& profile, QString* error) const;
    // Explicit creation only. No automatic choice when several profiles match.
    std::unique_ptr<DriverInstance> create(const DeviceIdentity& identity,
        const DeviceProfile& profile, IHidDevice& transport, QString* error) const;
private:
    struct Entry {
        QString id;
        std::shared_ptr<QPluginLoader> loader;
        IDriverModule* module;
    };
    std::vector<Entry> entries_;
};
