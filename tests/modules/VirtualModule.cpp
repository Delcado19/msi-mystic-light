#include "modules/IDriverModule.h"
#include <QObject>

// An independent test protocol proves the host needs no MSI-specific branch.
class VirtualDriver : public IDriver {
public:
    explicit VirtualDriver(IHidDevice& transport) : transport_(transport) {}
    bool setNativeMode(const QString&, const NativeEffect&) override { return false; }
    bool pushColors(const QString& channel, const QVector<RgbColor>& colors) override
    {
        if (!transport_.isOpen() || channel != "Virtual" || colors.size() != 1) return false;
        const std::uint8_t report[]{1, colors[0].r, colors[0].g, colors[0].b};
        return transport_.sendFeatureReport(report, sizeof(report));
    }
private:
    IHidDevice& transport_;
};

class VirtualModule : public QObject, public IDriverModule {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "virtual.json")
    Q_INTERFACES(IDriverModule)
public:
    bool acceptsProfile(const DeviceProfile& profile, QString* error) const override
    {
        if (profile.driverId == "test.virtual") return true;
        if (error) *error = "Wrong virtual profile";
        return false;
    }
    std::unique_ptr<IDriver> create(IHidDevice& transport, const DeviceProfile&) override
    {
        return std::make_unique<VirtualDriver>(transport);
    }
};
#include "VirtualModule.moc"
