#pragma once
#include <QDialog>
#include <QDialogButtonBox>
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <functional>
#include <cmath>

// Hue/saturation are direct manipulation; RGB and hex remain exact input paths.
class ColorWheel : public QWidget {
public:
    QColor color;
    std::function<void(QColor)> changed;
    explicit ColorWheel(QWidget* parent=nullptr):QWidget(parent) {
        setFixedSize(250,250);setAccessibleName("Hue and saturation wheel");
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        static const QImage wheel=[] {
            QImage image(250,250,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);
            for(int y=0;y<250;++y)for(int x=0;x<250;++x) {
                const double dx=x-125.,dy=125.-y,r=std::hypot(dx,dy)/120.;
                if(r<=1)image.setPixelColor(x,y,QColor::fromHsvF(std::fmod(std::atan2(dy,dx)/6.283185307+1.,1.),r,1.));
            }
            return image;
        }();
        p.drawImage(0,0,wheel);
        const double angle=qMax(0.,color.hsvHueF())*6.283185307,r=color.hsvSaturationF()*120.;
        p.setPen(QPen(Qt::black,4));p.setBrush(Qt::NoBrush);QPointF point(125+std::cos(angle)*r,125-std::sin(angle)*r);
        p.drawEllipse(point,6,6);p.setPen(QPen(Qt::white,2));p.drawEllipse(point,6,6);
    }
    void mousePressEvent(QMouseEvent* e) override {choose(e);}
    void mouseMoveEvent(QMouseEvent* e) override {if(e->buttons()&Qt::LeftButton)choose(e);}
    void choose(QMouseEvent* e) {
        const double x=e->position().x()-125.,y=125.-e->position().y();
        color=QColor::fromHsvF(std::fmod(std::atan2(y,x)/6.283185307+1.,1.),qMin(1.,std::hypot(x,y)/120.),color.valueF());
        update();if(changed)changed(color);
    }
};

class ColorPicker : public QDialog {
public:
    explicit ColorPicker(QColor initial,QWidget* parent=nullptr):QDialog(parent),color_(initial) {
        setWindowTitle("Color");auto* layout=new QVBoxLayout(this);
        wheel_=new ColorWheel(this);layout->addWidget(wheel_,0,Qt::AlignHCenter);
        auto* form=new QFormLayout;layout->addLayout(form);
        value_=new QSlider(Qt::Horizontal,this);value_->setRange(0,255);value_->setAccessibleName("Brightness");form->addRow("Brightness",value_);
        for(int i=0;i<3;++i) {
            rgb_[i]=new QSpinBox(this);rgb_[i]->setRange(0,255);rgb_[i]->setObjectName(QString("color%1").arg("RGB"[i]));
            form->addRow(QString("RGB"[i]),rgb_[i]);
            connect(rgb_[i],&QSpinBox::valueChanged,this,[this]{if(!syncing_)setColor(QColor(rgb_[0]->value(),rgb_[1]->value(),rgb_[2]->value()));});
        }
        hex_=new QLineEdit(this);hex_->setObjectName("colorHex");form->addRow("Hex",hex_);
        connect(hex_,&QLineEdit::editingFinished,this,[this]{QColor c(hex_->text());if(c.isValid())setColor(c);else setColor(color_);});
        connect(value_,&QSlider::valueChanged,this,[this](int v){if(!syncing_){auto c=color_;c.setHsvF(qMax(0.,c.hsvHueF()),c.hsvSaturationF(),v/255.);setColor(c);}});
        wheel_->changed=[this](QColor c){setColor(c);};
        auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);layout->addWidget(buttons);
        connect(buttons,&QDialogButtonBox::accepted,this,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);setColor(initial);
    }
    QColor color()const{return color_;}
    static QColor getColor(QColor initial,QWidget* parent,const QString& title) {
        ColorPicker dialog(initial,parent);dialog.setWindowTitle(title);return dialog.exec()==QDialog::Accepted?dialog.color():QColor();
    }
private:
    void setColor(QColor c) {
        syncing_=true;color_=c;wheel_->color=c;wheel_->update();value_->setValue(c.value());
        rgb_[0]->setValue(c.red());rgb_[1]->setValue(c.green());rgb_[2]->setValue(c.blue());hex_->setText(c.name());syncing_=false;
    }
    QColor color_;ColorWheel* wheel_;QSlider* value_;QSpinBox* rgb_[3];QLineEdit* hex_;bool syncing_=false;
};
