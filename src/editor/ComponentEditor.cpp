#include "ComponentEditor.h"
#include "HardwareIdentity.h"
#include "HardwareArtwork.h"
#include "SnapGrid.h"
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
#include <algorithm>
#include <QTimer>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QStyledItemDelegate>

namespace {
// Display rounded coordinates without changing the exact text used for editing and serialization.
class CoordinateDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QString displayText(const QVariant& value,const QLocale& locale) const override {
        bool ok=false;const double number=value.toString().toDouble(&ok);
        if(!ok || !std::isfinite(number) || std::abs(number)>100000)return value.toString();
        return (std::abs(number-std::round(number))>1e-7 ? QString::fromUtf8("\xE2\x89\x88 ") : QString{})+locale.toString(qRound64(number));
    }
};
class GeometryView : public QGraphicsView {
public:
    using QGraphicsView::QGraphicsView;
    std::function<void()> commit;
    QPointF pressPosition;
    bool moved=false;
    ComponentDefinition artwork;
protected:
    void snapSelected(Qt::KeyboardModifiers modifiers) {
        auto grid=SnapGrid::load();
        if(!moved || !grid.enabled || (modifiers & Qt::AltModifier) || !scene()->mouseGrabberItem())return;
        grid.spacing*=property("coordinateScale").toDouble()/20; // Component units are 20 Layout pixels, independent of presentation zoom.
        auto items=scene()->selectedItems();
        if(items.isEmpty())return;
        auto* anchor=*std::min_element(items.begin(),items.end(),[](auto* a,auto* b){return a->data(0).toInt()<b->data(0).toInt();});
        const auto delta=grid.snap(anchor->pos())-anchor->pos();
        for(auto* item:items)item->setPos(item->pos()+delta);
    }
    void mousePressEvent(QMouseEvent* event) override {
        pressPosition=event->position();moved=false;QGraphicsView::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if(event->buttons() & Qt::LeftButton)moved=moved || event->position()!=pressPosition;
        QGraphicsView::mouseMoveEvent(event);snapSelected(event->modifiers());
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        if(event->button()==Qt::LeftButton && event->position()!=pressPosition) {moved=true;QGraphicsView::mouseMoveEvent(event);snapSelected(event->modifiers());}
        QGraphicsView::mouseReleaseEvent(event); if(commit) commit();
        moved=false;
    }
    void drawBackground(QPainter* painter,const QRectF& rect) override {
        QGraphicsView::drawBackground(painter,rect);
        auto grid=SnapGrid::load();grid.spacing*=property("coordinateScale").toDouble()/20;
        grid.paint(*painter,rect,transform().m11());
        painter->save();painter->scale(24,24);drawHardwareComponentArtwork(*painter,artwork);painter->restore();
    }
    void wheelEvent(QWheelEvent* event) override {
        const double factor=std::pow(1.15,event->angleDelta().y()/120.0);
        if(transform().m11()*factor>0.00001 && transform().m11()*factor<20) scale(factor,factor);
        event->accept();
    }
};
}
ComponentEditor::ComponentEditor(const ComponentDefinition& initial, bool existing, QWidget* parent, bool instance)
    : QDialog(parent), manufacturer_(hardwareManufacturer(initial)), category_(hardwareCategory(initial)), nonPhysicalSlots_(initial.nonPhysicalSlots)
{
    setWindowTitle(instance ? "Arrange device LEDs" : existing ? "Edit component geometry" : "Create component");
    resize(1180, 760);
    applyStudioStyle(this);
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout; layout->addLayout(form);
    auto* id = new QLineEdit(initial.id, this); id->setObjectName("componentId"); id->setReadOnly(existing);
    auto* name = new QLineEdit(initial.name); name->setObjectName("componentName");
    auto* count = new QSpinBox; count->setObjectName("componentCount");
    count->setRange(1, 65535); count->setKeyboardTracking(false);
    count->setValue(qMax(1, int(initial.positions.size())));
    if (!instance) form->addRow("Stable ID", id);
    else id->hide();
    name->setReadOnly(instance); count->setEnabled(!instance && nonPhysicalSlots_.isEmpty());
    form->addRow("Name", name); form->addRow(nonPhysicalSlots_.isEmpty() ? "LED count" : "Protocol slots", count);
    if (!nonPhysicalSlots_.isEmpty()) form->addRow("Physical LEDs", new QLabel(QString::number(initial.positions.size()-nonPhysicalSlots_.size())));
    auto* note = new QLabel(instance ? "Arrange LEDs using a shape, then drag individual LEDs to refine. Numbers show physical wire order. Only this device changes." : "Drag LEDs to arrange the component; wheel zooms. Numbers show the physical wire order. "
        "Use Coordinates for exact positions or wire mapping. Editing a definition changes all its instances.");
    note->setWordWrap(true); layout->addWidget(note);
    auto* table = new QTableWidget(0, 3);
    table->setItemDelegateForColumn(0,new CoordinateDelegate(table));table->setItemDelegateForColumn(1,new CoordinateDelegate(table)); table->setObjectName("componentLeds");
    table->setHorizontalHeaderLabels({"X", "Y", "Wire index"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    if (instance) table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    auto* arrangement = new QHBoxLayout; layout->addLayout(arrangement);
    auto* shape = new QToolButton; shape->setObjectName("ledArrangementShapes"); shape->setText("Arrange LEDs");
    shape->setPopupMode(QToolButton::InstantPopup); auto* shapes = new QMenu(shape); shape->setMenu(shapes);
    shape->setToolTip("Arrange this device's LED positions using a shape; physical wire order is preserved");
    arrangement->addWidget(shape); arrangement->addStretch();
    auto* snapAction=parent ? parent->findChild<QAction*>("snapToGrid") : nullptr;
    if(!snapAction) {
        snapAction=new QAction("Snap to grid",this);snapAction->setCheckable(true);snapAction->setChecked(SnapGrid::load().enabled);
        connect(snapAction,&QAction::toggled,this,[](bool enabled){auto grid=SnapGrid::load();grid.enabled=enabled;grid.save();});
    }
    auto* snapButton=new QToolButton;snapButton->setDefaultAction(snapAction);arrangement->addWidget(snapButton);
    snapButton->setToolTip("Snap LEDs to the grid without changing wire order. Hold Alt to move freely.");
    auto* step=new QSpinBox;step->setObjectName("ledGridSpacing");step->setRange(1,100);step->setValue(int(SnapGrid::load().spacing));step->setSuffix(" px");
    step->setAccessibleName("LED grid spacing in Layout pixels");arrangement->addWidget(step);
    auto* mainStep=parent ? parent->findChild<QSpinBox*>("gridSpacing") : nullptr;
    if(mainStep) connect(mainStep,&QSpinBox::valueChanged,step,&QSpinBox::setValue);
    connect(step,&QSpinBox::valueChanged,this,[mainStep](int value){auto grid=SnapGrid::load();grid.spacing=value;grid.save();if(mainStep)mainStep->setValue(value);});
    auto* tabs=new QTabWidget; tabs->setObjectName("componentViews"); layout->addWidget(tabs,1);
    auto* geometry=new GeometryView; geometry->setObjectName("componentGeometry");
    auto* scene=new QGraphicsScene(geometry); geometry->setScene(scene); geometry->setRenderHint(QPainter::Antialiasing);
    geometry->setBackgroundBrush(QColor("#17191f")); geometry->setDragMode(QGraphicsView::RubberBandDrag);
    geometry->setMinimumHeight(260); tabs->addTab(geometry,"Visual layout"); tabs->addTab(table,"Coordinates & wire mapping");
    connect(snapAction,&QAction::toggled,geometry->viewport(),qOverload<>(&QWidget::update));
    connect(step,&QSpinBox::valueChanged,geometry->viewport(),qOverload<>(&QWidget::update));
    auto rebuild=[table,scene,geometry,reserved=nonPhysicalSlots_,category=hardwareCategory(initial)] {
        scene->clear();
        // Keep LED markers readable at any geometry scale; positions still use component units.
        const double coordinateScale=24;
        geometry->setProperty("coordinateScale",coordinateScale);
        ComponentDefinition visible;visible.category=category;
        for(int row=0;row<table->rowCount();++row) {
            if(!table->item(row,0) || !table->item(row,1) || !table->item(row,2)) continue;
            if (reserved.contains(table->item(row,2)->text().toInt())) continue;
            bool validX=false,validY=false;
            const double x=table->item(row,0)->text().toDouble(&validX),y=table->item(row,1)->text().toDouble(&validY);
            if(!validX || !validY || !std::isfinite(x) || !std::isfinite(y) || std::abs(x)>100000 || std::abs(y)>100000) continue;
            visible.positions.append({x,y});visible.mapping.append(row);
            auto* dot=scene->addEllipse(-16,-16,32,32,QPen(QColor("#93bfff"),1.5),QBrush(QColor("#243b53")));
            dot->setData(0,row); dot->setPos(x*coordinateScale,y*coordinateScale);
            dot->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIgnoresTransformations);
            auto* text=new QGraphicsSimpleTextItem(table->item(row,2)->text(),dot); text->setBrush(QColor("#ffffff")); QFont font=text->font(); font.setPixelSize(12); font.setWeight(QFont::DemiBold); text->setFont(font);
            text->setScale(qMin(1.0,25.0/qMax(1.0,text->boundingRect().width())));
            const auto size=text->boundingRect().size()*text->scale(); text->setPos(-size.width()/2,-size.height()/2);
            text->setAcceptedMouseButtons(Qt::NoButton);
        }
        geometry->artwork=visible;
        const auto housing=hardwareBodyBounds(visible);
        const QRectF sceneHousing(housing.topLeft()*coordinateScale,housing.size()*coordinateScale);
        scene->setSceneRect(scene->itemsBoundingRect().united(sceneHousing).adjusted(-8,-8,8,8));
        geometry->fitInView(scene->sceneRect(),Qt::KeepAspectRatio);
        // Do not magnify a single LED until it fills the dialog.
        if(table->rowCount()==1 && geometry->transform().m11()>2) { geometry->resetTransform(); geometry->scale(2,2); }
    };
    // Commit only on release; table validation remains the single boundary when accepting the definition.
    geometry->commit=[table,scene,geometry] {
        const QSignalBlocker block(table);
        for(auto* item:scene->items()) if(item->type()==QGraphicsEllipseItem::Type) {
            const int row=item->data(0).toInt();
            table->item(row,0)->setText(QString::number(item->pos().x()/geometry->property("coordinateScale").toDouble(),'g',17));
            table->item(row,1)->setText(QString::number(item->pos().y()/geometry->property("coordinateScale").toDouble(),'g',17));
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
    for (const auto& label : QStringList{"Line", "Circle", "Ellipse", "Rectangle"}) {
        auto* action = shapes->addAction(label); action->setObjectName("arrange" + label);
        connect(action, &QAction::triggered, this, [table, geometry, rebuild, label, reserved=nonPhysicalSlots_] {
            geometry->commit();
            QRectF bounds;
            for (int row=0; row<table->rowCount(); ++row) {
                const QPointF point(table->item(row,0)->text().toDouble(), table->item(row,1)->text().toDouble());
                if (!row) bounds=QRectF(point,QSizeF(0,0));
                else bounds=QRectF(QPointF(qMin(bounds.left(),point.x()),qMin(bounds.top(),point.y())),
                    QPointF(qMax(bounds.right(),point.x()),qMax(bounds.bottom(),point.y())));
            }
            const double width=qMax(4.0,bounds.width()), height=qMax(4.0,bounds.height());
            const int physicalCount=table->rowCount()-reserved.size();
            int physicalRow=0;
            const QSignalBlocker block(table);
            for (int row=0; row<table->rowCount(); ++row) {
                if (reserved.contains(table->item(row,2)->text().toInt())) continue;
                const int index=physicalRow++;
                const double t=double(index)/physicalCount, angle=t*2*3.141592653589793-3.141592653589793/2;
                QPointF point;
                if (physicalCount==1) point={width/2,height/2};
                else if (label=="Line") point={index*width/(physicalCount-1),0};
                else if (label=="Circle" || label=="Ellipse")
                    point={width/2+width/2*std::cos(angle),height/2+(label=="Circle" ? width : height)/2*std::sin(angle)};
                else {
                    const double edge=t*4, f=edge-int(edge);
                    switch(int(edge)) {
                    case 0: point={f*width,0}; break;
                    case 1: point={width,f*height}; break;
                    case 2: point={(1-f)*width,height}; break;
                    default: point={0,(1-f)*height}; break;
                    }
                }
                point+=bounds.topLeft();
                table->item(row,0)->setText(QString::number(point.x(),'g',17));
                table->item(row,1)->setText(QString::number(point.y(),'g',17));
            }
            rebuild();
        });
    }
    auto* reset=new QPushButton("Reset arrangement"); reset->setObjectName("resetLedArrangement"); arrangement->addWidget(reset);
    connect(reset,&QPushButton::clicked,this,[table,initial,rebuild] {
        const QSignalBlocker block(table);
        if (table->rowCount()!=initial.positions.size()) return;
        for(int row=0;row<initial.positions.size();++row) {
            table->item(row,0)->setText(QString::number(initial.positions[row].x(),'g',17));
            table->item(row,1)->setText(QString::number(initial.positions[row].y(),'g',17));
        }
        rebuild();
    });
    auto* fit=new QPushButton("Fit LEDs"); arrangement->addWidget(fit); connect(fit,&QPushButton::clicked,this,rebuild);
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
    QJsonArray reserved; for (int wire : nonPhysicalSlots_) reserved.append(wire);
    return ComponentDefinition::fromJson({{"schemaVersion", 1}, {"nonPhysicalSlots", reserved},
        {"manufacturer",manufacturer_},{"category",category_},
        {"id", findChild<QLineEdit*>("componentId")->text()},
        {"name", findChild<QLineEdit*>("componentName")->text()},
        {"ledCount", table->rowCount()}, {"positions", positions}, {"mapping", mapping}});
}
