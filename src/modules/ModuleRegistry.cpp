#include "ModuleRegistry.h"

#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <exception>

namespace {
bool fail(QString* error, const QString& message)
{
    if (error) *error = message;
    return false;
}
}

bool ModuleRegistry::load(const QString& fileName, QString* error)
{
    if (error) error->clear();
    const QFileInfo file(fileName);
    if (!file.isFile() || !QLibrary::isLibrary(file.fileName()))
        return fail(error, "Not a module library: " + fileName);
    auto loader = std::make_shared<QPluginLoader>(file.canonicalFilePath());
    const auto metadata = loader->metaData();
    const auto manifest = metadata.value("MetaData").toObject();
    const auto id = manifest.value("id").toString();
    // Inspect metadata before instantiating executable code. This is an ABI
    // compatibility check, not a signature or a sandbox for untrusted plugins.
    if (metadata.value("IID").toString() != DRIVER_MODULE_IID ||
        manifest.value("apiVersion").toDouble() != 1 || id.trimmed().isEmpty() ||
        manifest.value("version").toString().isEmpty())
        return fail(error, "Incompatible or incomplete module metadata: " + fileName);
    for (const auto& entry : entries_) {
        if (entry.id == id) return fail(error, "Duplicate module ID: " + id);
    }
    loader->setLoadHints(QLibrary::ResolveAllSymbolsHint);
    QObject* instance = loader->instance();
    if (!instance) return fail(error, loader->errorString());
    auto* module = qobject_cast<IDriverModule*>(instance);
    if (!module) return fail(error, "Module does not implement the declared interface: " + id);
    entries_.push_back({id, std::move(loader), module});
    return true;
}

QStringList ModuleRegistry::loadDirectory(const QString& directory)
{
    QStringList errors;
    if (!QDir(directory).exists()) {
        errors.append("Module directory does not exist: " + directory);
        return errors;
    }
    for (const auto& file : QDir(directory).entryInfoList(QDir::Files, QDir::Name)) {
        if (!QLibrary::isLibrary(file.fileName())) continue;
        QString error;
        if (!load(file.absoluteFilePath(), &error)) errors.append(error);
    }
    return errors;
}

QStringList ModuleRegistry::moduleIds() const
{
    QStringList result;
    for (const auto& entry : entries_) result.append(entry.id);
    return result;
}

bool ModuleRegistry::acceptsProfile(const DeviceProfile& profile, QString* error) const
{
    if (error) error->clear();
    for (const auto& entry : entries_) {
        if (entry.id != profile.driverId) continue;
        try {
            return entry.module->acceptsProfile(profile, error);
        } catch (const std::exception& exception) {
            return fail(error, QString::fromUtf8(exception.what()));
        }
    }
    return fail(error, "Required driver module is not loaded: " + profile.driverId);
}

std::unique_ptr<DriverInstance> ModuleRegistry::create(const DeviceIdentity& identity,
    const DeviceProfile& profile, IHidDevice& transport, QString* error) const
{
    if (error) error->clear();
    if (identity.path.isEmpty()) { fail(error, "Select an exact device path before creating a driver"); return {}; }
    if (!profile.matches(identity)) { fail(error, "Device identity does not match the selected profile"); return {}; }
    if (!acceptsProfile(profile, error)) return {};
    for (const auto& entry : entries_) {
        if (entry.id != profile.driverId) continue;
        try {
            auto driver = entry.module->create(transport, profile);
            if (!driver) { fail(error, "Module refused to create a driver"); return {}; }
            auto instance = std::make_unique<DriverInstance>();
            instance->loader_ = entry.loader;
            instance->driver_ = std::move(driver);
            return instance;
        } catch (const std::exception& exception) {
            fail(error, QString::fromUtf8(exception.what()));
            return {};
        }
    }
    return {};
}
