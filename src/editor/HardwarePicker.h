#pragma once
#include "components/ComponentCatalog.h"
#include <QDialog>
#include <QVariantMap>
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QSpinBox;
#include <QSet>
// Every built-in product the picker offers: the given catalog plus validated imports of locally installed
// SignalRGB metadata ("hardware.*"), without reverse fan variants. Built-ins themselves are files only.
ComponentCatalog hardwareProducts(const ComponentCatalog& catalog);
// One header in the Attach dialog. kind is "5 V ARGB" / "12 V RGB"; an unsupported header is listed but
// cannot be chosen, with reason as its tooltip (plan 2026-11-27 Task 6).
struct HeaderChoice { QString name; QVariantMap identity; int remaining=0, required=0; QString kind; bool supported=true; QString reason; };
// "Attach component": a data-only picker; choosing never discovers, opens or writes hardware.
// Counts are addresses only (owner decision 2026-11-27).
class HardwarePicker : public QDialog {
public:
    HardwarePicker(const ComponentCatalog& builtIn,const ComponentCatalog& user,const QVector<HeaderChoice>& headers,const QVariantMap& selected,QWidget* parent=nullptr);
    // Built-in catalog only (no user components).
    HardwarePicker(const ComponentCatalog& catalog,const QVector<HeaderChoice>& headers,const QVariantMap& selected,QWidget* parent=nullptr)
        : HardwarePicker(catalog,{},headers,selected,parent) {}
    ComponentDefinition component() const;
    int quantity() const;
    QVariantMap header() const;
private:
    void filter();
    void update();
    ComponentCatalog products_;
    QSet<QString> user_;
    QVector<HeaderChoice> headers_;
    QComboBox *header_, *category_, *manufacturer_;
    QLineEdit* search_;
    QListWidget* productsList_;
    QListWidgetItem* create_ = nullptr;
    QLabel *remaining_, *selectionName_, *selectionAddresses_;
    QSpinBox* quantity_;
    QPushButton* attach_;
};
