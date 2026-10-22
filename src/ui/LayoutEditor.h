#pragma once
#include "device/Component.h"
#include "ui/DeviceArtwork.h"
#include <QTimer>
#include <QCheckBox>
#include <QSpinBox>
#include <QScrollBar>
#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QWheelEvent>
#include <QSvgRenderer>
#include <cmath>
#include <memory>

class ComponentTransformHandle : public QGraphicsEllipseItem {
public:
    ComponentTransformHandle(QGraphicsRectItem* owner,bool rotate):QGraphicsEllipseItem(-6,-6,12,12,owner),owner_(owner),rotate_(rotate) {
        setBrush(rotate?QColor("#17222e"):QColor("#71e2c5"));
        if(rotate)setRect(-10,-10,20,20);setFlag(ItemIgnoresTransformations);setZValue(5);
        setCursor(rotate?Qt::CrossCursor:Qt::SizeFDiagCursor);setToolTip(rotate?"Drag to rotate component":"Drag to resize component");
        setPos(rotate?QPointF(owner->rect().center().x(),owner->rect().top()-25):owner->rect().bottomRight());
    }
protected:
    void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget)override {
        if(!rotate_){QGraphicsEllipseItem::paint(painter,option,widget);return;}
        static QSvgRenderer renderer(QStringLiteral(":/studio/rotate.svg"));
        painter->setRenderHint(QPainter::Antialiasing);
        renderer.render(painter,rect());
    }
    void mousePressEvent(QGraphicsSceneMouseEvent* e)override {
        owner_->setSelected(true);center_=owner_->mapToScene(owner_->transformOriginPoint());
        const auto d=e->scenePos()-center_;distance_=qMax(1.,std::hypot(d.x(),d.y()));angle_=std::atan2(d.y(),d.x());scale_=owner_->scale();rotation_=owner_->rotation();e->accept();
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e)override {
        const auto d=e->scenePos()-center_;
        if(rotate_)owner_->setRotation(std::remainder(rotation_+(std::atan2(d.y(),d.x())-angle_)*180./3.141592653589793,360.));
        else owner_->setScale(qBound(.1,scale_*std::hypot(d.x(),d.y())/distance_,10.));
        e->accept();
    }
private:
    QGraphicsRectItem* owner_;bool rotate_;QPointF center_;double distance_=1,angle_=0,scale_=1,rotation_=0;
};

class LayoutView : public QGraphicsView {
public:
    using QGraphicsView::QGraphicsView;
protected:
    void drawBackground(QPainter* painter,const QRectF& area)override {
        QGraphicsView::drawBackground(painter,area);if(!scene()->property("snapToGrid").toBool())return;
        const int step=scene()->property("gridSpacing").toInt();if(step<1 || area.width()/step+area.height()/step>3000)return;
        painter->setPen(QPen(QColor("#283749"),0));
        for(double x=std::floor(area.left()/step)*step;x<=area.right();x+=step)painter->drawLine(QPointF(x,area.top()),QPointF(x,area.bottom()));
        for(double y=std::floor(area.top()/step)*step;y<=area.bottom();y+=step)painter->drawLine(QPointF(area.left(),y),QPointF(area.right(),y));
    }
    void wheelEvent(QWheelEvent* e)override {
        if(e->angleDelta().x()!=0 || (e->modifiers()&Qt::ShiftModifier)){QGraphicsView::wheelEvent(e);return;}
        const double factor=e->angleDelta().y()>0?1.12:1/1.12;if(transform().m11()*factor>.1 && transform().m11()*factor<12)scale(factor,factor);e->accept();}
};

// Snap in scene coordinates, including LEDs inside rotated/scaled components.
inline QVariant snappedLayoutPosition(QGraphicsItem* item,const QVariant& value) {
    if(!item->scene() || !item->scene()->property("snapToGrid").toBool())return value;
    const int step=item->scene()->property("gridSpacing").toInt();if(step<1)return value;
    auto point=item->parentItem()?item->parentItem()->mapToScene(value.toPointF()):value.toPointF();
    point=QPointF(std::round(point.x()/step)*step,std::round(point.y()/step)*step);
    return item->parentItem()?item->parentItem()->mapFromScene(point):point;
}
class LedPoint : public QGraphicsEllipseItem {public:using QGraphicsEllipseItem::QGraphicsEllipseItem;
protected:QVariant itemChange(GraphicsItemChange change,const QVariant& value)override {return QGraphicsEllipseItem::itemChange(change,change==ItemPositionChange?snappedLayoutPosition(this,value):value);}};
class LayoutComponentItem : public QGraphicsRectItem {public:using QGraphicsRectItem::QGraphicsRectItem;
protected:QVariant itemChange(GraphicsItemChange change,const QVariant& value)override {return QGraphicsRectItem::itemChange(change,change==ItemPositionChange?snappedLayoutPosition(this,value):value);}};

// Coordinates are baked back only on Save; cancelling never changes wire order,
// overrides, or the live scene. LED positions, not image decoration, drive effects.
class LayoutEditor : public QDialog {
public:
    explicit LayoutEditor(std::vector<Component> components,QWidget* parent=nullptr):QDialog(parent),components_(std::move(components)) {
        setWindowTitle("Component and LED layout");resize(920,680);setWindowState(windowState()|Qt::WindowMaximized);auto* root=new QVBoxLayout(this);
        auto* help=new QLabel("Drag component frames to move them. Corner handle: resize proportionally; rotation arrow: rotate. Drag fan/strip LEDs individually. Wheel: zoom.",this);help->setWordWrap(true);root->addWidget(help);
        auto* bar=new QHBoxLayout;root->addLayout(bar);selection_=new QComboBox(this);selection_->setObjectName("layoutComponent");
        for(const auto& c:components_)selection_->addItem(QString::fromStdString(c.displayName));bar->addWidget(selection_,1);
        auto* shapes=new QComboBox(this);shapes->setObjectName("layoutShape");shapes->addItems({"Line","Circle","Ellipse","Rectangle"});bar->addWidget(shapes);
        auto* arrange=new QPushButton("Apply shape",this);bar->addWidget(arrange);
        auto* reset=new QPushButton("Reset layout",this);reset->setObjectName("resetComponentLayout");bar->addWidget(reset);
        scene_=new QGraphicsScene(this);view_=new LayoutView(scene_,this);view_->setObjectName("componentLayoutView");view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);view_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);view_->setRenderHint(QPainter::Antialiasing);view_->setBackgroundBrush(QColor("#10161e"));view_->setDragMode(QGraphicsView::RubberBandDrag);root->addWidget(view_,1);
        auto* controls=new QHBoxLayout;root->addLayout(controls);
        auto* snap=new QCheckBox("Snap to grid",this);snap->setObjectName("layoutSnapToGrid");controls->addWidget(snap);
        auto* spacing=new QSpinBox(this);spacing->setObjectName("layoutGridSpacing");spacing->setRange(1,100);spacing->setValue(10);spacing->setSuffix(" units");controls->addWidget(spacing);scene_->setProperty("gridSpacing",10);
        connect(snap,&QCheckBox::toggled,this,[this](bool on){scene_->setProperty("snapToGrid",on);view_->viewport()->update();});
        connect(spacing,&QSpinBox::valueChanged,this,[this](int value){scene_->setProperty("gridSpacing",value);view_->viewport()->update();});
        auto* size=new QDoubleSpinBox(this);size->setObjectName("componentScale");size->setRange(.1,10);size->setSingleStep(.1);size->setValue(1);controls->addWidget(new QLabel("Component scale",this));controls->addWidget(size);
        auto* rotation=new QDoubleSpinBox(this);rotation->setObjectName("componentRotation");rotation->setRange(-360,360);rotation->setSuffix(QString::fromUtf8("\xc2\xb0"));controls->addWidget(new QLabel("Rotation",this));controls->addWidget(rotation);
        auto* fit=new QPushButton("Fit view",this);controls->addWidget(fit);
        auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,this);root->addWidget(buttons);
        rebuild();
        connect(arrange,&QPushButton::clicked,this,[this,shapes]{capture();const int index=selection_->currentIndex();if(index>=0 && components_[index].type!="Keyboard" && components_[index].type!="KeyboardZones"){components_[index].arrange(shapes->currentText().toStdString());rebuild();}});
        connect(reset,&QPushButton::clicked,this,[this]{capture();const int index=selection_->currentIndex();if(index>=0){components_[index].resetLayout();rebuild();}});
        connect(size,&QDoubleSpinBox::valueChanged,this,[this](double v){const int i=selection_->currentIndex();if(i>=0 && i<int(bodies_.size()))bodies_[i]->setScale(v);});
        connect(rotation,&QDoubleSpinBox::valueChanged,this,[this](double v){const int i=selection_->currentIndex();if(i>=0 && i<int(bodies_.size()))bodies_[i]->setRotation(v);});
        // Dragging the corner/rotation handles changes scale/rotation directly on the
        // QGraphicsItem, bypassing the spinboxes; mirror it back so the readout never
        // goes stale, and re-sync on selection change since each component has its own.
        const auto syncSpinboxes=[this,size,rotation]{
            const int i=selection_->currentIndex();if(i<0 || i>=int(bodies_.size()))return;
            const QSignalBlocker b1(size),b2(rotation);size->setValue(bodies_[i]->scale());rotation->setValue(bodies_[i]->rotation());
        };
        connect(scene_,&QGraphicsScene::changed,this,syncSpinboxes);
        // The initial explicit scene rect must follow moved/scaled items, or
        // scrollbars cannot reach components dragged beyond the original bounds.
        connect(scene_,&QGraphicsScene::changed,this,[this]{
            const auto bounds=scene_->itemsBoundingRect().adjusted(-80,-80,80,80);
            if(scene_->sceneRect()!=bounds)scene_->setSceneRect(bounds);
        });
        connect(selection_,&QComboBox::currentIndexChanged,this,syncSpinboxes);
        connect(fit,&QPushButton::clicked,this,[this]{view_->fitInView(componentBounds().adjusted(-30,-30,30,30),Qt::KeepAspectRatio);});
        connect(buttons,&QDialogButtonBox::accepted,this,[this]{capture();accept();});connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    }
    const std::vector<Component>& components()const{return components_;}
protected:
    void showEvent(QShowEvent* event)override {
        QDialog::showEvent(event);
        // Fit after the dialog receives its actual viewport size, not constructor geometry.
        QTimer::singleShot(0,this,[this]{view_->fitInView(componentBounds().adjusted(-30,-30,30,30),Qt::KeepAspectRatio);});
    }
private:
    // itemsBoundingRect() also sweeps up the fixed-pixel-size handle/label
    // children (ItemIgnoresTransformations): their reported scene bounds grow
    // or shrink with the parent's scale/rotation even though they always draw
    // at the same screen size, so a scaled or rotated component threw the fit
    // off. The component frames carry the real, correctly transformed geometry.
    QRectF componentBounds()const{
        QRectF bounds;for(auto* body:bodies_)bounds=bounds.united(body->sceneBoundingRect());return bounds;
    }
    void capture() {
        for(size_t c=0;c<components_.size();++c)for(int i=0;i<components_[c].ledCount;++i)if(points_[c][i]) {
            const auto p=points_[c][i]->scenePos();components_[c].ledCoordinates[i]={qBound(-100000,qRound(p.x()),100000),qBound(-100000,qRound(p.y()),100000)};
        }
    }
    void rebuild() {
        scene_->clear();bodies_.clear();points_.clear();
        for(auto& c:components_) {
            if(c.layoutDefaults.empty())c.layoutDefaults=c.ledCoordinates;
            int left=c.ledCoordinates[0].first,top=c.ledCoordinates[0].second,right=left,bottom=top;
            for(int i=0;i<c.ledCount;++i)if(c.ledNames[i].find("Wire slot")!=0){left=qMin(left,c.ledCoordinates[i].first);right=qMax(right,c.ledCoordinates[i].first);top=qMin(top,c.ledCoordinates[i].second);bottom=qMax(bottom,c.ledCoordinates[i].second);}
            // Native keyboard matrices are in key units; magnify their editor view
            // through the item transform, without changing saved wire coordinates.
            const bool keyboard=c.type=="Keyboard" || c.type=="KeyboardZones";
            const double radius=keyboard?.35:3.5,padding=keyboard?1.:12.;
            auto* body=new LayoutComponentItem(QRectF(left-padding,top-padding,qMax(1,right-left)+padding*2,qMax(1,bottom-top)+padding*2));
            body->setPen(QPen(QColor("#536b80"),0));body->setBrush(QColor(30,43,58,90));body->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemSendsGeometryChanges);body->setTransformOriginPoint(body->rect().center());scene_->addItem(body);bodies_.push_back(body);
            auto* title=new QGraphicsSimpleTextItem(QFontMetrics(font()).elidedText(compactHardwareName(c),Qt::ElideRight,qMax(40,int(body->rect().width()))),body);title->setToolTip(QString::fromStdString(c.displayName));title->setBrush(QColor("#a9b4c3"));title->setPos(left,top-padding-18);
            std::vector<QGraphicsEllipseItem*> points(c.ledCount,nullptr);
            for(int i=0;i<c.ledCount;++i) {
                if(c.ledNames[i].find("Wire slot")==0)continue;
                auto* point=new LedPoint(-radius,-radius,radius*2,radius*2,body);point->setBrush(QColor("#71e2c5"));point->setPen(QPen(QColor("#e8f4ff"),0));point->setPos(c.ledCoordinates[i].first,c.ledCoordinates[i].second);
                point->setData(0,"LED");point->setData(1,i);point->setToolTip(QString::fromStdString(c.ledNames[i])+QString(" / wire address %1").arg(c.ledMapping[i]));
                if(!keyboard)point->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemSendsGeometryChanges);
                points[i]=point;
            }
            points_.push_back(std::move(points));new ComponentTransformHandle(body,false);new ComponentTransformHandle(body,true);
        }
        scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-80,-80,80,80));view_->fitInView(componentBounds().adjusted(-30,-30,30,30),Qt::KeepAspectRatio);
    }
    std::vector<Component> components_;QGraphicsScene* scene_;LayoutView* view_;QComboBox* selection_;
    std::vector<QGraphicsRectItem*> bodies_;std::vector<std::vector<QGraphicsEllipseItem*>> points_;
};
