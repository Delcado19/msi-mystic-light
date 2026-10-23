#pragma once
#include "components/ComponentCatalog.h"
#include <QDialog>
#include <QVariantMap>
class QComboBox;
class QListWidget;
class QSpinBox;
struct HeaderChoice { QString name; QVariantMap identity; int remaining=0, required=0; };
// A data-only product picker; selecting a model never discovers, opens or writes hardware.
class HardwarePicker : public QDialog {
public:
    HardwarePicker(const ComponentCatalog& catalog,const QVector<HeaderChoice>& headers,const QVariantMap& selected,QWidget* parent=nullptr);
    ComponentDefinition component() const;
    int quantity() const;
    QVariantMap header() const;
private:
    ComponentCatalog products_;
    QVector<HeaderChoice> headers_;
    QComboBox* header_;
    QListWidget* productsList_;
    QSpinBox *quantity_, *leds_;
};
