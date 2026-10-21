#include "effects/EffectClock.h"
#pragma once
#include "effects/CustomGradientEffect.h"
#include "ui/ColorPicker.h"
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
#include <array>
#include <QDoubleSpinBox>
#include <QSignalBlocker>

class PatternLibrary : public QListWidget {
public:
    explicit PatternLibrary(QWidget* parent=nullptr):QListWidget(parent) {
        addItems({"Wave","Vertical","Diagonal","Radial","Spiral","Plasma","Breathing","Sweep","Static gradient"});
        setDragEnabled(true);setSpacing(5);setFixedWidth(185);setAccessibleName("Drag a pattern onto the preview");
    }
protected:
    void startDrag(Qt::DropActions) override {
        if(!currentItem())return;auto* mime=new QMimeData;mime->setData("application/x-lighting-pattern",currentItem()->text().toUtf8());
        QDrag drag(this);drag.setMimeData(mime);drag.exec(Qt::CopyAction);
    }
};

class EffectPreview : public QWidget {
public:
    CustomEffectDefinition* recipe;
    std::function<void(QString)> patternDropped;
    std::shared_ptr<IEffect> renderedEffect;
    explicit EffectPreview(CustomEffectDefinition* d,QWidget* parent):QWidget(parent),recipe(d) {
        setAcceptDrops(true);setMinimumSize(380,200);(void)effectTimeMs();
        timer_.setTimerType(Qt::PreciseTimer);timer_.setInterval(33);
        connect(&timer_,&QTimer::timeout,this,QOverload<>::of(&QWidget::update));timer_.start();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.setRenderHint(QPainter::SmoothPixmapTransform);
        if(recipe->path!="Canvas positions") {
            p.fillRect(rect(),QColor("#10161e"));CustomGradientEffect effect(*recipe);const auto time=effectTimeMs();
            for(int fan=0;fan<2;++fan){const QPointF center(width()*(fan?.72:.28),height()*.5);const double radius=qMin(width()*.17,height()*.30);p.setPen(QPen(QColor("#3d5168"),2));p.setBrush(QColor("#1c2734"));p.drawEllipse(center,radius+14,radius+14);
                for(int led=0;led<12;++led){const double angle=led*6.283185307/12-1.570796327;const auto c=effect.colorForLed(0,0,time,fan*12+led,24,led,12);p.setPen(Qt::NoPen);p.setBrush(QColor(c.r,c.g,c.b));p.drawEllipse(center+QPointF(std::cos(angle)*radius,std::sin(angle)*radius),7,7);}
                p.setPen(QColor("#d7e1ec"));p.drawText(QRectF(center.x()-radius,center.y()-15,radius*2,30),Qt::AlignCenter,QString("Component %1").arg(fan+1));}
            p.setPen(QColor("#a9b4c3"));p.drawText(rect().adjusted(14,12,-14,-12),Qt::AlignBottom|Qt::AlignLeft,"12 individual LED addresses per component");return;
        }
        QImage image(220,140,QImage::Format_RGB32);CustomGradientEffect effect(*recipe);const auto time=effectTimeMs();
        for(int y=0;y<140;++y){auto* row=reinterpret_cast<QRgb*>(image.scanLine(y));for(int x=0;x<220;++x){auto c=renderedEffect?renderedEffect->colorAt(x*5,y*5,time):effect.colorAt(x*5,y*5,time);row[x]=qRgb(c.r,c.g,c.b);}}
        p.drawImage(rect(),image);p.setPen(Qt::white);p.drawText(rect().adjusted(14,12,-14,-12),Qt::AlignTop|Qt::AlignLeft,QString::fromStdString(recipe->pattern));
    }
    void dragEnterEvent(QDragEnterEvent* e) override {if(e->mimeData()->hasFormat("application/x-lighting-pattern"))e->acceptProposedAction();}
    void dropEvent(QDropEvent* e) override {
        const auto pattern=QString::fromUtf8(e->mimeData()->data("application/x-lighting-pattern"));
        if(CustomEffectDefinition::validPattern(pattern.toStdString())){recipe->pattern=pattern.toStdString();if(patternDropped)patternDropped(pattern);update();e->acceptProposedAction();}
    }
private:
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
        const auto color=CustomGradientEffect(*recipe).colorAt(int(position*recipe->wavelength),0,0);
        const auto i=std::lower_bound(recipe->stops.begin(),recipe->stops.end(),position)-recipe->stops.begin();
        recipe->stops.insert(recipe->stops.begin()+i,position);recipe->colors.insert(recipe->colors.begin()+i,color);selected=int(i);update();if(changed)changed();
    }
    void removeStop(){if(recipe->colors.size()<=2)return;recipe->colors.erase(recipe->colors.begin()+selected);recipe->stops.erase(recipe->stops.begin()+selected);selected=qMin(selected,int(recipe->colors.size())-1);update();if(changed)changed();}
    void editStop(){const auto c=recipe->colors[selected];const QColor chosen=ColorPicker::getColor(QColor(c.r,c.g,c.b),this,"Gradient stop color");if(chosen.isValid()){recipe->colors[selected]={uint8_t(chosen.red()),uint8_t(chosen.green()),uint8_t(chosen.blue())};update();if(changed)changed();}}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        auto palette=*recipe;palette.pattern="Static gradient";CustomGradientEffect gradient(palette);
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
    explicit EffectEditor(CustomEffectDefinition recipe,QWidget* parent=nullptr):QDialog(parent),recipe_(std::move(recipe)) {
        setWindowTitle("Visual effect studio");resize(840,620);auto* root=new QVBoxLayout(this);
        name_=new QLineEdit(QString::fromStdString(recipe_.name),this);name_->setObjectName("effectRecipeName");name_->setMaxLength(80);name_->setPlaceholderText("Effect name");root->addWidget(name_);
        auto* hint=new QLabel("Drag a pattern onto the preview. Drag color stops below; double-click a stop for the color wheel and RGB.",this);hint->setWordWrap(true);root->addWidget(hint);
        auto* body=new QHBoxLayout;root->addLayout(body,1);auto* library=new PatternLibrary(this);library->setObjectName("patternLibrary");body->addWidget(library);
        auto* preview=new EffectPreview(&recipe_,this);preview->setObjectName("effectPreview");body->addWidget(preview,1);
        library->setCurrentRow(qMax(0,library->row(library->findItems(QString::fromStdString(recipe_.pattern),Qt::MatchExactly).value(0))));
        connect(library,&QListWidget::currentTextChanged,this,[this,preview](const QString& text){recipe_.pattern=text.toStdString();preview->update();});
        preview->patternDropped=[library](QString text){const auto items=library->findItems(text,Qt::MatchExactly);if(!items.empty())library->setCurrentItem(items.front());};
        auto* path=new QComboBox(this);path->setObjectName("effectPath");path->addItems({"Component LEDs","Entire chain","Canvas positions"});path->setCurrentText(QString::fromStdString(recipe_.path));
        auto* pathRow=new QHBoxLayout;pathRow->addWidget(new QLabel("Run colors along",this));pathRow->addWidget(path,1);root->addLayout(pathRow);
        connect(path,&QComboBox::currentTextChanged,this,[this,preview](const QString& text){recipe_.path=text.toStdString();preview->update();});
        auto* stops=new GradientStops(&recipe_,this);root->addWidget(stops);
        auto* colorRow=new QHBoxLayout;root->addLayout(colorRow);auto* swatch=new QPushButton(this);swatch->setMinimumHeight(32);colorRow->addWidget(swatch);
        std::array<QSpinBox*,3> rgb;for(int i=0;i<3;++i){colorRow->addWidget(new QLabel(QString("RGB"[i]),this));rgb[i]=new QSpinBox(this);rgb[i]->setObjectName(QString("stop%1").arg("RGB"[i]));rgb[i]->setRange(0,255);colorRow->addWidget(rgb[i]);}
        const auto syncColor=[this,stops,swatch,rgb]{const auto c=recipe_.colors[stops->selected];const QColor color(c.r,c.g,c.b);swatch->setText(color.name()+" / color wheel");swatch->setStyleSheet("background:"+color.name()+";color:"+(color.lightness()>140?QString("#111820"):QString("#ffffff"))+";");const int values[]={c.r,c.g,c.b};for(int i=0;i<3;++i){const QSignalBlocker b(rgb[i]);rgb[i]->setValue(values[i]);}};
        stops->changed=syncColor;syncColor();connect(swatch,&QPushButton::clicked,this,[stops]{stops->editStop();});
        for(auto* value:rgb)connect(value,&QSpinBox::valueChanged,this,[this,stops,rgb,syncColor]{recipe_.colors[stops->selected]={uint8_t(rgb[0]->value()),uint8_t(rgb[1]->value()),uint8_t(rgb[2]->value())};stops->update();syncColor();});
        auto* actions=new QHBoxLayout;root->addLayout(actions);
        for(const auto& text:{"Add color","Edit color","Remove color"}){auto* button=new QPushButton(text,this);actions->addWidget(button);connect(button,&QPushButton::clicked,this,[stops,text]{if(QString(text)=="Add color"){
            double largest=-1,position=.5;for(size_t i=0;i<stops->recipe->stops.size();++i){const double end=i+1<stops->recipe->stops.size()?stops->recipe->stops[i+1]:1.;const double gap=end-stops->recipe->stops[i];if(gap>largest){largest=gap;position=stops->recipe->stops[i]+gap/2.;}}stops->addStop(position);
        }else if(QString(text)=="Edit color")stops->editStop();else stops->removeStop();});}
        auto* controls=new QFormLayout;root->addLayout(controls);
        auto* durationRow=new QHBoxLayout;
        auto* speed=new QSlider(Qt::Horizontal,this);speed->setMinimumHeight(26);speed->setObjectName("effectDuration");speed->setRange(0,1000);
        // Equal travel changes duration by an equal ratio; the old linear range
        // reached a nearly stationary 30-second cycle after only one quarter.
        const auto sliderPosition=[](int ms){return int(std::lround(std::log(ms/500.)/std::log(240.)*1000.));};
        speed->setValue(sliderPosition(recipe_.periodMs));speed->setAccessibleName("Cycle duration, fast to slow");durationRow->addWidget(speed,1);
        auto* seconds=new QDoubleSpinBox(this);seconds->setObjectName("effectDurationSeconds");seconds->setRange(.5,120);seconds->setDecimals(3);seconds->setSingleStep(.1);seconds->setSuffix(" s");seconds->setValue(recipe_.periodMs/1000.);seconds->setAccessibleName("Seconds per complete cycle");durationRow->addWidget(seconds);
        controls->addRow("Cycle duration",durationRow);
        connect(speed,&QSlider::valueChanged,this,[this,seconds](int position){recipe_.periodMs=int(std::lround(500.*std::pow(240.,position/1000.)));const QSignalBlocker block(seconds);seconds->setValue(recipe_.periodMs/1000.);});
        connect(seconds,&QDoubleSpinBox::valueChanged,this,[this,speed,sliderPosition](double value){recipe_.periodMs=int(std::lround(value*1000.));const QSignalBlocker block(speed);speed->setValue(sliderPosition(recipe_.periodMs));});
        auto* scale=new QSlider(Qt::Horizontal,this);scale->setMinimumHeight(26);scale->setObjectName("effectScale");scale->setRange(40,4000);scale->setValue(recipe_.wavelength);controls->addRow("Pattern size (small to large)",scale);
        connect(scale,&QSlider::valueChanged,this,[this](int v){recipe_.wavelength=v;});
        auto* error=new QLabel(this);error->setWordWrap(true);error->setObjectName("effectSaveError");root->addWidget(error);
        auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,this);root->addWidget(buttons);
        connect(buttons,&QDialogButtonBox::accepted,this,[this,error]{recipe_.name=name_->text().trimmed().toStdString();if(recipe_.name.empty()){error->setText("Name this effect before saving.");name_->setFocus();return;}if(!recipe_.validStops() || (saveEffect && !saveEffect(recipe_))){error->setText("Could not save. Check color stops and the 64-effect library limit; your edits are still here.");return;}accept();});
        connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    }
    CustomEffectDefinition definition()const{return recipe_;}
private:
    CustomEffectDefinition recipe_;QLineEdit* name_;
};
