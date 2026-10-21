#pragma once
#include "device/Component.h"
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
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(QColor("#e4ecf7"),1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        painter->setBrush(brush());painter->drawEllipse(rect());painter->setBrush(Qt::NoBrush);
        painter->drawArc(QRectF(-5,-5,10,10),30*16,285*16);
        painter->drawPolyline(QPolygonF{QPointF(1,-6),QPointF(5,-5),QPointF(4,-1)});
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
    void wheelEvent(QWheelEvent* e)override {const double factor=e->angleDelta().y()>0?1.12:1/1.12;if(transform().m11()*factor>.1 && transform().m11()*factor<12)scale(factor,factor);e->accept();}
};

// LEDs grouped via "Group selected LEDs" share one physical sample point:
// dragging any member moves the rest of the group by the same delta. The
// syncing flag lives on the shared group, not per item: a per-item flag only
// guards that item's own propagation loop, so the member being dragged by
// the loop would re-enter and apply the delta to the dragged item a second
// time once its own itemChange fired.
class LedPoint;
struct LedGroup{std::vector<LedPoint*> members;bool syncing=false;};
class LedPoint : public QGraphicsEllipseItem {
public:
    using QGraphicsEllipseItem::QGraphicsEllipseItem;
    std::shared_ptr<LedGroup> group;
    QPointF lastPos;
protected:
    QVariant itemChange(GraphicsItemChange change,const QVariant& value)override {
        if(change==ItemPositionHasChanged) {
            const auto newPos=value.toPointF();
            if(group && !group->syncing) {
                group->syncing=true;const auto delta=newPos-lastPos;
                for(auto* other:group->members)if(other!=this)other->setPos(other->pos()+delta);
                group->syncing=false;
            }
            lastPos=newPos;
        }
        return QGraphicsEllipseItem::itemChange(change,value);
    }
};

// Coordinates are baked back only on Save; cancelling never changes wire order,
// overrides, or the live scene. LED positions, not image decoration, drive effects.
class LayoutEditor : public QDialog {
public:
    explicit LayoutEditor(std::vector<Component> components,QWidget* parent=nullptr):QDialog(parent),components_(std::move(components)) {
        setWindowTitle("Component and LED layout");resize(920,680);auto* root=new QVBoxLayout(this);
        auto* help=new QLabel("Drag component frames to move them. Corner handle: resize proportionally; rotation arrow: rotate. Drag fan/strip LEDs individually. Wheel: zoom.",this);help->setWordWrap(true);root->addWidget(help);
        auto* bar=new QHBoxLayout;root->addLayout(bar);selection_=new QComboBox(this);selection_->setObjectName("layoutComponent");
        for(const auto& c:components_)selection_->addItem(QString::fromStdString(c.displayName));bar->addWidget(selection_,1);
        auto* shapes=new QComboBox(this);shapes->setObjectName("layoutShape");shapes->addItems({"Line","Circle","Ellipse","Rectangle"});bar->addWidget(shapes);
        auto* arrange=new QPushButton("Apply shape",this);bar->addWidget(arrange);
        auto* reset=new QPushButton("Reset layout",this);reset->setObjectName("resetComponentLayout");bar->addWidget(reset);
        auto* group=new QPushButton("Group selected LEDs",this);bar->addWidget(group);
        scene_=new QGraphicsScene(this);view_=new LayoutView(scene_,this);view_->setObjectName("componentLayoutView");view_->setRenderHint(QPainter::Antialiasing);view_->setBackgroundBrush(QColor("#10161e"));view_->setDragMode(QGraphicsView::RubberBandDrag);root->addWidget(view_,1);
        auto* controls=new QHBoxLayout;root->addLayout(controls);
        auto* size=new QDoubleSpinBox(this);size->setObjectName("componentScale");size->setRange(.1,10);size->setSingleStep(.1);size->setValue(1);controls->addWidget(new QLabel("Component scale",this));controls->addWidget(size);
        auto* rotation=new QDoubleSpinBox(this);rotation->setObjectName("componentRotation");rotation->setRange(-360,360);rotation->setSuffix(" deg");controls->addWidget(new QLabel("Rotation",this));controls->addWidget(rotation);
        auto* fit=new QPushButton("Fit view",this);controls->addWidget(fit);
        auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,this);root->addWidget(buttons);
        rebuild();
        connect(arrange,&QPushButton::clicked,this,[this,shapes]{capture();const int index=selection_->currentIndex();if(index>=0 && components_[index].type!="Keyboard" && components_[index].type!="KeyboardZones"){components_[index].arrange(shapes->currentText().toStdString());rebuild();}});
        connect(reset,&QPushButton::clicked,this,[this]{capture();const int index=selection_->currentIndex();if(index>=0){components_[index].resetLayout();rebuild();}});
        connect(group,&QPushButton::clicked,this,[this]{
            QList<LedPoint*> selected;
            for(auto* item:scene_->selectedItems())if(auto* led=dynamic_cast<LedPoint*>(item))selected.push_back(led);
            if(selected.size()<2)return;
            const auto target=selected.first()->pos();
            for(auto* led:selected)led->setPos(target);
            auto grp=std::make_shared<LedGroup>();grp->members.assign(selected.begin(),selected.end());
            for(auto* led:selected){led->group=grp;led->lastPos=led->pos();}
        });
        connect(size,&QDoubleSpinBox::valueChanged,this,[this](double v){const int i=selection_->currentIndex();if(i>=0 && i<int(bodies_.size()))bodies_[i]->setScale(v);});
        connect(rotation,&QDoubleSpinBox::valueChanged,this,[this](double v){const int i=selection_->currentIndex();if(i>=0 && i<int(bodies_.size()))bodies_[i]->setRotation(v);});
        connect(fit,&QPushButton::clicked,this,[this]{view_->fitInView(scene_->itemsBoundingRect().adjusted(-30,-30,30,30),Qt::KeepAspectRatio);});
        connect(buttons,&QDialogButtonBox::accepted,this,[this]{capture();accept();});connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    }
    const std::vector<Component>& components()const{return components_;}
private:
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
            auto* body=new QGraphicsRectItem(QRectF(left-padding,top-padding,qMax(1,right-left)+padding*2,qMax(1,bottom-top)+padding*2));
            body->setPen(QPen(QColor("#536b80"),0));body->setBrush(QColor(30,43,58,90));body->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);body->setTransformOriginPoint(body->rect().center());scene_->addItem(body);bodies_.push_back(body);
            auto* title=new QGraphicsSimpleTextItem(QString::fromStdString(c.displayName),body);title->setBrush(QColor("#a9b4c3"));title->setFlag(QGraphicsItem::ItemIgnoresTransformations);title->setPos(left,top-padding-18);
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
        scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-80,-80,80,80));view_->fitInView(scene_->sceneRect(),Qt::KeepAspectRatio);
    }
    std::vector<Component> components_;QGraphicsScene* scene_;LayoutView* view_;QComboBox* selection_;
    std::vector<QGraphicsRectItem*> bodies_;std::vector<std::vector<QGraphicsEllipseItem*>> points_;
};
