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
#include <functional>
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
    // Quick-create (plan 2026-11-27 Task 7): saves a new user component and returns its ID; throws with a
    // reason the dialog shows. The window saves into the user component folder.
    std::function<QString(const ComponentDefinition&)> saveComponent;
    // exec() result when the user chose "Open in editor…"; editorName() is the name typed so far.
    static constexpr int OpenEditor = 2;
    QString editorName() const;
    void selectComponent(const QString& id);
private:
    void filter();
    void update();
    void showQuickForm(const QString& name);
    void quickCreate();
    ComponentCatalog products_;
    QSet<QString> user_;
    QVector<HeaderChoice> headers_;
    QComboBox *header_, *category_, *manufacturer_;
    QLineEdit* search_;
    QListWidget* productsList_;
    QListWidgetItem *create_ = nullptr, *newRow_ = nullptr;
    QWidget* quickForm_ = nullptr;
    QLineEdit* quickName_ = nullptr;
    QComboBox* quickManufacturer_ = nullptr;
    QSpinBox* quickAddresses_ = nullptr;
    QString quickType_ = "Fans";
    QLabel* quickError_ = nullptr;
    QLabel *remaining_, *selectionName_, *selectionAddresses_;
    QSpinBox* quantity_;
    QPushButton* attach_;
};
