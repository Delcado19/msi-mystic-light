#include "ComponentEditor.h"
#include <QDialogButtonBox>
#include <QAbstractItemDelegate>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <stdexcept>

ComponentEditor::ComponentEditor(const ComponentDefinition& initial, bool existing, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(existing ? "Edit component geometry" : "Create component");
    resize(540, 560);
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout; layout->addLayout(form);
    auto* id = new QLineEdit(initial.id); id->setObjectName("componentId"); id->setReadOnly(existing);
    auto* name = new QLineEdit(initial.name); name->setObjectName("componentName");
    auto* count = new QSpinBox; count->setObjectName("componentCount");
    count->setRange(1, 65535); count->setKeyboardTracking(false);
    count->setValue(qMax(1, int(initial.positions.size())));
    form->addRow("Stable ID", id); form->addRow("Name", name); form->addRow("LED count", count);
    auto* note = new QLabel("One row per logical LED. X/Y use catalog units (20 canvas pixels at 100%). "
        "Wire indices must contain each integer from 0 to LED count - 1 once. "
        "Editing an existing definition changes all its instances in this document.");
    note->setWordWrap(true); layout->addWidget(note);
    auto* table = new QTableWidget(0, 3); table->setObjectName("componentLeds");
    table->setHorizontalHeaderLabels({"X", "Y", "Wire index"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(table);
    auto resizeRows = [table](int rows) {
        const int before = table->rowCount(); table->setRowCount(rows);
        for (int row = before; row < rows; ++row) {
            table->setItem(row, 0, new QTableWidgetItem(QString::number(row)));
            table->setItem(row, 1, new QTableWidgetItem("0"));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(row)));
        }
    };
    resizeRows(count->value());
    for (int row = 0; row < initial.positions.size(); ++row) {
        table->item(row, 0)->setText(QString::number(initial.positions[row].x(), 'g', 17));
        table->item(row, 1)->setText(QString::number(initial.positions[row].y(), 'g', 17));
        if (row < initial.mapping.size()) table->item(row, 2)->setText(QString::number(initial.mapping[row]));
    }
    connect(count, &QSpinBox::valueChanged, this, resizeRows);
    auto* error = new QLabel; error->setObjectName("componentError"); error->setWordWrap(true);
    layout->addWidget(error);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, [this, error, table] {
        // Finish the active cell editor before reading the candidate; invalid input stays in the dialog.
        for (auto* cell : table->findChildren<QLineEdit*>()) if (cell->isVisible())
            table->itemDelegate()->setModelData(cell, table->model(), table->currentIndex());
        table->setFocus();
        try { component(); accept(); }
        catch (const std::exception& failure) { error->setText(QString::fromUtf8(failure.what())); }
    });
}

ComponentDefinition ComponentEditor::component() const
{
    const auto* table = findChild<QTableWidget*>("componentLeds");
    QJsonArray positions, mapping;
    for (int row = 0; row < table->rowCount(); ++row) {
        double values[3];
        for (int column = 0; column < 3; ++column) {
            bool ok = false;
            const auto* item = table->item(row, column);
            values[column] = item ? item->text().toDouble(&ok) : 0;
            if (!ok) throw std::invalid_argument("Every LED cell must contain a number");
        }
        positions.append(QJsonArray{values[0], values[1]}); mapping.append(values[2]);
    }
    return ComponentDefinition::fromJson({{"schemaVersion", 1},
        {"id", findChild<QLineEdit*>("componentId")->text()},
        {"name", findChild<QLineEdit*>("componentName")->text()},
        {"ledCount", table->rowCount()}, {"positions", positions}, {"mapping", mapping}});
}
