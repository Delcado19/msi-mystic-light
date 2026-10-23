#include "ComponentEditor.h"
#include "StudioWidgets.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QTabWidget>
#include <QPushButton>
#include <QSignalBlocker>
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
#include <cmath>
#include <functional>
#include <QTimer>

namespace {
class GeometryView : public QGraphicsView {
public:
    using QGraphicsView::QGraphicsView;
    std::function<void()> commit;
protected:
    void mouseReleaseEvent(QMouseEvent* event) override { QGraphicsView::mouseReleaseEvent(event); if(commit) commit(); }
    void wheelEvent(QWheelEvent* event) override {
        const double factor=std::pow(1.15,event->angleDelta().y()/120.0);
        if(transform().m11()*factor>0.00001 && transform().m11()*factor<20) scale(factor,factor);
        event->accept();
    }
};
}
ComponentEditor::ComponentEditor(const ComponentDefinition& initial, bool existing, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(existing ? "Edit component geometry" : "Create component");
    resize(760, 640);
    applyStudioStyle(this);
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout; layout->addLayout(form);
    auto* id = new QLineEdit(initial.id); id->setObjectName("componentId"); id->setReadOnly(existing);
    auto* name = new QLineEdit(initial.name); name->setObjectName("componentName");
    auto* count = new QSpinBox; count->setObjectName("componentCount");
    count->setRange(1, 65535); count->setKeyboardTracking(false);
    count->setValue(qMax(1, int(initial.positions.size())));
    form->addRow("Stable ID", id); form->addRow("Name", name); form->addRow("LED count", count);
    auto* note = new QLabel("Drag LEDs to arrange the component; wheel zooms. Numbers show the physical wire order. "
        "Use Coordinates for exact positions or wire mapping. Editing a definition changes all its instances.");
    note->setWordWrap(true); layout->addWidget(note);
    auto* table = new QTableWidget(0, 3); table->setObjectName("componentLeds");
    table->setHorizontalHeaderLabels({"X", "Y", "Wire index"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    auto* tabs=new QTabWidget; tabs->setObjectName("componentViews"); layout->addWidget(tabs,1);
    auto* geometry=new GeometryView; geometry->setObjectName("componentGeometry");
    auto* scene=new QGraphicsScene(geometry); geometry->setScene(scene); geometry->setRenderHint(QPainter::Antialiasing);
    geometry->setBackgroundBrush(QColor("#17191f")); geometry->setDragMode(QGraphicsView::RubberBandDrag);
    geometry->setMinimumHeight(260); tabs->addTab(geometry,"Visual layout"); tabs->addTab(table,"Coordinates & wire mapping");
    auto rebuild=[table,scene,geometry] {
        scene->clear();
        for(int row=0;row<table->rowCount();++row) {
            if(!table->item(row,0) || !table->item(row,1) || !table->item(row,2)) continue;
            bool validX=false,validY=false;
            const double x=table->item(row,0)->text().toDouble(&validX),y=table->item(row,1)->text().toDouble(&validY);
            if(!validX || !validY || !std::isfinite(x) || !std::isfinite(y) || std::abs(x)>100000 || std::abs(y)>100000) continue;
            auto* dot=scene->addEllipse(-7,-7,14,14,QPen(QColor("#c7d8ff")),QBrush(QColor("#7199ee")));
            dot->setData(0,row); dot->setPos(x*24,y*24);
            dot->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
            auto* text=new QGraphicsSimpleTextItem(table->item(row,2)->text(),dot); text->setBrush(QColor("#dce7ff")); text->setPos(9,-16);
        }
        scene->setSceneRect(scene->itemsBoundingRect().adjusted(-28,-28,28,28));
        geometry->fitInView(scene->sceneRect(),Qt::KeepAspectRatio);
        // Do not magnify a single LED until it fills the dialog.
        if(geometry->transform().m11()>2) { geometry->resetTransform(); geometry->scale(2,2); }
    };
    // Commit only on release; table validation remains the single boundary when accepting the definition.
    geometry->commit=[table,scene] {
        const QSignalBlocker block(table);
        for(auto* item:scene->items()) if(item->type()==QGraphicsEllipseItem::Type) {
            const int row=item->data(0).toInt();
            table->item(row,0)->setText(QString::number(item->pos().x()/24,'g',17));
            table->item(row,1)->setText(QString::number(item->pos().y()/24,'g',17));
        }
    };
    auto resizeRows = [table](int rows) {
        const QSignalBlocker block(table);
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
    connect(count, &QSpinBox::valueChanged, this, [resizeRows,rebuild](int rows) { resizeRows(rows); rebuild(); });
    connect(table,&QTableWidget::itemChanged,this,[rebuild] { rebuild(); });
    auto* fit=new QPushButton("Fit LEDs"); layout->addWidget(fit); connect(fit,&QPushButton::clicked,this,rebuild);
    QTimer::singleShot(0,this,rebuild);
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
    installParameterTooltips(this);
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
