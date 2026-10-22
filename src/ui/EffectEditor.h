#include "effects/EffectClock.h"
#pragma once
#include "effects/CustomGradientEffect.h"
#include "effects/EffectFactory.h"
#include "ui/ColorPicker.h"
#include "ui/StudioControls.h"
#include "ui/LightingCanvasWidget.h"
#include <QListWidget>
#include <QDrag>
#include <QMimeData>
#include <QDropEvent>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QKeyEvent>
#include <QSplitter>
#include <QComboBox>
#include <QCheckBox>
#include <QScrollArea>
#include <array>
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QWheelEvent>
#include <QApplication>
#include <map>

class PatternLibrary : public QListWidget {
public:
    explicit PatternLibrary(QWidget* parent=nullptr):QListWidget(parent) {
        addItems({"Scroll","Color source","Radial","Angular","Spiral","Plasma","Breathing","Comet","Marquee","Static gradient"});
        for(int i=0;i<count();++i){auto* entry=item(i);entry->setData(Qt::UserRole,entry->text());entry->setText(QString::fromStdString(effectDisplayName(entry->text().toStdString())));}
        setDragEnabled(true);setWordWrap(true);setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);setSpacing(5);setFixedWidth(185);setAccessibleName("Drag a pattern onto the preview");
    }
protected:
    void startDrag(Qt::DropActions) override {
        if(!currentItem())return;auto* mime=new QMimeData;mime->setData("application/x-lighting-pattern",currentItem()->data(Qt::UserRole).toString().toUtf8());
        QDrag drag(this);drag.setMimeData(mime);drag.exec(Qt::CopyAction);
    }
};

class EffectPreview : public QWidget {
public:
    CustomEffectDefinition* recipe;
    std::function<void(QString)> patternDropped;
    std::function<void()> beforePaint;
    std::function<void(int,int)> centerDragged;
    QString caption;
    bool drawZoneMode=false;
    std::function<void(QRect)> zoneDrawn;
    std::function<void()> zoneDrawFinished;
    QRectF sceneViewport()const {const double scale=qMin(width()/1100.,height()/700.);const QSizeF size(1100*scale,700*scale);return QRectF(QPointF((width()-size.width())/2,(height()-size.height())/2),size);}

    void setPreviewPaused(bool paused) {
        if(paused && !paused_)pausedTime_=effectTimeMs();
        paused_=paused;
        if(paused)timer_.stop();else if(isVisible())timer_.start();
        update();
    }
    bool previewPaused()const{return paused_;}
    std::shared_ptr<IEffect> renderedEffect;
    explicit EffectPreview(CustomEffectDefinition* d,QWidget* parent):QWidget(parent),recipe(d) {
        setAcceptDrops(true);setMinimumSize(380,200);(void)effectTimeMs();
        timer_.setTimerType(Qt::PreciseTimer);timer_.setInterval(33);
        connect(&timer_,&QTimer::timeout,this,QOverload<>::of(&QWidget::update));timer_.start();
    }
protected:
    void showEvent(QShowEvent* event)override {QWidget::showEvent(event);if(!paused_)timer_.start();}
    void hideEvent(QHideEvent* event)override {timer_.stop();QWidget::hideEvent(event);}
    void paintEvent(QPaintEvent*) override {
        if(beforePaint)beforePaint();
        QPainter p(this);p.setRenderHint(QPainter::SmoothPixmapTransform);
        if(recipe->path!="Canvas positions") {
            p.fillRect(rect(),QColor("#10161e"));CustomGradientEffect effect(*recipe);const auto time=paused_?pausedTime_:effectTimeMs();
            for(int fan=0;fan<2;++fan){const QPointF center(width()*(fan?.72:.28),height()*.5);const double radius=qMin(width()*.17,height()*.30);p.setPen(QPen(QColor("#3d5168"),2));p.setBrush(QColor("#1c2734"));p.drawEllipse(center,radius+14,radius+14);
                for(int led=0;led<12;++led){const double angle=led*6.283185307/12-1.570796327;const auto c=effect.colorForLed(0,0,time,fan*12+led,24,led,12);p.setPen(Qt::NoPen);p.setBrush(QColor(c.r,c.g,c.b));p.drawEllipse(center+QPointF(std::cos(angle)*radius,std::sin(angle)*radius),7,7);}
                p.setPen(QColor("#d7e1ec"));p.drawText(QRectF(center.x()-radius,center.y()-15,radius*2,30),Qt::AlignCenter,QString("Component %1").arg(fan+1));}
            p.setPen(QColor("#a9b4c3"));p.drawText(rect().adjusted(14,12,-14,-12),Qt::AlignBottom|Qt::AlignLeft,"12 individual LED addresses per component");return;
        }
        QImage image(220,140,QImage::Format_RGB32);CustomGradientEffect effect(*recipe);const auto time=paused_?pausedTime_:effectTimeMs();
        for(int y=0;y<140;++y){auto* row=reinterpret_cast<QRgb*>(image.scanLine(y));for(int x=0;x<220;++x){auto c=renderedEffect?renderedEffect->colorAt(x*5,y*5,time):effect.colorAt(x*5,y*5,time);row[x]=qRgb(c.r,c.g,c.b);}}
        p.fillRect(rect(),QColor("#10161e"));const auto viewport=sceneViewport();p.drawImage(viewport,image);
        if(centerDragged && (recipe->pattern=="Radial" || recipe->pattern=="Angular" || recipe->pattern=="Spiral")){const QPointF center(viewport.left()+recipe->centerX*viewport.width()/1100.,viewport.top()+recipe->centerY*viewport.height()/700.);p.setPen(QPen(Qt::black,4));p.drawEllipse(center,7,7);p.setPen(QPen(Qt::white,2));p.drawEllipse(center,7,7);p.drawLine(center-QPointF(12,0),center+QPointF(12,0));p.drawLine(center-QPointF(0,12),center+QPointF(0,12));}
        if(zoneDrawn && recipe->zoneEnabled){const QRectF zone(viewport.left()+recipe->zoneX*viewport.width()/1100.,viewport.top()+recipe->zoneY*viewport.height()/700.,recipe->zoneWidth*viewport.width()/1100.,recipe->zoneHeight*viewport.height()/700.);p.setPen(QPen(QColor("#a8f3de"),2,Qt::DashLine));p.setBrush(QColor(113,226,197,24));p.drawRect(zone);}
        p.setPen(Qt::white);p.drawText(rect().adjusted(14,12,-14,-12),Qt::AlignTop|Qt::AlignLeft,(!caption.isEmpty()?caption:recipe->layers.empty()?QString::fromStdString(recipe->pattern):QString("Combined preview / %1 layers").arg(recipe->layers.size()+1)));
    }
    void mousePressEvent(QMouseEvent* e)override {
        if(e->button()!=Qt::LeftButton || !sceneViewport().contains(e->position()))return;
        if(drawZoneMode && zoneDrawn){zoneAnchor_=scenePoint(e->position());drawingZone_=true;e->accept();return;}
        if(centerDragged && (recipe->pattern=="Radial" || recipe->pattern=="Angular" || recipe->pattern=="Spiral")){draggingCenter_=true;moveCenter(e->position());e->accept();}
    }
    void mouseMoveEvent(QMouseEvent* e)override {if(drawingZone_)moveZone(e->position());else if(draggingCenter_)moveCenter(e->position());}
    void mouseReleaseEvent(QMouseEvent* e)override {if(drawingZone_){moveZone(e->position());drawingZone_=false;drawZoneMode=false;unsetCursor();if(zoneDrawFinished)zoneDrawFinished();}draggingCenter_=false;}
    void dragEnterEvent(QDragEnterEvent* e) override {if(e->mimeData()->hasFormat("application/x-lighting-pattern"))e->acceptProposedAction();}
    void dropEvent(QDropEvent* e) override {
        const auto pattern=QString::fromUtf8(e->mimeData()->data("application/x-lighting-pattern"));
        if(CustomEffectDefinition::validPattern(pattern.toStdString())){if(patternDropped)patternDropped(pattern);else recipe->pattern=pattern.toStdString();update();e->acceptProposedAction();}
    }
private:
    bool paused_=false;
    int64_t pausedTime_=0;
    bool draggingCenter_=false,drawingZone_=false;
    QPoint zoneAnchor_;
    QPoint scenePoint(QPointF p)const {const auto viewport=sceneViewport();return {qBound(0,qRound((p.x()-viewport.left())*1100/viewport.width()),1100),qBound(0,qRound((p.y()-viewport.top())*700/viewport.height()),700)};}
    void moveZone(QPointF p){const auto end=scenePoint(p);const QRect zone(QPoint(qMin(zoneAnchor_.x(),end.x()),qMin(zoneAnchor_.y(),end.y())),QSize(std::abs(end.x()-zoneAnchor_.x()),std::abs(end.y()-zoneAnchor_.y())));if(zone.width()>=4 && zone.height()>=4)zoneDrawn(zone);update();}

    void moveCenter(QPointF p){const auto viewport=sceneViewport();centerDragged(qBound(0,qRound((p.x()-viewport.left())*1100/viewport.width()),1100),qBound(0,qRound((p.y()-viewport.top())*700/viewport.height()),700));update();}
    QTimer timer_;
};

class GradientStops : public QWidget {
public:
    CustomEffectDefinition* recipe;
    int selected=0;
    std::function<void()> changed;
    explicit GradientStops(CustomEffectDefinition* d,QWidget* parent):QWidget(parent),recipe(d) {
        setObjectName("gradientStops");setMinimumHeight(74);setFocusPolicy(Qt::StrongFocus);
        setAccessibleName("Gradient stops: drag to position, double click to add or recolor, Delete to remove");
        setToolTip(accessibleName());
        if(recipe->stops.empty())for(size_t i=0;i<recipe->colors.size();++i)recipe->stops.push_back(i/double(recipe->colors.size()));
    }
    void addStop(double position) {
        if(recipe->colors.size()>=16)return;
        position=std::clamp(position,.005,.995);
        for(double stop:recipe->stops)if(std::abs(stop-position)<.005)return;
        // A palette edit samples the palette, independent of the layer's spatial mask.
        auto palette=*recipe;palette.pattern="Static gradient";palette.zoneEnabled=false;palette.enabled=true;palette.layers.clear();
        const auto color=CustomGradientEffect(palette).colorAt(int(position*recipe->wavelength),0,0);
        const auto i=std::lower_bound(recipe->stops.begin(),recipe->stops.end(),position)-recipe->stops.begin();
        recipe->stops.insert(recipe->stops.begin()+i,position);recipe->colors.insert(recipe->colors.begin()+i,color);selected=int(i);update();if(changed)changed();
    }
    void removeStop(){if(recipe->colors.size()<=2)return;recipe->colors.erase(recipe->colors.begin()+selected);recipe->stops.erase(recipe->stops.begin()+selected);selected=qMin(selected,int(recipe->colors.size())-1);update();if(changed)changed();}
    void editStop(){const auto c=recipe->colors[selected];const QColor chosen=ColorPicker::getColor(QColor(c.r,c.g,c.b),this,"Gradient stop color");if(chosen.isValid()){recipe->colors[selected]={uint8_t(chosen.red()),uint8_t(chosen.green()),uint8_t(chosen.blue())};update();if(changed)changed();}}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        auto palette=*recipe;palette.pattern="Static gradient";palette.zoneEnabled=false;palette.enabled=true;palette.layers.clear();CustomGradientEffect gradient(palette);
        QImage image(qMax(1,width()-24),1,QImage::Format_RGB32);
        for(int x=0;x<image.width();++x){const auto c=gradient.colorAt(int(x*palette.wavelength/double(image.width())),0,0);image.setPixel(x,0,qRgb(c.r,c.g,c.b));}
        p.drawImage(QRect(12,10,width()-24,32),image);
        for(size_t i=0;i<recipe->stops.size();++i){const auto c=recipe->colors[i];p.setPen(QPen(i==selected?Qt::white:Qt::gray,i==selected?3:1));p.setBrush(QColor(c.r,c.g,c.b));p.drawEllipse(QPointF(12+recipe->stops[i]*(width()-24),45),7,9);}
    }
    int hit(QPointF point)const{for(int i=0;i<int(recipe->stops.size());++i)if(std::abs(point.x()-(12+recipe->stops[i]*(width()-24)))<11)return i;return -1;}
    void mousePressEvent(QMouseEvent* e)override{int i=hit(e->position());if(i>=0){selected=i;dragging_=true;update();if(changed)changed();}}
    void mouseReleaseEvent(QMouseEvent*)override{dragging_=false;}
    void mouseMoveEvent(QMouseEvent* e)override{
        if(!dragging_ || !(e->buttons()&Qt::LeftButton))return;
        const double low=selected?recipe->stops[selected-1]+.005:0.;
        const double high=selected+1<int(recipe->stops.size())?recipe->stops[selected+1]-.005:.995;
        recipe->stops[selected]=std::clamp((e->position().x()-12)/qMax(1,width()-24),low,high);update();
    }
    void mouseDoubleClickEvent(QMouseEvent* e)override{int i=hit(e->position());if(i>=0){selected=i;editStop();}else addStop((e->position().x()-12)/qMax(1,width()-24));}
    void keyPressEvent(QKeyEvent* e)override{if(e->key()==Qt::Key_Delete)removeStop();else if(e->key()==Qt::Key_Return)editStop();else if(e->key()==Qt::Key_Left || e->key()==Qt::Key_Right){selected=qBound(0,selected+(e->key()==Qt::Key_Left?-1:1),int(recipe->colors.size())-1);update();}else QWidget::keyPressEvent(e);}
private:
    bool dragging_=false;
};

class EffectEditor : public QDialog {
public:
    std::function<bool(const CustomEffectDefinition&)> saveEffect;
    std::string protectedBaseName;
    explicit EffectEditor(CustomEffectDefinition recipe,QWidget* parent=nullptr,bool allowLayers=true):QDialog(parent),recipe_(std::move(recipe)) {
        recipe_.normalizeScroll();editing_=recipe_;editing_.layers.clear();
        setWindowTitle("Effect studio");setStyleSheet(compactStudioControlsStyle());resize(1120,800);auto* outer=new QVBoxLayout(this);
        name_=new QLineEdit(QString::fromStdString(recipe_.name),this);name_->setObjectName("effectRecipeName");outer->addWidget(name_);
        auto* split=new QSplitter(this);outer->addWidget(split,1);
        auto* left=new QWidget;auto* leftLayout=new QVBoxLayout(left);leftLayout->setContentsMargins(0,0,8,0);split->addWidget(left);
        auto* layerHeader=new QHBoxLayout;leftLayout->addLayout(layerHeader);layerHeader->addWidget(new QLabel("LAYERS / bottom to top",left));layerHeader->addStretch();
        auto* pause=new QPushButton("Pause preview",left);pause->setObjectName("pauseEffectPreview");pause->setCheckable(true);leftLayout->addWidget(pause);
        stack_=new QListWidget(left);stack_->setObjectName("effectLayers");stack_->setMinimumWidth(185);stack_->setDragDropMode(QAbstractItemView::InternalMove);leftLayout->addWidget(stack_,1);
        auto* actions=new QHBoxLayout;leftLayout->addLayout(actions);auto* add=new QPushButton("+ Add layer",left);auto* remove=new QPushButton("Remove",left);actions->addWidget(add);actions->addWidget(remove);actions->addStretch();add->setEnabled(allowLayers);remove->setEnabled(allowLayers);
        auto* right=new QWidget;auto* rightLayout=new QVBoxLayout(right);rightLayout->setContentsMargins(0,0,0,0);split->addWidget(right);
        auto* previews=new QHBoxLayout;rightLayout->addLayout(previews);
        auto* combinedColumn=new QVBoxLayout;previews->addLayout(combinedColumn,1);combinedColumn->addWidget(new QLabel("COMBINED OUTPUT / all checked layers",right));
        preview_=new EffectPreview(&recipe_,right);preview_->setObjectName("effectPreview");preview_->setMinimumSize(220,190);preview_->setMaximumHeight(240);combinedColumn->addWidget(preview_);
        auto* selectedColumn=new QVBoxLayout;previews->addLayout(selectedColumn,1);soloTitle_=new QLabel(right);selectedColumn->addWidget(soloTitle_);
        auto* solo=new EffectPreview(&editing_,right);solo_=solo;solo->setObjectName("selectedLayerPreview");solo->setMinimumSize(220,190);solo->setMaximumHeight(240);selectedColumn->addWidget(solo);
        preview_->beforePaint=[this]{commit();};
        // Reuse the parent's preview preference without altering hardware timing.
        bool paused=false;if(parent)if(auto* canvas=parent->findChild<LightingCanvasWidget*>())paused=canvas->previewPaused();
        preview_->setPreviewPaused(paused);solo_->setPreviewPaused(paused);pause->setChecked(paused);pause->setText(paused?"Resume preview":"Pause preview");
        connect(pause,&QPushButton::toggled,this,[this,pause](bool on){preview_->setPreviewPaused(on);solo_->setPreviewPaused(on);pause->setText(on?"Resume preview":"Pause preview");});
        solo_->centerDragged=[this](int x,int y){editing_.centerX=x;editing_.centerY=y;commit();solo_->update();preview_->update();};
        solo_->setToolTip("For concentric rings, color wheel and spiral: drag anywhere to place the center.");
        auto* inspectorScroll=new QScrollArea(right);inspectorScroll_=inspectorScroll;inspectorScroll->setWidgetResizable(true);rightLayout->addWidget(inspectorScroll,1);auto* inspector=new QWidget;inspectorScroll->setWidget(inspector);auto* root=new QVBoxLayout(inspector);root->setContentsMargins(8,6,8,6);root->setSpacing(5);
        selectedTitle_=new QLabel(inspector);root->addWidget(selectedTitle_);
        auto* form=new QFormLayout;form->setVerticalSpacing(5);form->setHorizontalSpacing(10);form_=form;root->addLayout(form);
        pattern_=new QComboBox(inspector);pattern_->setObjectName("patternLibrary");pattern_->addItems({"Scroll","Breathing","Color source","Radial","Angular","Spiral","Plasma","Comet","Marquee","Static gradient"});for(int i=0;i<pattern_->count();++i){const auto id=pattern_->itemText(i);pattern_->setItemData(i,id);pattern_->setItemText(i,QString::fromStdString(effectDisplayName(id.toStdString())));}form->addRow("Effect",pattern_);
        auto* layerName=new QLineEdit(inspector);layerName->setObjectName("layerName");layerName->setMaxLength(80);form->insertRow(0,"Layer name",layerName);
        connect(layerName,&QLineEdit::textEdited,this,[this](QString name){if(selected_<1)return;editing_.name=name.trimmed().toStdString();const QSignalBlocker b(stack_);stack_->item(selected_)->setText(name);selectedTitle_->setText(name);updateParameters();});
        sync_.push_back([this,layerName]{const QSignalBlocker b(layerName);layerName->setText(selected_?QString::fromStdString(editing_.name):"Base");layerName->setEnabled(selected_>0);});
        connect(pattern_,&QComboBox::currentTextChanged,this,[this](QString text){editing_.pattern=pattern_->currentData().toString().toStdString();if(editing_.pattern=="Color source"){stops_->selected=0;syncColor();}updateParameters();commit();});
        auto* blend=new QComboBox(inspector);blend->setObjectName("effectBlend");blend->addItems({"Screen","Add","Normal"});form->addRow("Mix with layers below",blend);
        connect(blend,&QComboBox::currentTextChanged,this,[this](QString text){editing_.blend=text.toStdString();});
        sync_.push_back([this,blend]{const QSignalBlocker b(blend);blend->setCurrentText(QString::fromStdString(editing_.blend));form_->setRowVisible(blend,selected_>0);});
        auto* opacity=new QDoubleSpinBox(inspector);opacity->setObjectName("effectOpacity");opacity->setRange(0,1);opacity->setSingleStep(.05);form->addRow("Opacity",opacity);
        connect(opacity,&QDoubleSpinBox::valueChanged,this,[this](double value){editing_.opacity=value;});sync_.push_back([this,opacity]{const QSignalBlocker b(opacity);opacity->setValue(editing_.opacity);form_->setRowVisible(sliderRows_.count(opacity)?sliderRows_[opacity]:opacity,selected_>0);});
        auto* palette=new QComboBox(inspector);palette_=palette;palette->setObjectName("effectPalette");palette->addItems({"Custom colors","Aurora","Ocean","Ember","Pastel","Polar","Sunset"});form->addRow("Color palette",palette);
        stops_=new GradientStops(&editing_,inspector);root->addWidget(stops_);
        auto* colorRow=new QHBoxLayout;root->addLayout(colorRow);swatch_=new QPushButton(inspector);colorRow->addWidget(swatch_,1);
        for(int i=0;i<3;++i){colorRow->addWidget(new QLabel(QString("RGB"[i]),inspector));rgb_[i]=new QSpinBox(inspector);rgb_[i]->setObjectName(QString("stop%1").arg("RGB"[i]));rgb_[i]->setRange(0,255);colorRow->addWidget(rgb_[i]);connect(rgb_[i],&QSpinBox::valueChanged,this,[this]{editing_.colors[stops_->selected]={uint8_t(rgb_[0]->value()),uint8_t(rgb_[1]->value()),uint8_t(rgb_[2]->value())};syncColor();});}
        stops_->changed=[this]{syncColor();};connect(swatch_,&QPushButton::clicked,this,[this]{stops_->editStop();});
        connect(palette,&QComboBox::currentTextChanged,this,[this](QString name){if(name=="Custom colors")return;editing_.colors=effectDefinition(name.toStdString(),{}).colors;editing_.stops.clear();for(size_t i=0;i<editing_.colors.size();++i)editing_.stops.push_back(i/double(editing_.colors.size()));stops_->selected=0;syncColor();});
        sync_.push_back([palette]{const QSignalBlocker b(palette);palette->setCurrentIndex(0);});
        auto* colors=new QHBoxLayout;colors_=colors;root->addLayout(colors);
        for(const auto* text:{"Add color","Edit color","Remove color"}){auto* button=new QPushButton(text,inspector);colors->addWidget(button);connect(button,&QPushButton::clicked,this,[this,text]{if(QString(text)=="Edit color")stops_->editStop();else if(QString(text)=="Remove color")stops_->removeStop();else {double gap=-1,pos=.5;for(size_t i=0;i<editing_.stops.size();++i){const double end=i+1<editing_.stops.size()?editing_.stops[i+1]:1.;if(end-editing_.stops[i]>gap){gap=end-editing_.stops[i];pos=editing_.stops[i]+gap/2;}}stops_->addStop(pos);}});}
        colors->addStretch();
        parameters_=new QFormLayout;parameters_->setVerticalSpacing(5);parameters_->setHorizontalSpacing(10);root->addLayout(parameters_);
        direction_=new QComboBox(inspector);direction_->setObjectName("effectDirection");direction_->addItems({"Right","Down","Left","Up","Custom angle"});parameters_->addRow("Direction",direction_);
        angle_=new QDoubleSpinBox(inspector);angle_->setObjectName("effectAngle");angle_->setRange(-360,360);angle_->setSuffix(" deg");parameters_->addRow("Angle / orientation",angle_);
        connect(direction_,&QComboBox::currentIndexChanged,this,[this](int index){if(index<4)angle_->setValue(index*90.);});
        connect(angle_,&QDoubleSpinBox::valueChanged,this,[this](double value){editing_.angle=value;const QSignalBlocker b(direction_);direction_->setCurrentIndex(std::abs(std::remainder(value,90.))<.01?(int(std::lround(value/90.))+4)%4:4);});
        motion_=new QComboBox(inspector);motion_->setObjectName("sourceMotion");motion_->addItems({"Sine","Cosine","Ping-pong","Static"});parameters_->addRow("Movement",motion_);connect(motion_,&QComboBox::currentTextChanged,this,[this](QString text){editing_.sourceMotion=text.toStdString();});
        rotation_=new QComboBox(inspector);rotation_->setObjectName("effectRotationDirection");parameters_->addRow("Rotation / travel",rotation_);connect(rotation_,&QComboBox::currentIndexChanged,this,[this](int i){editing_.reverse=i==1;});
        spread_=new QDoubleSpinBox(inspector);spread_->setObjectName("effectSpread");spread_->setRange(15,720);spread_->setSuffix(" deg");parameters_->addRow("Palette fan angle",spread_);connect(spread_,&QDoubleSpinBox::valueChanged,this,[this](double value){editing_.spreadDegrees=value;});
        size_=new QSpinBox(inspector);size_->setObjectName("effectScale");size_->setRange(40,4000);parameters_->addRow("Pattern size / spacing",size_);connect(size_,&QSpinBox::valueChanged,this,[this](int value){editing_.wavelength=value;});
        auto* durationRow=new QHBoxLayout;speed_=new QSlider(Qt::Horizontal,inspector);speed_->setObjectName("effectDuration");speed_->setRange(0,1000);speed_->setMinimumHeight(26);durationRow->addWidget(speed_,1);
        seconds_=new QDoubleSpinBox(inspector);seconds_->setObjectName("effectDurationSeconds");seconds_->setRange(.5,120);seconds_->setDecimals(3);seconds_->setSingleStep(.1);seconds_->setSuffix(" s");durationRow->addWidget(seconds_);parameters_->addRow("Cycle duration",durationRow);durationRow_=durationRow;
        connect(speed_,&QSlider::valueChanged,this,[this](int v){editing_.periodMs=int(std::lround(500.*std::pow(240.,v/1000.)));const QSignalBlocker b(seconds_);seconds_->setValue(editing_.periodMs/1000.);});
        connect(seconds_,&QDoubleSpinBox::valueChanged,this,[this](double v){editing_.periodMs=int(std::lround(v*1000));const QSignalBlocker b(speed_);speed_->setValue(durationPosition(editing_.periodMs));});
        auto* path=new QComboBox(inspector);path->setObjectName("effectPath");path->addItems({"Canvas positions"});path->setEnabled(false);path->hide(); // Layer masks and blending always use the shared scene.
        zone_=new QCheckBox("Restrict this layer to a rectangle",inspector);zone_->setObjectName("effectZone");auto* zoneTools=new QHBoxLayout;root->addLayout(zoneTools);zoneTools->addWidget(zone_,1);auto* drawZone=new QPushButton("Draw zone",inspector);drawZone->setObjectName("drawLayerZone");drawZone->setToolTip("Drag a rectangle in the selected layer preview. Drag in either direction.");drawZone->setCheckable(true);zoneTools->addWidget(drawZone);
        connect(drawZone,&QPushButton::toggled,this,[this](bool on){solo_->drawZoneMode=on;solo_->setCursor(on?Qt::CrossCursor:Qt::ArrowCursor);});
        solo_->zoneDrawn=[this](QRect rect){editing_.zoneEnabled=true;const QSignalBlocker b(zone_);zone_->setChecked(true);const int values[]={rect.x(),rect.y(),rect.width(),rect.height()};int* fields[]={&editing_.zoneX,&editing_.zoneY,&editing_.zoneWidth,&editing_.zoneHeight};for(int i=0;i<4;++i){*fields[i]=values[i];const QSignalBlocker block(zoneFields_[i]);zoneFields_[i]->setValue(values[i]);}commit();preview_->update();};solo_->zoneDrawFinished=[drawZone]{drawZone->setChecked(false);};connect(zone_,&QCheckBox::toggled,this,[this,drawZone](bool v){editing_.zoneEnabled=v;drawZone->setChecked(v);});
        auto* zoneRow=new QHBoxLayout;zoneRow->setSpacing(8);root->addLayout(zoneRow);
        for(int i=0;i<4;++i){zoneRow->addWidget(new QLabel(QStringList{"X","Y","Width","Height"}[i],inspector));zoneFields_[i]=new QSpinBox(inspector);zoneFields_[i]->setObjectName(QString("effectZone%1").arg(i));zoneFields_[i]->setRange(i<2?0:1,i%2?700:1100);zoneFields_[i]->setFixedWidth(110);zoneRow->addWidget(zoneFields_[i]);connect(zoneFields_[i],&QSpinBox::valueChanged,this,[this,i](int value){int* fields[]={&editing_.zoneX,&editing_.zoneY,&editing_.zoneWidth,&editing_.zoneHeight};*fields[i]=value;});}
        // Keep coordinate labels beside their fields instead of spreading them across the inspector.
        zoneRow->addStretch();
        for(auto* field:{static_cast<QAbstractSpinBox*>(angle_),static_cast<QAbstractSpinBox*>(spread_),static_cast<QAbstractSpinBox*>(size_),static_cast<QAbstractSpinBox*>(opacity)}){
            auto* slider=new QSlider(Qt::Horizontal,inspector);slider->setObjectName(field->objectName()+"Slider");slider->setMinimumHeight(24);
            if(auto* real=qobject_cast<QDoubleSpinBox*>(field)){const int multiplier=real==opacity?100:1;slider->setRange(qRound(real->minimum()*multiplier),qRound(real->maximum()*multiplier));slider->setValue(qRound(real->value()*multiplier));connect(slider,&QSlider::valueChanged,this,[real,multiplier](int v){real->setValue(v/double(multiplier));});connect(real,&QDoubleSpinBox::valueChanged,slider,[slider,multiplier](double v){const QSignalBlocker b(slider);slider->setValue(qRound(v*multiplier));});}
            else if(auto* integer=qobject_cast<QSpinBox*>(field)){slider->setRange(integer->minimum(),integer->maximum());connect(slider,&QSlider::valueChanged,integer,&QSpinBox::setValue);connect(integer,&QSpinBox::valueChanged,slider,&QSlider::setValue);}
            auto* layout=field==opacity?form_:parameters_;int rowIndex=0;QFormLayout::ItemRole role;layout->getWidgetPosition(field,&rowIndex,&role);const auto row=layout->takeRow(field);auto* wrapper=new QWidget(inspector);auto* controls=new QHBoxLayout(wrapper);controls->setContentsMargins(0,0,0,0);controls->addWidget(slider,1);controls->addWidget(field);field->setMaximumWidth(140);layout->insertRow(rowIndex,row.labelItem->widget(),wrapper);delete row.labelItem;delete row.fieldItem;sliderRows_[field]=wrapper;
        }
        root->addStretch();error_=new QLabel(this);error_->setObjectName("effectSaveError");outer->addWidget(error_);
        auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,this);outer->addWidget(buttons);buttons->button(QDialogButtonBox::Save)->setProperty("primary",true);
        for(auto* action:findChildren<QPushButton*>())if(action!=swatch_)sizeStudioAction(action);
        connect(buttons,&QDialogButtonBox::accepted,this,[this]{commit();recipe_.name=name_->text().trimmed().toStdString();if(!protectedBaseName.empty() && (recipe_.name==protectedBaseName || isBuiltInEffect(recipe_.name))){error_->setText("Base effects are read-only. Enter a different name to save your copy.");name_->setFocus();return;}if(recipe_.name.empty() || !recipe_.validStops() || !recipe_.validLayerSettings() || (saveEffect && !saveEffect(recipe_))){error_->setText("Could not save. Check the name, colors and layer settings.");return;}accept();});connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
        connect(add,&QPushButton::clicked,this,[this]{append("Color source");});
        connect(remove,&QPushButton::clicked,this,[this]{if(selected_<1)return;commit();recipe_.layers.erase(recipe_.layers.begin()+selected_-1);selected_=0;editing_=recipe_;editing_.layers.clear();refreshStack();loadControls();});
        connect(stack_,&QListWidget::itemChanged,this,[this](QListWidgetItem* item){commit();const int row=stack_->row(item);const bool enabled=item->checkState()==Qt::Checked;if(row==0)recipe_.enabled=enabled;else recipe_.layers.at(row-1).enabled=enabled;if(row==selected_)editing_.enabled=enabled;preview_->update();});
        connect(stack_,&QListWidget::currentRowChanged,this,[this](int row){if(reordering_ || row<0 || row==selected_)return;commit();selected_=row;editing_=row?recipe_.layers.at(row-1):recipe_;editing_.layers.clear();loadControls();});
        connect(stack_->model(),&QAbstractItemModel::rowsAboutToBeMoved,this,[this]{commit();reordering_=true;});
        connect(stack_->model(),&QAbstractItemModel::rowsMoved,this,[this]{reordering_=false;const auto old=recipe_.layers;std::vector<CustomEffectDefinition> ordered;int row=0;for(int i=0;i<stack_->count();++i){const int id=stack_->item(i)->data(Qt::UserRole).toInt();if(id<0){row=i;continue;}ordered.push_back(old.at(id));}if(row!=0){refreshStack();return;}recipe_.layers=std::move(ordered);selected_=qMax(0,stack_->currentRow());editing_=selected_?recipe_.layers[selected_-1]:recipe_;editing_.layers.clear();refreshStack();loadControls();});
        if(allowLayers){leftLayout->addWidget(new QLabel("ADD EFFECT / drag onto preview",left));auto* templates=new PatternLibrary(left);templates->setObjectName("effectTemplates");leftLayout->addWidget(templates,1);connect(templates,&QListWidget::itemDoubleClicked,this,[this](QListWidgetItem* item){append(item->data(Qt::UserRole).toString());});}
        preview_->patternDropped=[this](QString text){append(text);};
        // Scrolling the inspector must not silently select a different effect or
        // change a number merely because the pointer passes over that control.
        for(auto* widget:inspector->findChildren<QWidget*>())if(qobject_cast<QComboBox*>(widget) || qobject_cast<QAbstractSpinBox*>(widget) || qobject_cast<QSlider*>(widget)){widget->setFocusPolicy(Qt::StrongFocus);widget->installEventFilter(this);}
        refreshStack();loadControls();split->setSizes({220,850});
    }
    CustomEffectDefinition definition()const {auto result=recipe_;if(selected_==0){auto layers=result.layers;result=editing_;result.layers=std::move(layers);}else result.layers[selected_-1]=editing_;return result;}
protected:
    bool eventFilter(QObject* watched,QEvent* event)override {
        if(event->type()==QEvent::Wheel){auto* widget=qobject_cast<QWidget*>(watched);auto* focus=QApplication::focusWidget();if(widget && !widget->hasFocus() && !(focus && widget->isAncestorOf(focus))){auto* wheel=static_cast<QWheelEvent*>(event);auto* bar=inspectorScroll_->verticalScrollBar();const int delta=wheel->pixelDelta().isNull()?wheel->angleDelta().y()/2:wheel->pixelDelta().y();bar->setValue(bar->value()-delta);event->accept();return true;}}
        return QDialog::eventFilter(watched,event);
    }
private:
    static int durationPosition(int ms){return int(std::lround(std::log(ms/500.)/std::log(240.)*1000.));}
    void commit(){if(selected_==0){auto layers=std::move(recipe_.layers);recipe_=editing_;recipe_.layers=std::move(layers);}else recipe_.layers[selected_-1]=editing_;recipe_.path="Canvas positions";}
    void append(QString pattern){commit();if(recipe_.layers.size()>=8)return;CustomEffectDefinition layer;int number=1;do{layer.name="Layer "+std::to_string(number++);}while(std::any_of(recipe_.layers.begin(),recipe_.layers.end(),[&](const auto& existing){return existing.name==layer.name;}));layer.pattern=pattern.toStdString();layer.path="Canvas positions";const MsiColor colors[]={{0,80,255},{255,40,120},{255,170,0},{30,220,110}};const auto color=colors[recipe_.layers.size()%4];layer.colors={color,{0,0,0}};layer.angle=180;recipe_.layers.push_back(layer);selected_=int(recipe_.layers.size());editing_=layer;refreshStack();loadControls();}
    void refreshStack(){const QSignalBlocker b(stack_);stack_->clear();auto* base=new QListWidgetItem("Base",stack_);base->setData(Qt::UserRole,-1);base->setCheckState(recipe_.enabled?Qt::Checked:Qt::Unchecked);base->setFlags(base->flags()&~Qt::ItemIsDragEnabled);for(size_t i=0;i<recipe_.layers.size();++i){auto* item=new QListWidgetItem(QString::fromStdString(recipe_.layers[i].name),stack_);item->setData(Qt::UserRole,int(i));item->setCheckState(recipe_.layers[i].enabled?Qt::Checked:Qt::Unchecked);item->setToolTip("Checked: included in the combined output. Click the name to inspect this layer.");}stack_->setCurrentRow(selected_);}
    void syncColor(){if(editing_.colors.empty())editing_.colors={{255,0,0},{0,0,0}};stops_->selected=qBound(0,stops_->selected,int(editing_.colors.size())-1);const auto c=editing_.colors[stops_->selected];const QColor color(c.r,c.g,c.b);swatch_->setText(color.name()+" / color wheel");swatch_->setStyleSheet("background:"+color.name()+";color:"+(color.lightness()>140?QString("#101820"):QString("white"))+";");const int values[]={c.r,c.g,c.b};for(int i=0;i<3;++i){const QSignalBlocker b(rgb_[i]);rgb_[i]->setValue(values[i]);}stops_->update();}
    void updateParameters(){
        // Layer names survive deletion/reordering; the row index is not identity.
        const QString title=selected_?QString::fromStdString(editing_.name):QString("Layer 0 / Base");
        soloTitle_->setText(title+" / solo preview");solo_->caption=title+" / "+QString::fromStdString(effectDisplayName(editing_.pattern));const auto& p=editing_.pattern;const bool linear=p=="Scroll" || p=="Comet" || p=="Marquee" || p=="Color source";const bool circular=p=="Radial" || p=="Angular" || p=="Spiral";parameters_->setRowVisible(direction_,linear && p!="Color source");parameters_->setRowVisible(motion_,p=="Color source");parameters_->setRowVisible(sliderRows_.count(angle_)?sliderRows_[angle_]:angle_,linear || p=="Angular" || p=="Spiral");parameters_->setRowVisible(rotation_,circular);parameters_->setRowVisible(sliderRows_.count(spread_)?sliderRows_[spread_]:spread_,p=="Angular" || p=="Spiral");parameters_->setRowVisible(sliderRows_.count(size_)?sliderRows_[size_]:size_,p!="Breathing");parameters_->setRowVisible(durationRow_,p!="Static gradient");stops_->setVisible(p!="Color source");form_->setRowVisible(palette_,p!="Color source");for(int i=0;i<colors_->count();++i)if(auto* action=colors_->itemAt(i)->widget())action->setVisible(p!="Color source");const QSignalBlocker b(rotation_);rotation_->clear();rotation_->addItems(p=="Radial"?QStringList{"Outward","Inward"}:QStringList{"Clockwise","Counterclockwise"});rotation_->setCurrentIndex(editing_.reverse?1:0);}
    void loadControls(){solo_->drawZoneMode=false;solo_->unsetCursor();if(auto* draw=findChild<QPushButton*>("drawLayerZone")){const QSignalBlocker b(draw);draw->setChecked(false);}if(editing_.stops.size()!=editing_.colors.size()){editing_.stops.clear();for(size_t i=0;i<editing_.colors.size();++i)editing_.stops.push_back(i/double(editing_.colors.size()));}{const QSignalBlocker b(motion_);motion_->setCurrentText(QString::fromStdString(editing_.sourceMotion));}editing_.path="Canvas positions";selectedTitle_->setText(selected_?QString::fromStdString(editing_.name):"Base layer");{const QSignalBlocker b(pattern_);pattern_->setCurrentIndex(pattern_->findData(QString::fromStdString(editing_.pattern)));}stops_->selected=0;{const QSignalBlocker b(angle_);angle_->setValue(editing_.angle);}{const QSignalBlocker b(direction_);direction_->setCurrentIndex(std::abs(std::remainder(editing_.angle,90.))<.01?(int(std::lround(editing_.angle/90.))+4)%4:4);}{const QSignalBlocker b(spread_);spread_->setValue(editing_.spreadDegrees);}{const QSignalBlocker b(size_);size_->setValue(editing_.wavelength);}{const QSignalBlocker b(seconds_);seconds_->setValue(editing_.periodMs/1000.);}{const QSignalBlocker b(speed_);speed_->setValue(durationPosition(editing_.periodMs));}{const QSignalBlocker b(zone_);zone_->setChecked(editing_.zoneEnabled);}const int values[]={editing_.zoneX,editing_.zoneY,editing_.zoneWidth,editing_.zoneHeight};for(int i=0;i<4;++i){const QSignalBlocker b(zoneFields_[i]);zoneFields_[i]->setValue(values[i]);}for(auto& sync:sync_)sync();for(const auto& entry:sliderRows_){auto* slider=entry.second->findChild<QSlider*>();if(auto* real=qobject_cast<QDoubleSpinBox*>(entry.first)){const QSignalBlocker b(slider);slider->setValue(qRound(real->value()*(real->objectName()=="effectOpacity"?100:1)));}else if(auto* integer=qobject_cast<QSpinBox*>(entry.first)){const QSignalBlocker b(slider);slider->setValue(integer->value());}}syncColor();updateParameters();}
    std::map<QWidget*,QWidget*> sliderRows_;QLabel* soloTitle_;EffectPreview* solo_;QFormLayout* form_;QComboBox* palette_;QHBoxLayout *colors_,*durationRow_;QScrollArea* inspectorScroll_;CustomEffectDefinition recipe_,editing_;int selected_=0;bool reordering_=false;QLineEdit* name_;QListWidget* stack_;EffectPreview* preview_;QLabel* selectedTitle_;QLabel* error_;GradientStops* stops_;QPushButton* swatch_;std::array<QSpinBox*,3> rgb_;std::array<QSpinBox*,4> zoneFields_;QComboBox *pattern_,*direction_,*rotation_,*motion_;QDoubleSpinBox *angle_,*spread_,*seconds_;QSpinBox* size_;QSlider* speed_;QCheckBox* zone_;QFormLayout* parameters_;std::vector<std::function<void()>> sync_;
};
