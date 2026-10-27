#include "StudioWidgets.h"
#include <QApplication>
#include <QCheckBox>
#include <memory>
#include <QCursor>
#include <QKeyEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <algorithm>
#include <functional>

namespace {
class SaturationValueField : public QWidget {
public:
    int hue=0,saturation=255,brightness=255;
    std::function<void(int,int)> changed;
    SaturationValueField(QWidget* parent):QWidget(parent) {
        setObjectName("saturationValueField");setMinimumSize(220,150);setFocusPolicy(Qt::StrongFocus);
        setAccessibleName("Saturation and brightness");setToolTip("Drag to choose saturation and brightness. Arrow keys adjust; numeric fields provide exact input.");
    }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);QLinearGradient saturationGradient(0,0,width(),0);
        saturationGradient.setColorAt(0,Qt::white);saturationGradient.setColorAt(1,QColor::fromHsv(hue,255,255));
        p.fillRect(rect(),saturationGradient);QLinearGradient valueGradient(0,0,0,height());
        valueGradient.setColorAt(0,Qt::transparent);valueGradient.setColorAt(1,Qt::black);p.fillRect(rect(),valueGradient);
        p.setRenderHint(QPainter::Antialiasing);p.setBrush(Qt::NoBrush);
        const QPointF point(saturation/255.0*(width()-1),(1-brightness/255.0)*(height()-1));
        p.setPen(QPen(Qt::black,3));p.drawEllipse(point,5,5);p.setPen(QPen(Qt::white,1.5));p.drawEllipse(point,5,5);
    }
    void pick(QPointF point) {
        saturation=qBound(0,qRound(255*point.x()/std::max(1,width()-1)),255);
        brightness=qBound(0,qRound(255*(1-point.y()/std::max(1,height()-1))),255);
        update();changed(saturation,brightness);
    }
    void mousePressEvent(QMouseEvent* event) override {if(event->button()==Qt::LeftButton){setFocus();pick(event->position());}}
    void mouseMoveEvent(QMouseEvent* event) override {if(event->buttons()&Qt::LeftButton)pick(event->position());}
    void keyPressEvent(QKeyEvent* event) override {
        if(event->key()==Qt::Key_Left)saturation=std::max(0,saturation-1);
        else if(event->key()==Qt::Key_Right)saturation=std::min(255,saturation+1);
        else if(event->key()==Qt::Key_Up)brightness=std::min(255,brightness+1);
        else if(event->key()==Qt::Key_Down)brightness=std::max(0,brightness-1);
        else {QWidget::keyPressEvent(event);return;}
        update();changed(saturation,brightness);
    }
};
// Each overlay belongs to one screen. A virtual-desktop-sized window stretches
// captured UI on Windows when monitors have different DPI scales or coordinate gaps.
class ScreenSampler : public QDialog {
    QImage image_;
public:
    std::function<void(QColor)> picked;
    ScreenSampler(QWidget* owner,QScreen* screen,QImage image):QDialog(owner,Qt::Tool|Qt::FramelessWindowHint),image_(std::move(image)) {
        setObjectName("screenColorDialog");setAttribute(Qt::WA_DeleteOnClose);setCursor(Qt::CrossCursor);setMouseTracking(true);
        setScreen(screen);setGeometry(screen->geometry());setFixedSize(screen->geometry().size());
        setAccessibleName("Pick a screen color. Click to accept, Escape to cancel.");
    }
    bool event(QEvent* event) override {if(event->type()==QEvent::ShortcutOverride){event->accept();return true;}return QDialog::event(event);}
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);painter.drawImage(rect(),image_);
        painter.fillRect(QRect(20,20,330,32),QColor("#17191f"));painter.setPen(Qt::white);painter.drawText(QRect(28,20,314,32),Qt::AlignVCenter,"Click a color - Escape cancels");
    }
    void mousePressEvent(QMouseEvent* event) override {
        if(event->button()==Qt::RightButton){reject();return;}
        if(event->button()!=Qt::LeftButton || !rect().contains(event->position().toPoint()))return;
        const auto local=event->position();
        const QPoint pixel(qBound(0,int(local.x()*image_.width()/width()),image_.width()-1),qBound(0,int(local.y()*image_.height()/height()),image_.height()-1));
        if(picked)picked(image_.pixelColor(pixel).toRgb());accept();
    }
};

class ColorPopup : public QDialog {
public:
    using QDialog::QDialog;
    bool finished=false,sampling=false,gestureBegun=false;
    void clampToOwner() {
        if(!parentWidget())return;
        const auto available=parentWidget()->screen()->availableGeometry();
        const auto anchor=parentWidget()->mapToGlobal(QPoint(0,parentWidget()->height()));
        move(qBound(available.left(),anchor.x(),std::max(available.left(),available.right()-width()+1)),
             qBound(available.top(),anchor.y(),std::max(available.top(),available.bottom()-height()+1)));
    }
    bool event(QEvent* event) override {
        // Global project/tool shortcuts must not retarget a live color session.
        if(event->type()==QEvent::ShortcutOverride){event->accept();return true;}
        const bool result=QDialog::event(event);
        // Restoring sampling and expanding RGB details can change the popup's screen bounds.
        if(event->type()==QEvent::Show || event->type()==QEvent::Resize)clampToOwner();
        return result;
    }
    void done(int result) override {if(sampling){hide();return;}finished=true;QDialog::done(result);}
    void hideEvent(QHideEvent* event) override {
        QDialog::hideEvent(event);
        // Clicking outside a popup is Cancel, including restoration of live preview.
        if(!finished && !sampling)done(QDialog::Rejected);
    }
};
}

void openStudioColorPicker(ColorEditor* editor,bool mouseInitiated)
{
    if(editor->findChild<QDialog*>("liveColorPicker"))return;
    const QColor original=editor->color();
    auto* popup=new ColorPopup(editor,Qt::Popup);popup->setObjectName("liveColorPicker");
    popup->setAttribute(Qt::WA_DeleteOnClose);popup->setWindowTitle("Choose color");
    auto* stack=new QVBoxLayout(popup);auto* square=new SaturationValueField(popup);stack->addWidget(square);
    auto* hue=new QSlider(Qt::Horizontal);hue->setRange(0,359);hue->setObjectName("pickerHue");hue->setAccessibleName("Hue degrees");
    hue->setStyleSheet("QSlider::groove:horizontal {height:10px;background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 red,stop:.167 yellow,stop:.333 lime,stop:.5 cyan,stop:.667 blue,stop:.833 magenta,stop:1 red);}");
    stack->addWidget(hue);
    auto* form=new QFormLayout;stack->addLayout(form);QVector<QSpinBox*> hsv,rgb;
    for(const auto& name:QStringList{"Hue","Saturation","Brightness","Red","Green","Blue"}) {
        auto* value=new QSpinBox;value->setObjectName("picker"+name);value->setAccessibleName(name);
        value->setRange(0,name=="Hue" ? 359 : hsv.size()<3 ? 100 : 255);
        if(hsv.size()<3){hsv.append(value);value->setSuffix(name=="Hue" ? " deg" : " %");}else rgb.append(value);
        form->addRow(name,value);
    }
    auto* hex=new QLineEdit;hex->setObjectName("pickerHex");hex->setAccessibleName("Hex RGB");hex->setMaxLength(7);form->addRow("Hex",hex);
    auto* numeric=new QCheckBox("RGB details");numeric->setObjectName("pickerRgbDetails");stack->addWidget(numeric);
    for(auto* field:rgb){field->hide();form->labelForField(field)->hide();}
    QObject::connect(numeric,&QCheckBox::toggled,popup,[=](bool on){for(auto* field:rgb){field->setVisible(on);form->labelForField(field)->setVisible(on);}popup->adjustSize();});
    auto update=std::make_shared<std::function<void(QColor)>>();
    *update=[=](QColor color) {
        color=color.toRgb();const bool changed=color!=editor->color();
        const QSignalBlocker h(hue),text(hex);
        if(color.hsvHue()>=0)square->hue=color.hsvHue(); // Keep hue when black/white has no chromatic hue.
        square->saturation=color.hsvSaturation();square->brightness=color.value();square->update();hue->setValue(square->hue);
        const int values[]{square->hue,qRound(square->saturation*100./255),qRound(square->brightness*100./255),color.red(),color.green(),color.blue()};
        for(int i=0;i<3;++i){const QSignalBlocker a(hsv[i]),b(rgb[i]);hsv[i]->setValue(values[i]);rgb[i]->setValue(values[i+3]);}
        hex->setText(color.name().toUpper());
        if(changed && !editor->signalsBlocked() && !popup->gestureBegun){popup->gestureBegun=true;emit editor->gestureStarted();}
        editor->setColor(color);if(changed)emit editor->colorChanged(color);
    };
    const auto fromHsv=[=] {(*update)(QColor::fromHsv(square->hue,square->saturation,square->brightness));};
    square->changed=[=](int,int){fromHsv();};
    QObject::connect(hue,&QSlider::valueChanged,popup,[=](int value){square->hue=value;fromHsv();});
    for(int i=0;i<3;++i) {
        QObject::connect(hsv[i],&QSpinBox::valueChanged,popup,[=](int value) {
            if(i==0)square->hue=value;else if(i==1)square->saturation=qRound(value*255./100);else square->brightness=qRound(value*255./100);fromHsv();
        });
        QObject::connect(rgb[i],&QSpinBox::valueChanged,popup,[=](int) {(*update)(QColor(rgb[0]->value(),rgb[1]->value(),rgb[2]->value()));});
    }
    auto* colorError=new QLabel; colorError->setObjectName("pickerColorError");colorError->setStyleSheet("color:#ff9b9b;");colorError->setWordWrap(true);stack->addWidget(colorError);colorError->hide();
    QObject::connect(hex,&QLineEdit::editingFinished,popup,[=] {
        const QString text=hex->text();const QColor color(text.startsWith('#') ? text : "#"+text);
        if(color.isValid() && color.alpha()==255){colorError->hide();(*update)(color);}
        else {colorError->setText("Enter a valid RGB hex color, for example #55DEFF.");colorError->show();hex->setText(editor->color().name().toUpper());popup->adjustSize();}
    });
    auto* palette=new QHBoxLayout;stack->addLayout(palette);
    QStringList colors{original.name()};
    for(auto* field:editor->window()->findChildren<ColorEditor*>())if(!colors.contains(field->color().name()))colors.append(field->color().name());
    for(const auto& color:QSettings().value("editor/recentColors").toStringList())if(!colors.contains(color))colors.append(color);
    for(const auto& color:colors.mid(0,8)) {
        auto* swatch=new QPushButton;swatch->setFixedSize(24,24);swatch->setAccessibleName("Use "+color);swatch->setToolTip(color);
        swatch->setStyleSheet("background:"+color+";border:1px solid #afb4bd;");palette->addWidget(swatch);
        QObject::connect(swatch,&QPushButton::clicked,popup,[=]{(*update)(QColor(color));});
    }
    auto* screen=new QPushButton("Screen eyedropper...");stack->addWidget(screen);
    screen->setToolTip("Pick a color anywhere on your screens. Click to accept; Escape or right-click cancels.");
    QObject::connect(screen,&QPushButton::clicked,popup,[=] {
        // The native eyedropper is a sibling window, never a stack child of a popup
        // that Qt can dismiss while the dialog changes focus.
        const QColor before=editor->color();popup->sampling=true;
        popup->setAttribute(Qt::WA_DeleteOnClose,false);popup->hide();
        auto samplers=std::make_shared<QVector<QPointer<ScreenSampler>>>();
        auto finished=std::make_shared<bool>(false);
        QStringList unavailable;
        // Capture every screen before showing any overlay, so screenshots cannot
        // include another sampler. No overlay enters the main window's layout.
        for(auto* display:QGuiApplication::screens()) {
            const auto image=display->grabWindow(0).toImage();
            if(image.isNull()){unavailable.append(display->name());continue;}
            auto* dialog=new ScreenSampler(editor->window(),display,image);samplers->append(dialog);
            dialog->picked=[=](QColor color){(*update)(color);};
            QObject::connect(dialog,&QDialog::finished,popup,[=](int result){
                if(*finished)return;*finished=true;
                for(const auto& other:*samplers)if(other && other!=dialog)other->reject();
                if(result!=QDialog::Accepted)(*update)(before);
                popup->sampling=false;popup->setAttribute(Qt::WA_DeleteOnClose);popup->show();
            });
        }
        if(samplers->isEmpty()) {
            popup->sampling=false;popup->setAttribute(Qt::WA_DeleteOnClose);popup->show();
            screen->setText("Screen capture unavailable");screen->setEnabled(false);
        } else {
            if(!unavailable.isEmpty()){screen->setText("Some screens unavailable");screen->setToolTip("Cannot capture: "+unavailable.join(", "));}
            ScreenSampler* active=nullptr;
            for(const auto& dialog:*samplers){dialog->show();dialog->raise();if(dialog->screen()==editor->screen())active=dialog;}
            if(active)active->activateWindow();
        }
    });
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Apply|QDialogButtonBox::Cancel);stack->addWidget(buttons);
    QObject::connect(buttons->button(QDialogButtonBox::Apply),&QPushButton::clicked,popup,&QDialog::accept);
    QObject::connect(buttons,&QDialogButtonBox::rejected,popup,&QDialog::reject);
    // Initialization must not create a document edit or an undo command.
    {const QSignalBlocker block(editor);(*update)(original);}
    QObject::connect(popup,&QDialog::finished,editor,[=](int result) {
        if(result!=QDialog::Accepted){const bool changed=editor->color()!=original;editor->setColor(original);if(changed)emit editor->colorChanged(original);}
        else {auto recent=QSettings().value("editor/recentColors").toStringList();recent.removeAll(editor->color().name());recent.prepend(editor->color().name());QSettings().setValue("editor/recentColors",recent.mid(0,8));}
        if(popup->gestureBegun)emit editor->gestureFinished(result==QDialog::Accepted && editor->color()!=original);
        editor->setFocus();
    });
    popup->adjustSize();const QRect available=editor->screen()->availableGeometry();const auto anchor=editor->mapToGlobal(QPoint(0,editor->height()));
    popup->move(qBound(available.left(),anchor.x(),std::max(available.left(),available.right()-popup->width())),
                qBound(available.top(),anchor.y(),std::max(available.top(),available.bottom()-popup->height())));
    if(mouseInitiated && studioMotionEnabled()) {
        popup->setWindowOpacity(0);auto* fade=new QPropertyAnimation(popup,"windowOpacity",popup);
        fade->setStartValue(0.0);fade->setEndValue(1.0);fade->setDuration(140);fade->setEasingCurve(QEasingCurve::OutCubic);popup->show();fade->start(QAbstractAnimation::DeleteWhenStopped);
    }else popup->show();
}
