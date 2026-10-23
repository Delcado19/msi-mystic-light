#include "modules/IDriverModule.h"
#include <QObject>

class IncompatibleModule : public QObject {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "incompatible.json")
};
#include "IncompatibleModule.moc"
