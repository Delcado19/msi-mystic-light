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
#include <QComboBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QDialogButtonBox>
#include <QAbstractItemDelegate>
#include <QFormLayout>
#include <QGroupBox>
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
#include <memory>
#include <tuple>
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
    std::function<void(QPointF)> addPoint;
    std::function<void(int)> wirePoint;
    QPointF pressPosition;
    bool moved=false;
    ComponentDefinition artwork;
    bool showArtwork=true;
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
        if (wirePoint && event->button()==Qt::LeftButton) {
            auto* item=itemAt(event->position().toPoint());
            if (item && item->parentItem()) item=item->parentItem();
            if (item && item->type()==QGraphicsEllipseItem::Type) wirePoint(item->data(1).toInt());
            event->accept();
            return;
        }
        pressPosition=event->position();moved=false;QGraphicsView::mousePressEvent(event);
    }
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        if (addPoint && event->button()==Qt::LeftButton && !itemAt(event->position().toPoint())) {
            addPoint(mapToScene(event->position().toPoint())/property("coordinateScale").toDouble());
            event->accept();
            return;
        }
        QGraphicsView::mouseDoubleClickEvent(event);
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if(event->buttons() & Qt::LeftButton)moved=moved || event->position()!=pressPosition;
        QGraphicsView::mouseMoveEvent(event);snapSelected(event->modifiers());
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        if (wirePoint && event->button()==Qt::LeftButton) { event->accept(); return; }
        if(event->button()==Qt::LeftButton && event->position()!=pressPosition) {moved=true;QGraphicsView::mouseMoveEvent(event);snapSelected(event->modifiers());}
        QGraphicsView::mouseReleaseEvent(event); if(commit) commit();
        moved=false;
    }
    void drawBackground(QPainter* painter,const QRectF& rect) override {
        QGraphicsView::drawBackground(painter,rect);
        auto grid=SnapGrid::load();grid.spacing*=property("coordinateScale").toDouble()/20;
        grid.paint(*painter,rect,transform().m11());
        if (showArtwork) {
            painter->save();painter->scale(24,24);drawHardwareComponentArtwork(*painter,artwork);painter->restore();
        }
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
    setObjectName("componentLab");
    setWindowTitle(instance ? "Arrange device LEDs" : "Component Lab");
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
    auto* note = new QLabel(instance ? "Drag the light points to arrange this component." :
        "Place LEDs sets the physical positions. Wire LEDs sets the address order and groups points that share a color.");
    note->setWordWrap(true); layout->addWidget(note);
    auto* table = new QTableWidget(0, 3);
    table->setItemDelegateForColumn(0,new CoordinateDelegate(table));table->setItemDelegateForColumn(1,new CoordinateDelegate(table)); table->setObjectName("componentLeds");
    table->setHorizontalHeaderLabels({"X", "Y", "Wire index"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    if (instance) table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    auto* arrangement = new QHBoxLayout; layout->addLayout(arrangement);
    auto* shape = new QToolButton; shape->setObjectName("ledArrangementShapes"); shape->setText("Shapes and transforms");
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
    geometry->setMinimumHeight(260);
    auto* visual = new QWidget;
    auto* visualLayout = new QVBoxLayout(visual);
    visualLayout->addWidget(geometry,1);
    tabs->addTab(visual,"Place LEDs");
    table->setParent(this);
    table->hide();
    auto* wire = new QWidget(tabs);
    auto* wireLayout = new QHBoxLayout(wire);
    auto* wiringPanel=new QGroupBox("Wiring");
    auto* wireButtons=new QVBoxLayout(wiringPanel);
    wireLayout->addWidget(wiringPanel);
    auto* wiringInfo=new QLabel("Click LEDs in data order or apply a preset. Points with the same number share an address.");
    wiringInfo->setWordWrap(true);
    wiringInfo->setMaximumWidth(256);
    wireButtons->addWidget(wiringInfo);
    auto* orderButton=new QPushButton("Set order by clicking");
    orderButton->setObjectName("startWireOrder");
    orderButton->setCheckable(true);
    auto* orderStatus=new QLabel;
    orderStatus->setObjectName("wireOrderProgress");
    auto* applyOrder=new QPushButton("Apply order");
    applyOrder->setObjectName("applyWireOrder");
    auto* cancelOrder=new QPushButton("Cancel order");
    auto* preset=new QComboBox;
    preset->setObjectName("wiringPreset");
    preset->setAccessibleName("Wiring order preset");
    preset->addItems({"Wiring presets…","Rows: left to right","Rows: serpentine","Columns: top to bottom","Clockwise from top"});
    for (auto* control:std::initializer_list<QWidget*>{orderButton,orderStatus,applyOrder,cancelOrder,preset})
        wireButtons->addWidget(control);
    auto* addressRow=new QHBoxLayout;
    wireButtons->addLayout(addressRow);
    auto* previousAddress = new QPushButton("Previous address");
    previousAddress->setObjectName("previousLedAddress");
    auto* nextAddress = new QPushButton("Next address");
    nextAddress->setObjectName("nextLedAddress");
    auto* applyAddress = new QPushButton("Assign selected points");
    applyAddress->setObjectName("assignLedAddress");
    applyAddress->setEnabled(nonPhysicalSlots_.isEmpty());
    applyAddress->setToolTip("Choose an address, select light points on the canvas, then assign them together.");
    auto* address = new QSpinBox;
    address->setObjectName("componentWireAddress");
    address->setAccessibleName("LED address");
    address->setRange(1,count->value());
    // Keep the address dial independent of zoom. Selecting a zone highlights all its points.
    for (auto* control : std::initializer_list<QWidget*>{previousAddress,nextAddress}) addressRow->addWidget(control);
    wireButtons->addWidget(new QLabel("LED address"));
    wireButtons->addWidget(address);
    wireButtons->addWidget(applyAddress);
    wireButtons->addStretch();
    if (!instance) tabs->addTab(wire,"Wire LEDs");
    else wire->hide();
    connect(tabs,&QTabWidget::currentChanged,this,[=](int page) {
        geometry->commit();
        shape->setEnabled(page==0);
        if (page==0) visualLayout->addWidget(geometry,1);
        else wireLayout->insertWidget(0,geometry,1);
        geometry->show();
    });
    connect(previousAddress,&QPushButton::clicked,address,&QSpinBox::stepDown);
    connect(nextAddress,&QPushButton::clicked,address,&QSpinBox::stepUp);
    connect(address,&QSpinBox::valueChanged,this,[scene](int value) {
        for (auto* item : scene->items()) if (item->type()==QGraphicsEllipseItem::Type)
            item->setSelected(item->data(1).toInt()==value-1);
    });
    connect(count,&QSpinBox::valueChanged,address,&QSpinBox::setMaximum);
    connect(snapAction,&QAction::toggled,geometry->viewport(),qOverload<>(&QWidget::update));
    connect(step,&QSpinBox::valueChanged,geometry->viewport(),qOverload<>(&QWidget::update));
    const auto pendingOrder=std::make_shared<QVector<int>>();
    auto rebuild=[this,table,scene,geometry,tabs,orderButton,pendingOrder,reserved=nonPhysicalSlots_] {
        scene->clear();
        const bool wiring=tabs->currentIndex()==1;
        // Keep LED markers readable at any geometry scale; positions still use component units.
        const double coordinateScale=24;
        geometry->setProperty("coordinateScale",coordinateScale);
        ComponentDefinition visible;visible.category=category_;
        for(int row=0;row<table->rowCount();++row) {
            if(!table->item(row,0) || !table->item(row,1) || !table->item(row,2)) continue;
            if (reserved.contains(table->item(row,2)->text().toInt())) continue;
            bool validX=false,validY=false;
            const double x=table->item(row,0)->text().toDouble(&validX),y=table->item(row,1)->text().toDouble(&validY);
            if(!validX || !validY || !std::isfinite(x) || !std::isfinite(y) || std::abs(x)>100000 || std::abs(y)>100000) continue;
            visible.positions.append({x,y});visible.mapping.append(row);
            const int address=table->item(row,2)->text().toInt();
            const int pending=orderButton->isChecked() ? pendingOrder->indexOf(address) : -1;
            auto* dot=scene->addEllipse(-16,-16,32,32,QPen(QColor("#93bfff"),1.5),
                QBrush(QColor(pending>=0 ? "#126276" : "#243b53")));
            dot->setData(0,row); dot->setData(1,address); dot->setPos(x*coordinateScale,y*coordinateScale);
            dot->setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIgnoresTransformations);
            dot->setFlag(QGraphicsItem::ItemIsMovable,tabs->currentIndex()==0);
            const auto number=orderButton->isChecked() ? (pending>=0 ? QString::number(pending+1) : QString()) : QString::number(address+1);
            auto* text=new QGraphicsSimpleTextItem(number,dot); text->setBrush(QColor("#ffffff")); QFont font=text->font(); font.setPixelSize(12); font.setWeight(QFont::DemiBold); text->setFont(font);
            text->setScale(qMin(1.0,25.0/qMax(1.0,text->boundingRect().width())));
            const auto size=text->boundingRect().size()*text->scale(); text->setPos(-size.width()/2,-size.height()/2);
            text->setAcceptedMouseButtons(Qt::NoButton);
        }
        if (wiring) {
            QMap<int,QPointF> addressCenters;
            QPen sharedPen(QColor("#f5be69"),2,Qt::DotLine);
            sharedPen.setCosmetic(true);
            for (auto* item:scene->items(Qt::AscendingOrder)) if (item->type()==QGraphicsEllipseItem::Type) {
                const auto address=item->data(1).toInt();
                if (!addressCenters.contains(address)) addressCenters.insert(address,item->pos());
                else scene->addLine(QLineF(addressCenters.value(address),item->pos()),sharedPen)->setZValue(-1);
            }
            QPen pathPen(QColor("#73d9ef"),2,Qt::DashLine);
            pathPen.setCosmetic(true);
            const auto order=orderButton->isChecked() ? *pendingOrder : addressCenters.keys();
            for (int i=1;i<order.size();++i) {
                scene->addLine(QLineF(addressCenters.value(order[i-1]),addressCenters.value(order[i])),pathPen)->setZValue(-1);
            }
        }
        geometry->artwork=visible;
        // Positioning uses the housing as a reference. Wiring gives the address path its own workplane.
        geometry->showArtwork=!wiring;
        const auto housing=hardwareBodyBounds(visible);
        const QRectF sceneHousing(housing.topLeft()*coordinateScale,housing.size()*coordinateScale);
        const auto pointBounds=scene->itemsBoundingRect();
        const int padding=wiring ? 16 : 8;
        scene->setSceneRect((wiring ? pointBounds : pointBounds.united(sceneHousing)).adjusted(-padding,-padding,padding,padding));
        if (!orderButton->isChecked()) geometry->fitInView(scene->sceneRect(),Qt::KeepAspectRatio);
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
    const auto addressPositions=[table] {
        QMap<int,QPointF> positions;
        for (int row=0;row<table->rowCount();++row) {
            const int wire=table->item(row,2)->text().toInt();
            if (!positions.contains(wire))
                positions.insert(wire,{table->item(row,0)->text().toDouble(),table->item(row,1)->text().toDouble()});
        }
        return positions;
    };
    const auto renumber=[table](const QVector<int>& order) {
        QHash<int,int> addresses;
        for (int i=0;i<order.size();++i) addresses.insert(order[i],i);
        const QSignalBlocker block(table);
        for (int row=0;row<table->rowCount();++row) {
            auto* cell=table->item(row,2);
            cell->setText(QString::number(addresses.value(cell->text().toInt())));
        }
    };
    const auto showOrderProgress=[=] {
        const auto count=addressPositions().size();
        orderStatus->setText(QString("%1 of %2 addresses selected").arg(pendingOrder->size()).arg(count));
        applyOrder->setEnabled(pendingOrder->size()==count);
    };
    const bool editableWires=nonPhysicalSlots_.isEmpty();
    orderButton->setEnabled(editableWires);
    preset->setEnabled(editableWires);
    const auto toggleOrder=[=](bool on) {
        pendingOrder->clear();
        geometry->wirePoint={};
        if (on) geometry->wirePoint=[=](int address) {
            // A repeated click on a point in a shared zone counts only once.
            if (pendingOrder->contains(address)) return;
            pendingOrder->append(address);
            showOrderProgress();
            rebuild();
        };
        for (auto* control:std::initializer_list<QWidget*>{orderStatus,applyOrder,cancelOrder}) control->setVisible(on);
        for (auto* control:std::initializer_list<QWidget*>{preset,previousAddress,nextAddress,address,applyAddress})
            control->setEnabled(!on && (editableWires || (control!=applyAddress && control!=preset)));
        showOrderProgress();
        rebuild();
    };
    for (auto* control:std::initializer_list<QWidget*>{orderStatus,applyOrder,cancelOrder}) control->hide();
    connect(orderButton,&QPushButton::toggled,this,toggleOrder);
    connect(cancelOrder,&QPushButton::clicked,this,[orderButton] { orderButton->setChecked(false); });
    connect(applyOrder,&QPushButton::clicked,this,[=] {
        renumber(*pendingOrder);
        orderButton->setChecked(false);
    });
    connect(tabs,&QTabWidget::currentChanged,this,[orderButton](int page) {
        if (page!=1) orderButton->setChecked(false);
    });
    connect(count,&QSpinBox::valueChanged,this,[orderButton] { orderButton->setChecked(false); });
    connect(preset,&QComboBox::activated,this,[=](int pattern) {
        if (!pattern) return;
        const auto positions=addressPositions();
        auto order=positions.keys();
        QPointF center;
        for (const auto& point:positions) center+=point;
        center/=positions.size();
        const auto key=[&](int id) {
            const auto point=positions.value(id);
            if (pattern==4) {
                const auto delta=point-center;
                constexpr double circle=6.283185307179586;
                return std::tuple{std::fmod(std::atan2(delta.x(),-delta.y())+circle,circle),0.0,id};
            }
            if (pattern==3) return std::tuple{double(qRound(point.x()*10)),point.y(),id};
            const auto row=qRound(point.y()*10);
            return std::tuple{double(row),point.x(),id};
        };
        std::sort(order.begin(),order.end(),[&](int a,int b){return key(a)<key(b);});
        if (pattern==2) {
            // Alternate the traversal of successive occupied rows, including across gaps.
            bool reverse=false;
            for (auto first=order.begin();first!=order.end();) {
                auto last=std::find_if(first,order.end(),[&](int id){return std::get<0>(key(id))!=std::get<0>(key(*first));});
                if (reverse) std::reverse(first,last);
                reverse=!reverse;
                first=last;
            }
        }
        renumber(order);
        preset->setCurrentIndex(0);
        rebuild();
    });
    connect(applyAddress,&QPushButton::clicked,this,[=] {
        geometry->commit();
        const QSignalBlocker block(table);
        if (scene->selectedItems().isEmpty()) return;
        for (auto* item : scene->selectedItems())
            table->item(item->data(0).toInt(),2)->setText(QString::number(address->value()-1));
        // Compress unused addresses when a zone's last independent point is reassigned.
        QSet<int> occupied;
        for (int row=0; row<table->rowCount(); ++row) occupied.insert(table->item(row,2)->text().toInt());
        auto order=occupied.values();
        std::sort(order.begin(),order.end());
        for (int row=0; row<table->rowCount(); ++row) {
            auto* cell=table->item(row,2);
            cell->setText(QString::number(order.indexOf(cell->text().toInt())));
        }
        rebuild();
        for (auto* item : scene->items()) if (item->type()==QGraphicsEllipseItem::Type)
            item->setSelected(item->data(1).toInt()==address->value()-1);
    });
    auto resizeRows = [table](int rows) {
        const QSignalBlocker block(table);
        const int before = table->rowCount(); table->setRowCount(rows);
        int nextAddress=0;
        for (int row=0;row<qMin(before,rows);++row)
            nextAddress=qMax(nextAddress,1+table->item(row,2)->text().toInt());
        for (int row = before; row < rows; ++row) {
            table->setItem(row, 0, new QTableWidgetItem(QString::number(row)));
            table->setItem(row, 1, new QTableWidgetItem("0"));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(nextAddress++)));
        }
    };
    resizeRows(count->value());
    for (int row = 0; row < initial.positions.size(); ++row) {
        table->item(row, 0)->setText(QString::number(initial.positions[row].x(), 'g', 17));
        table->item(row, 1)->setText(QString::number(initial.positions[row].y(), 'g', 17));
        if (row < initial.mapping.size()) table->item(row, 2)->setText(QString::number(initial.mapping[row]));
    }
    connect(count,&QSpinBox::valueChanged,this,[=](int rows) {
        resizeRows(rows);
        renumber(addressPositions().keys());
        rebuild();
    });
    connect(table,&QTableWidget::itemChanged,this,[rebuild] { rebuild(); });
    connect(tabs,&QTabWidget::currentChanged,this,[rebuild] { rebuild(); });
    if (!instance && nonPhysicalSlots_.isEmpty()) geometry->addPoint=[=](QPointF point) {
        if (tabs->currentIndex()!=0 || count->value()>=count->maximum()) return;
        geometry->commit();
        const QSignalBlocker block(count);
        count->setValue(count->value()+1);
        resizeRows(count->value());
        const int row=table->rowCount()-1;
        const QSignalBlocker cells(table);
        table->item(row,0)->setText(QString::number(point.x(),'g',17));
        table->item(row,1)->setText(QString::number(point.y(),'g',17));
        address->setMaximum(count->value());
        rebuild();
    };
    for (const auto& label : QStringList{"Line", "Circle", "Ellipse", "Rectangle", "Matrix", "L-Shape", "U-Shape"}) {
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
                else if (label=="Matrix") {
                    const int columns=int(std::ceil(std::sqrt(physicalCount)));
                    const int rows=(physicalCount+columns-1)/columns;
                    point={width*(index%columns)/(columns-1),height*(index/columns)/qMax(1,rows-1)};
                } else if (label=="L-Shape" || label=="U-Shape") {
                    const bool u=label=="U-Shape";
                    const double length=double(index)*(width+height*(u ? 2 : 1))/(physicalCount-1);
                    if (length<=height) point={0,length};
                    else if (length<=height+width) point={length-height,height};
                    else point={width,2*height+width-length};
                }
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
    shapes->addSeparator();
    for (const auto& [label,column] : {qMakePair(QString("Mirror horizontally"),0),qMakePair(QString("Mirror vertically"),1)}) {
        auto* action=shapes->addAction(label);
        action->setObjectName(column==0 ? "mirrorLedsHorizontally" : "mirrorLedsVertically");
        connect(action,&QAction::triggered,this,[table,geometry,rebuild,column,reserved=nonPhysicalSlots_] {
            geometry->commit();
            QVector<QTableWidgetItem*> points;
            for (auto* item:geometry->scene()->selectedItems())
                if (item->type()==QGraphicsEllipseItem::Type) points.append(table->item(item->data(0).toInt(),column));
            if (points.isEmpty()) for (int row=0;row<table->rowCount();++row)
                if (!reserved.contains(table->item(row,2)->text().toInt())) points.append(table->item(row,column));
            if (points.isEmpty()) return;
            const auto coordinate=[](const auto* item) {return item->text().toDouble();};
            const auto bounds=std::minmax_element(points.begin(),points.end(),[&](auto* a,auto* b) {return coordinate(a)<coordinate(b);});
            const double sum=coordinate(*bounds.first)+coordinate(*bounds.second);
            const QSignalBlocker block(table);
            for (auto* point:points) point->setText(QString::number(sum-coordinate(point),'g',17));
            rebuild();
        });
    }
    if (!existing) {
        auto* templates = new QComboBox;
        templates->addItems({"Template","Fan","Ring","Strip"});
        templates->setAccessibleName("Start from a component template");
        arrangement->insertWidget(0,templates);
        connect(templates,&QComboBox::currentIndexChanged,this,[=](int item) {
            if (item==0) return;
            category_=item==1 ? "Fans" : item==3 ? "Strips" : "Other";
            rebuild();
            for (auto* action : shapes->actions())
                if (action->text()==(item==3 ? "Line" : "Circle")) action->trigger();
            templates->setCurrentIndex(0);
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
    connect(tabs,&QTabWidget::currentChanged,error,&QLabel::clear);
    connect(table,&QTableWidget::itemChanged,error,&QLabel::clear);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel); layout->addWidget(buttons);
    if (!instance) {
        buttons->button(QDialogButtonBox::Ok)->setText(existing ? "Save component" : "Create component");
        connect(orderButton,&QPushButton::toggled,buttons,[buttons](bool on) {buttons->button(QDialogButtonBox::Ok)->setEnabled(!on);});
    }
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
    QSet<int> addresses;
    for (int row = 0; row < table->rowCount(); ++row) {
        double values[3];
        for (int column = 0; column < 3; ++column) {
            bool ok = false;
            const auto* item = table->item(row, column);
            values[column] = item ? item->text().toDouble(&ok) : 0;
            if (!ok) throw std::invalid_argument("Every LED cell must contain a number");
        }
        positions.append(QJsonArray{values[0], values[1]}); mapping.append(values[2]);
        if (values[2]>=0 && values[2]<table->rowCount() && std::floor(values[2])==values[2])
            addresses.insert(int(values[2]));
    }
    QJsonArray reserved; for (int wire : nonPhysicalSlots_) reserved.append(wire);
    QJsonObject definition{{"schemaVersion", 1}, {"nonPhysicalSlots", reserved},
        {"manufacturer",manufacturer_},{"category",category_},
        {"id", findChild<QLineEdit*>("componentId")->text()},
        {"name", findChild<QLineEdit*>("componentName")->text()},
        {"ledCount", table->rowCount()}, {"positions", positions}, {"mapping", mapping}};
    if (addresses.size()<table->rowCount()) {
        definition["schemaVersion"]=2;
        definition["addressCount"]=int(addresses.size());
    }
    return ComponentDefinition::fromJson(definition);
}
