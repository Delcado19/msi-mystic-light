#include "modules/IDriverModule.h"
#include <QObject>

class MissingInterface : public QObject {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "missing-interface.json")
};
#include "MissingInterface.moc"
