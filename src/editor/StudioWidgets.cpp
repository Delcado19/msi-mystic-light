#include "StudioWidgets.h"
#include "SnapGrid.h"
#include "LightingDocument.h"
#include <QApplication>
#include <QSettings>
#include <QVariantAnimation>
#include <QAbstractSpinBox>
#include <QMap>
#include <QMenu>
#include <QFormLayout>
#include <QColorDialog>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QHideEvent>
#include <QLinearGradient>
#include <QTimer>
#include <QElapsedTimer>
#include <QCheckBox>
#include <QStyle>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>
#include <algorithm>

void applyStudioStyle(QWidget* window)
{
    static const bool resources=[] {Q_INIT_RESOURCE(branding);return true;}();Q_UNUSED(resources);
    auto font=window->font(); font.setPointSizeF(std::max(10.0,font.pointSizeF())); window->setFont(font);
    window->setStyleSheet(R"(
        QWidget { color: #f1f3f5; background: #1b1d21; }
        QMainWindow, QDialog { background: #111214; }
        QMenuBar, QToolBar, QStatusBar { background: #151619; spacing: 6px; }
        QMenuBar { padding: 4px; }
        QStatusBar::item { border: 0; }
        QDockWidget::close-button { background: #afb4bd; border-radius: 3px; }
        QDockWidget::close-button:hover { background: #55deff; }
        QMenuBar::item:selected, QMenu::item:selected { background: #293c58; }
        QToolBar { border: 0; border-bottom: 1px solid #34373d; padding: 5px; }
        QToolBar#effectTools { border-right: 1px solid #34373d; background: #151619; spacing: 3px; }
        QToolBar#effectTools QPushButton { border-color: transparent; background: transparent; }
        QToolBar#effectTools QPushButton:hover { background: #293c58; }
        QToolBar#effectTools QPushButton:checked { background: #17465c; border-color: #55deff; }
        QSplitter::handle { background: #34373d; height: 5px; }
        QDockWidget { font-weight: 600; }
        QDockWidget::title { padding: 8px 12px; background: #151619; border-bottom: 1px solid #34373d; }
        QTabWidget::pane { border: 0; }
        QTabBar::tab { padding: 10px 18px; color: #afb4bd; background: #151619; border-bottom: 2px solid transparent; }
        QTabBar::tab:selected { color: #f4f6ff; border-bottom-color: #55deff; background: #242e33; }
        QScrollArea { border: 0; }
        QPushButton, QToolButton { background: #24272d; border: 1px solid #555b65; border-radius: 4px; padding: 6px 10px; }
        QPushButton:hover, QToolButton:hover { background: #2c3c55; border-color: #55deff; }
        QPushButton:pressed, QToolButton:pressed, QToolButton:checked, QPushButton:checked { background: #20383e; border-color: #55deff; }
        QPushButton:disabled, QToolButton:disabled { color: #8492a8; background: #192232; border-color: #34373d; }
        QPushButton[primary="true"] { background: #174c68; border-color: #55deff; color: white; font-weight: 600; }
        QComboBox, QLineEdit, QSpinBox, QDoubleSpinBox { border: 1px solid #555b65; border-radius: 4px; background: #111214; padding: 6px; min-height: 18px; }
        QComboBox { padding-right: 20px; }
        QAbstractSpinBox::up-button, QAbstractSpinBox::down-button { width: 0; }
        QTreeWidget, QTreeView, QListWidget { background: #151619; border: 0; outline: 0; }
        QTreeWidget::item, QTreeView::item, QListWidget::item { padding: 8px 4px; border-radius: 5px; }
        QTreeWidget::item:selected, QTreeView::item:selected, QListWidget::item:selected { background: #28363c; color: white; }
        QHeaderView::section { background: #1b1d21; color: #afb4bd; border: 0; padding: 6px; }
        QSlider::groove:horizontal { background: #454c5e; height: 4px; border-radius: 2px; }
        QSlider::sub-page:horizontal { background: #55deff; border-radius: 2px; }
        QSlider::handle:horizontal { background: #dce5ff; width: 12px; margin: -5px 0; border-radius: 6px; }
        QSlider { min-height: 24px; }
        QCheckBox { spacing: 8px; padding: 4px 0; }
        QCheckBox::indicator { width: 15px; height: 15px; border: 1px solid #717b91; border-radius: 3px; background: #1d2027; }
        QCheckBox::indicator:checked { background: #55deff; border-color: #c1d0ff; }
        QLabel { background: transparent; }
        QLabel[muted="true"] { color: #afb4bd; }
        QLabel[heading="true"] { font-size: 16px; font-weight: 600; padding: 6px 0; }
        QToolButton[disclosure="true"] { text-align: left; background: transparent; border: 0; border-top: 1px solid #34373d; border-radius: 0; padding: 12px 0; }
        /* Style both orientations and page regions: native fallbacks otherwise draw mismatched/dithered tracks. */
        QScrollBar { background: #20242c; border: 0; margin: 0; }
        QScrollBar:vertical { width: 14px; }
        QScrollBar:horizontal { height: 14px; }
        QScrollBar::handle { background: #9aa8c2; border: 1px solid #b1bbcf; border-radius: 5px; }
        QScrollBar::handle:vertical { min-height: 28px; }
        QScrollBar::handle:horizontal { min-width: 28px; }
        QScrollBar::handle:hover { background: #b9c9e7; }
        QScrollBar::handle:pressed { background: #d2def4; }
        QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; border: 0; background: #20242c; }
        QScrollBar::add-page, QScrollBar::sub-page { background: #20242c; border: 0; }
        QAbstractScrollArea::corner { background: #20242c; border: 0; }
        QWidget:focus { border-color: #55deff; }
        QAbstractItemView:focus { border: 1px solid #55deff; }
        QToolBar#workspaceNavigation QToolButton:checked { background: #242e33; border-color: #55deff; color: #f1f3f5; font-weight: 600; }
        QPushButton#playPreview { background: #17465c; border-color: #55deff; color: #e8faff; }
        QLabel[statusBadge="true"], QLabel#previewPlaybackStatus { padding: 4px 10px; border: 1px solid #555b65; border-radius: 4px; background: #151619; }
        QLabel[statusBadge="true"][outputState="running"] { color: #7de8c3; border-color: #7de8c3; }
        QLabel[statusBadge="true"][outputState="error"], QLabel[statusBadge="true"][outputState="faulted"] { color: #ff9eac; border-color: #ff9eac; }
        QLabel[modeBadge="true"] { padding: 6px 10px; color: #55deff; border-left: 2px solid #55deff; background: #151619; }
        QLabel[emptyState="true"] { padding: 24px; border: 1px solid #555b65; border-radius: 6px; background: #151619; }
        QToolButton#workspacePanels { border-color: #8a8f98; }
        QToolTip { color: #edf1ff; background: #343d50; border: 1px solid #7788ad; padding: 6px; }
    )");
}

namespace {
int sliderPosition(QDoubleSpinBox* value) {
    const double scale=value->property("displayScale").isValid() ? value->property("displayScale").toDouble() : 1;
    const double lo=value->minimum()*scale,hi=value->maximum()*scale;
    double fraction=(value->value()*scale-lo)/std::max(1e-12,hi-lo);
    if(value->property("slowRange").toBool())fraction=std::sqrt(std::max(0.0,fraction));
    return qRound(fraction*1000);
}
}
QWidget* sliderField(QDoubleSpinBox* value)
{
    auto* row=new QWidget;auto* layout=new QVBoxLayout(row);layout->setContentsMargins(0,0,0,0);layout->setSpacing(2);
    auto* top=new QHBoxLayout;top->addStretch();top->addWidget(value);layout->addLayout(top);
    auto* slider=new QSlider(Qt::Horizontal);slider->setRange(0,1000);
    slider->setAccessibleName(value->accessibleName().isEmpty() ? value->objectName() : value->accessibleName());
    slider->setProperty("numericField",QVariant::fromValue(static_cast<QObject*>(value)));
    value->setButtonSymbols(QAbstractSpinBox::NoButtons);value->setMinimumWidth(96);
    if(!value->property("defaultValue").isValid())value->setProperty("defaultValue",value->value());
    value->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(value,&QWidget::customContextMenuRequested,value,[value](QPoint point){QMenu menu;auto* reset=menu.addAction("Reset to default");QObject::connect(reset,&QAction::triggered,value,[value]{value->setValue(value->property("defaultValue").toDouble());});if(value->minimum()<=0 && value->maximum()>=0){auto* off=menu.addAction("Zero / Off");QObject::connect(off,&QAction::triggered,value,[value]{value->setValue(0);});}menu.exec(value->mapToGlobal(point));});
    auto* reset=new QAction("Reset to default",value);reset->setShortcut(QKeySequence("Alt+R"));reset->setShortcutContext(Qt::WidgetShortcut);value->addAction(reset);QObject::connect(reset,&QAction::triggered,value,[value]{value->setValue(value->property("defaultValue").toDouble());});
    top->insertWidget(0,slider,1);auto* limits=new QLabel;limits->setProperty("muted",true);limits->setObjectName("parameterLimits");layout->addWidget(limits);
    const auto sync=[=] {
        const QSignalBlocker block(slider);slider->setValue(sliderPosition(value));slider->setEnabled(value->isEnabled());
        const double scale=value->property("displayScale").isValid() ? value->property("displayScale").toDouble() : 1;
        limits->setText(QString::number(qRound64(value->minimum()*scale))+" to "+QString::number(qRound64(value->maximum()*scale))+value->suffix());
        slider->setSingleStep(std::max(1,qRound(1000/std::max(1.0,(value->maximum()-value->minimum())*scale))));
        slider->setTickPosition(QSlider::TicksBelow);slider->setTickInterval(value->minimum()<0 ? 500 : 250);
    };
    QObject::connect(slider,&QSlider::valueChanged,value,[value](int n) {
        const double scale=value->property("displayScale").isValid() ? value->property("displayScale").toDouble() : 1;
        double fraction=n/1000.0;if(value->property("slowRange").toBool())fraction*=fraction;
        double shown=(value->minimum()+(value->maximum()-value->minimum())*fraction)*scale;
        const double step=value->objectName()=="trails" ? 10 : 1;shown=std::round(shown/step)*step;
        value->setValue(std::clamp(shown/scale,value->minimum(),value->maximum()));
    });
    QObject::connect(value,&QDoubleSpinBox::valueChanged,slider,[sync](double){sync();});sync();
    return row;
}
void syncSliderFields(QWidget* root)
{
    // Refresh blocks numeric signals; never round model values while synchronizing.
    for(auto* slider:root->findChildren<QSlider*>())if(auto* value=qobject_cast<QDoubleSpinBox*>(slider->property("numericField").value<QObject*>())) {
        const QSignalBlocker block(slider);slider->setEnabled(value->isEnabled());slider->setValue(sliderPosition(value));
        if(auto* limits=slider->parentWidget()->findChild<QLabel*>("parameterLimits")) {
            const double scale=value->property("displayScale").isValid() ? value->property("displayScale").toDouble() : 1;
            limits->setText(QString::number(qRound64(value->minimum()*scale))+" to "+QString::number(qRound64(value->maximum()*scale))+value->suffix());
        }
    }
}
QWidget* disclosure(QVBoxLayout* layout,const QString& title,QWidget* content)
{
    auto* box=new QWidget; auto* stack=new QVBoxLayout(box); stack->setContentsMargins(0,0,0,0); stack->setSpacing(0);
    auto* toggle=new QToolButton; toggle->setText(title); toggle->setProperty("disclosure",true);
    toggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); toggle->setArrowType(Qt::RightArrow); toggle->setCheckable(true);
    toggle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed); stack->addWidget(toggle); stack->addWidget(content); content->hide();
    const auto icon=[toggle](double angle){QPixmap pixmap(16,16);pixmap.fill(Qt::transparent);QPainter p(&pixmap);p.setRenderHint(QPainter::Antialiasing);p.translate(8,8);p.rotate(angle);p.setPen(QPen(QColor("#afb4bd"),1.5));p.drawLine(-2,-4,2,0);p.drawLine(2,0,-2,4);toggle->setIcon(QIcon(pixmap));};
    toggle->setArrowType(Qt::NoArrow);icon(0);
    auto* turn=new QVariantAnimation(toggle);turn->setDuration(140);turn->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(turn,&QVariantAnimation::valueChanged,toggle,[=](const QVariant& angle){toggle->setProperty("arrowAngle",angle);icon(angle.toDouble());});
    QObject::connect(toggle,&QToolButton::pressed,toggle,[=]{toggle->setProperty("mouseTriggered",QApplication::mouseButtons()!=Qt::NoButton);});
    QObject::connect(toggle,&QToolButton::toggled,content,[=](bool open) {
        content->setVisible(open);turn->stop();const bool mouse=toggle->property("mouseTriggered").toBool();toggle->setProperty("mouseTriggered",false);
        if(studioMotionEnabled() && mouse){turn->setStartValue(toggle->property("arrowAngle").toDouble());turn->setEndValue(open ? 90.0 : 0.0);turn->start();}
        else {toggle->setProperty("arrowAngle",open ? 90.0 : 0.0);icon(open ? 90 : 0);}
    });
    layout->addWidget(box); return box;
}
QIcon studioToolIcon(const QString& name)
{
    QPixmap pixmap(24,24); pixmap.fill(Qt::transparent); QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing); painter.setPen(QPen(QColor("#e0e8fc"),1.7,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    if(name=="eye" || name=="eye-off") {
        QPainterPath eye;eye.moveTo(2,12);eye.cubicTo(7,4,17,4,22,12);eye.cubicTo(17,20,7,20,2,12);
        painter.drawPath(eye);painter.drawEllipse(QPointF(12,12),3,3);
        if(name=="eye-off") {painter.setPen(QPen(QColor("#17191f"),4.5,Qt::SolidLine,Qt::RoundCap));painter.drawLine(3,3,21,21);painter.setPen(QPen(QColor("#b5c0d6"),1.7,Qt::SolidLine,Qt::RoundCap));painter.drawLine(3,3,21,21);}
    }
    else if(name=="zoom-in" || name=="zoom-out") {
        painter.drawEllipse(QRectF(3,3,12,12));painter.drawLine(14,14,21,21);
        painter.drawLine(6,9,12,9);if(name=="zoom-in")painter.drawLine(9,6,9,12);
    }
    else if(name=="fit") {
        for(int i=0;i<4;++i){painter.save();painter.translate(12,12);painter.rotate(i*90);painter.drawLine(-8,-3,-8,-8);painter.drawLine(-8,-8,-3,-8);painter.restore();}
        painter.drawRect(QRectF(8,8,8,8));
    }
    else if(name=="fullscreen") {
        for(const auto corner : {QPointF(4,4),QPointF(20,4),QPointF(4,20),QPointF(20,20)}) {
            painter.drawLine(corner,corner+QPointF(corner.x()<12 ? 5 : -5,0));
            painter.drawLine(corner,corner+QPointF(0,corner.y()<12 ? 5 : -5));
        }
    }
    else if(name=="open") { QPolygonF folder; folder<<QPointF(3,7)<<QPointF(10,7)<<QPointF(12,10)<<QPointF(21,10)<<QPointF(19,20)<<QPointF(3,20); painter.drawPolygon(folder); painter.drawLine(3,7,3,4); painter.drawLine(3,4,10,4); }
    else if(name=="save") { painter.drawRect(4,3,16,18); painter.drawRect(7,3,10,6); painter.drawRect(7,14,10,7); }
    else if(name=="save-as") {
        painter.drawRect(3,3,14,18); painter.drawRect(6,3,8,6); painter.drawRect(6,14,5,7);
        // A pencil distinguishes Save As from the adjacent plain floppy disk.
        painter.setBrush(QColor("#24262d")); QPolygonF pencil;
        pencil<<QPointF(11,20)<<QPointF(12,16)<<QPointF(20,8)<<QPointF(23,11)<<QPointF(15,19);
        painter.drawPolygon(pencil); painter.drawLine(18,10,21,13);
    }
    else if(name=="undo" || name=="redo") { if(name=="redo") { painter.translate(24,0); painter.scale(-1,1); } QPainterPath arrow; arrow.moveTo(4,9); arrow.lineTo(13,9); arrow.cubicTo(23,9,23,20,13,20); painter.drawPath(arrow); painter.drawLine(4,9,9,4); painter.drawLine(4,9,9,14); }
    else if(name=="move") {
        painter.drawLine(12,3,12,21); painter.drawLine(3,12,21,12);
        for(int i=0;i<4;++i) { painter.save(); painter.translate(12,12); painter.rotate(90*i); painter.drawLine(0,-9,-3,-6); painter.drawLine(0,-9,3,-6); painter.restore(); }
    } else if(name=="brush") {
        painter.drawLine(9,15,18,4); painter.drawLine(12,17,21,6); painter.drawLine(18,4,21,6);
        painter.drawEllipse(QRectF(4,15,7,6)); painter.drawLine(4,18,2,22);
    } else if(name=="pencil") {
        painter.drawLine(5,19,18,4); painter.drawLine(8,21,21,6); painter.drawLine(18,4,21,6); painter.drawLine(5,19,4,22); painter.drawLine(4,22,8,21);
    } else if(name=="line") painter.drawLine(4,20,20,4);
    else if(name=="rectangle") painter.drawRect(QRectF(4,5,16,14));
    else if(name=="ellipse") painter.drawEllipse(QRectF(4,4,16,16));
    else if(name=="shapes") { QPolygonF star; for(int i=0;i<10;++i) { const double angle=i*3.141592653589793/5-3.141592653589793/2, radius=i%2 ? 4 : 10; star<<QPointF(12+radius*std::cos(angle),12+radius*std::sin(angle)); } painter.drawPolygon(star); }
    else if(name=="particles") { painter.setBrush(QColor("#e0e8fc")); for(const auto point:{QPointF(5,5),QPointF(15,4),QPointF(21,12),QPointF(12,12),QPointF(5,19),QPointF(16,20)}) painter.drawEllipse(point,1.7,1.7); }
    else if(name=="sources") { painter.drawRect(QRectF(3,6,12,14));painter.drawLine(6,3,18,3);painter.drawLine(18,3,18,11);painter.drawLine(15,15,23,15);painter.drawLine(19,11,19,19); }
    else if(name=="gradient") { QLinearGradient gradient(4,12,20,12); gradient.setColorAt(0,Qt::white); gradient.setColorAt(1,QColor("#414957")); painter.setBrush(gradient); painter.drawRect(QRectF(3,5,18,14)); }
    else if(name=="fill") { QPolygonF bucket; bucket<<QPointF(5,10)<<QPointF(12,3)<<QPointF(20,11)<<QPointF(12,19); painter.drawPolygon(bucket); painter.drawLine(4,10,14,10); painter.drawLine(19,15,21,20); }
    else if(name=="eraser") { QPolygonF eraser; eraser<<QPointF(3,15)<<QPointF(13,4)<<QPointF(21,11)<<QPointF(12,21)<<QPointF(9,21); painter.drawPolygon(eraser); painter.drawLine(9,10,17,17); }
    else {
        painter.drawLine(6,2,6,18); painter.drawLine(6,18,22,18); painter.drawLine(2,6,18,6); painter.drawLine(18,6,18,22);
    }
    return QIcon(pixmap);
}

ColorEditor::ColorEditor(QWidget* parent) : QWidget(parent)
{
    auto* layout=new QHBoxLayout(this); layout->setContentsMargins(0,0,0,0); layout->setSpacing(6);
    picker_=new QPushButton; picker_->setFixedSize(40,30); picker_->setAccessibleName("Choose RGB color");
    picker_->setToolTip("Choose a color visually. Transparency is controlled separately.");
    hex_=new QLineEdit; hex_->setObjectName("rgbHex"); hex_->setMaxLength(7); hex_->setMinimumWidth(86);
    hex_->setValidator(new QRegularExpressionValidator(QRegularExpression("#?[0-9a-fA-F]{6}"),hex_));
    hex_->setToolTip("RGB hexadecimal color: #RRGGBB, for example #FF8800. Alpha is not part of the RGB value.");
    hex_->setAccessibleName("RGB hexadecimal value"); layout->addWidget(picker_); layout->addWidget(hex_,1);
    connect(picker_,&QPushButton::pressed,this,[this]{picker_->setProperty("mouseInitiated",QApplication::mouseButtons()!=Qt::NoButton);});
    connect(picker_,&QPushButton::clicked,this,[this] {
        openStudioColorPicker(this,picker_->property("mouseInitiated").toBool());
    });
    connect(hex_,&QLineEdit::editingFinished,this,[this] {
        if(!hex_->hasAcceptableInput()) { setColor(color_); return; }
        const auto text=hex_->text(); const QColor chosen(text.startsWith('#') ? text : "#"+text);
        if(chosen.isValid() && chosen!=color_) { setColor(chosen); emit colorChanged(color_); }
        else setColor(color_);
    });
    setColor(color_);
}
void ColorEditor::setColor(QColor color)
{
    color_=color; hex_->setText(color.name(QColor::HexRgb).toUpper());
    QPixmap icon(26,18); icon.fill(color); picker_->setIcon(QIcon(icon)); picker_->setIconSize({26,18});
}
namespace {
class PaletteStrip : public QWidget {
public:
    QVector<QColor> colors; QVector<double> positions; int selected=0; bool gradient=false, dragging=false;
    std::function<void(int)> select;
    std::function<void(double)> move, add;
    std::function<void()> begin,end,remove;
    PaletteStrip(QWidget* parent):QWidget(parent) { setMinimumHeight(66); setFocusPolicy(Qt::StrongFocus); setAccessibleName("Color stops: drag to move, double click to add, Delete to remove"); }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const QRectF bar(12,8,width()-24,28);
        QLinearGradient ramp(bar.topLeft(),bar.topRight());
        for(int i=0;i<colors.size();++i) ramp.setColorAt(positions.value(i),colors[i]);
        p.setPen(QPen(QColor("#565c6a"),1)); p.setBrush(ramp); p.drawRoundedRect(bar,5,5);
        const auto grid=SnapGrid::load();
        if(gradient && grid.enabled) {
            p.setPen(QColor("#d9e6ff"));
            for(double percent=0;percent<=100;percent+=std::max(5,QSettings().value("editor/paletteSnapPercent",1).toInt())) {
                const double x=bar.left()+percent/100*bar.width();p.drawLine(QPointF(x,bar.bottom()-4),QPointF(x,bar.bottom()));
            }
        }
        for(int i=0;i<colors.size();++i) {
            const QPointF point(12+positions[i]*(width()-24),47);
            p.setPen(QPen(i==selected ? QColor("#e8eeff") : QColor("#777f91"),i==selected ? 2.5 : 1));
            p.setBrush(colors[i]); p.drawEllipse(point,7,8);
            if(i==selected) { p.setPen(QColor("#b6caff")); p.drawLine(point+QPointF(0,11),point+QPointF(0,15)); }
        }
    }
    int hit(QPointF point) const {
        for(int i=0;i<positions.size();++i) if(std::abs(point.x()-(12+positions[i]*(width()-24)))<11) return i;
        return -1;
    }
    double position(QPointF point,Qt::KeyboardModifiers modifiers=Qt::NoModifier) const {
        auto grid=SnapGrid::load();grid.enabled=grid.enabled && !(modifiers & Qt::AltModifier);grid.spacing=QSettings().value("editor/paletteSnapPercent",1).toDouble()/100;
        if(!std::isfinite(grid.spacing) || grid.spacing<=0 || grid.spacing>1)grid.spacing=.01;
        return std::clamp(grid.snap(std::clamp((point.x()-12)/std::max(1,width()-24),0.0,1.0)),0.0,1.0);
    }
    void mousePressEvent(QMouseEvent* e) override {
        if(e->button()!=Qt::LeftButton) return;
        setFocus(); const int i=hit(e->position());
        if(i<0) return; select(i); dragging=gradient; if(dragging) begin();
    }
    void mouseMoveEvent(QMouseEvent* e) override { if(dragging) move(position(e->position(),e->modifiers())); }
    void mouseReleaseEvent(QMouseEvent* e) override { if(dragging) { move(position(e->position(),e->modifiers())); dragging=false; end(); } }
    void hideEvent(QHideEvent*) override { if(dragging) { dragging=false; end(); } }
    void mouseDoubleClickEvent(QMouseEvent* e) override { if(gradient && hit(e->position())<0) add(position(e->position(),e->modifiers())); }
    void keyPressEvent(QKeyEvent* e) override {
        if(e->key()==Qt::Key_Delete) remove();
        else if(e->key()==Qt::Key_Left || e->key()==Qt::Key_Right) select(std::clamp(selected+(e->key()==Qt::Key_Left ? -1 : 1),0,int(colors.size()-1)));
        else QWidget::keyPressEvent(e);
    }
};
}
ColorListEditor::ColorListEditor(int minimum,int maximum,QWidget* parent)
    : QWidget(parent),minimum_(minimum),maximum_(maximum)
{
    auto* layout=new QVBoxLayout(this); layout->setContentsMargins(0,0,0,0); layout->setSpacing(8);
    auto* strip=new PaletteStrip(this); rows_=strip; strip->setObjectName("colorStopBar"); layout->addWidget(strip);
    selectedColor_=new ColorEditor; selectedColor_->setObjectName("selectedStopColor"); layout->addWidget(selectedColor_);
    connect(selectedColor_,&ColorEditor::colorChanged,this,[this](QColor color) {
        if(selected_>=colors_.size()) return; colors_[selected_]=color; rebuild(); emit colorsChanged(colors_); emit stopsChanged();
    });
    auto* rgbDetails=new QWidget;auto* rgbLayout=new QVBoxLayout(rgbDetails);rgbLayout->setContentsMargins(0,0,0,0);
    disclosure(layout,"RGB details",rgbDetails);
    for(int channel=0;channel<3;++channel) {
        auto* row=new QHBoxLayout; auto* label=new QLabel(QString("RGB")[channel]); row->addWidget(label);
        auto* slider=new QSlider(Qt::Horizontal); slider->setRange(0,255); slider->setObjectName(QString("stop%1").arg(QString("RGB")[channel]));
        slider->setAccessibleName(QString("Selected stop %1").arg(QString("RGB")[channel]));
        slider->setToolTip("Adjust this RGB channel of the selected color stop."); row->addWidget(slider,1);
        auto* value=new QLabel; value->setFixedWidth(28); row->addWidget(value); rgbValues_.append(value); rgbLayout->addLayout(row); rgb_.append(slider);
        connect(slider,&QSlider::sliderPressed,this,&ColorListEditor::gestureStarted);
        connect(slider,&QSlider::sliderReleased,this,&ColorListEditor::gestureFinished);
        connect(slider,&QSlider::valueChanged,this,[this,channel](int value) {
            if(selected_>=colors_.size()) return;
            auto color=colors_[selected_]; if(channel==0) color.setRed(value); if(channel==1) color.setGreen(value); if(channel==2) color.setBlue(value);
            colors_[selected_]=color; rebuild(); emit colorsChanged(colors_); emit stopsChanged();
        });
    }
    if(minimum_>=2) {auto* snapStep=new QSpinBox;snapStep->setObjectName("paletteSnapPercent");snapStep->setRange(1,100);snapStep->setSuffix(" %");snapStep->setValue(QSettings().value("editor/paletteSnapPercent",1).toInt());snapStep->setAccessibleName("Color stop snap percent");
        auto* snapRow=new QHBoxLayout;snapRow->addWidget(new QLabel("Stop snap"));snapRow->addWidget(snapStep);layout->addLayout(snapRow);
        connect(snapStep,&QSpinBox::valueChanged,this,[](int n){QSettings().setValue("editor/paletteSnapPercent",n);});}
    auto* actions=new QHBoxLayout; add_=new QPushButton("Add"); remove_=new QPushButton("Remove");
    add_->setObjectName("addColorStop"); remove_->setObjectName("removeColorStop");
    actions->addWidget(add_); actions->addWidget(remove_); layout->addLayout(actions);
    connect(add_,&QPushButton::clicked,this,[this] {
        if(colors_.size()==1) { positions_[0]=0; addStop(1); return; }
        double gap=-1,position=.5;
        for(int i=0;i+1<positions_.size();++i) if(positions_[i+1]-positions_[i]>gap) { gap=positions_[i+1]-positions_[i]; position=(positions_[i]+positions_[i+1])/2; }
        addStop(position);
    });
    connect(remove_,&QPushButton::clicked,this,&ColorListEditor::removeStop);
    strip->select=[this](int index) { selected_=index; rebuild(); };
    strip->move=[this](double position) {
        const double lo=selected_ ? positions_[selected_-1]+.001 : 0, hi=selected_+1<positions_.size() ? positions_[selected_+1]-.001 : 1;
        if(hi<lo) return; positions_[selected_]=std::clamp(position,lo,hi); rebuild(); emit stopsChanged();
    };
    strip->begin=[this] { emit gestureStarted(); }; strip->end=[this] { emit gestureFinished(); };
    strip->add=[this](double position) { addStop(position); }; strip->remove=[this] { removeStop(); };
}
void ColorListEditor::setColors(const QVector<QColor>& colors) { setStops(colors,{}); }
void ColorListEditor::setStops(const QVector<QColor>& colors,const QVector<double>& positions)
{
    colors_=colors; positions_=positions;
    if(positions_.size()!=colors_.size()) { positions_.clear(); for(int i=0;i<colors_.size();++i) positions_.append(colors_.size()==1 ? .5 : i/double(colors_.size()-1)); }
    selected_=std::clamp(selected_,0,std::max(0,int(colors_.size()-1))); rebuild();
}
void ColorListEditor::addStop(double position)
{
    if(colors_.size()>=maximum_) return;
    position=std::clamp(position,0.0,1.0);
    for(double existing:positions_) if(std::abs(existing-position)<.001) return;
    const int index=int(std::lower_bound(positions_.begin(),positions_.end(),position)-positions_.begin());
    QColor color=Qt::white;
    if(index>0 && index<colors_.size()) {
        const double t=(position-positions_[index-1])/(positions_[index]-positions_[index-1]);
        const auto a=colors_[index-1], b=colors_[index]; color=QColor(qRound(a.red()+(b.red()-a.red())*t),qRound(a.green()+(b.green()-a.green())*t),qRound(a.blue()+(b.blue()-a.blue())*t));
    }
    colors_.insert(index,color); positions_.insert(index,position); selected_=index; rebuild(); emit colorsChanged(colors_); emit stopsChanged();
}
void ColorListEditor::removeStop()
{
    if(colors_.size()<=minimum_) return;
    colors_.removeAt(selected_); positions_.removeAt(selected_); selected_=std::min(selected_,int(colors_.size()-1));
    rebuild(); emit colorsChanged(colors_); emit stopsChanged();
}
void ColorListEditor::rebuild()
{
    auto* strip=static_cast<PaletteStrip*>(rows_); strip->colors=colors_; strip->positions=positions_; strip->selected=selected_; strip->gradient=gradient_; strip->update();
    selectedColor_->setEnabled(!colors_.isEmpty());
    if(!colors_.isEmpty()) {
        const auto color=colors_[selected_]; selectedColor_->setColor(color); const int values[]{color.red(),color.green(),color.blue()};
        for(int i=0;i<3;++i) { const QSignalBlocker block(rgb_[i]); rgb_[i]->setValue(values[i]); rgbValues_[i]->setText(QString::number(values[i])); }
    }
    add_->setEnabled(colors_.size()<maximum_); remove_->setEnabled(colors_.size()>minimum_);
}

void installParameterTooltips(QWidget* root)
{
    const QMap<QString,QString> descriptions{
        {"animationRate0","Turn the layer continuously: 90 means a quarter turn each second; -90 turns the other way. Zero stops rotation."},
        {"animationRate1","Pulse rate in cycles per minute: 30 means one pulse every two seconds. Zero keeps intensity steady."},
        {"animationRate2","Shift the source through the color wheel: 60 completes a cycle in six seconds. White and gray have no hue to shift. Zero keeps the source colors."},
        {"driftX","Horizontal speed in canvas pixels per second. Effects wrap from one edge to the opposite edge."},
        {"driftY","Vertical speed in canvas pixels per second. Effects wrap from one edge to the opposite edge."},
        {"zoom","Periodic size variation as a percentage of the base size. Zero keeps size constant."},
        {"warp","Horizontal wave distortion amplitude in canvas pixels. Zero disables distortion."},
        {"breathe","Breathing intensity rate in cycles per minute. This modulates opacity, not size. Zero disables breathing."},
        {"trails","Trail duration in milliseconds. Zero disables trails. Seeking recreates the same result."},
        {"cycleRate","Color palette loops per minute. Zero uses the selected effect color."},
        {"particleCount","Number of particles, from 1 to 400. More particles require more rendering time."},
        {"particleSeed","Random pattern seed. The same seed and time reproduce the same particle arrangement."},
        {"particleSpeed","Particle movement speed. Zero freezes movement within the effect."},
        {"particleSize","Particle radius in canvas pixels before effect rotation and scaling."},
        {"previewTime","Absolute preview time in seconds. Editing time pauses playback; it does not change the saved effect."},
        {"effectRepetitions","Number of repeated stripes or waves across the effect width."},
        {"channelRate","Channel playback speed in percent. 100% is normal; 0% holds the offset frame."},
        {"channelOffset","Channel playback offset in milliseconds, added after multiplying the preview time by the clock rate."},
        {"nativeBrightness","Firmware brightness level, 0 (lowest) to 5 (highest). Requires output review before applying."},
        {"nativeSpeed","Firmware animation speed level, 0 to 2. Actual timing depends on the controller."},
        {"componentCount","Number of logical LEDs. Geometry is editable visually; physical wire order must contain each index exactly once."},
        {"layerTransparency","Layer opacity: 100% is fully visible, 0% is invisible. The grayscale mask additionally controls local coverage."}
    };
    const QStringList geometry{"Horizontal effect center in canvas pixels.","Vertical effect center in canvas pixels.","Base effect width in canvas pixels.","Base effect height in canvas pixels.","Base effect rotation in degrees. Animation is applied on top of this angle."};
    const QStringList placement{"Horizontal component origin in layout pixels.","Vertical component origin in layout pixels.","Horizontal geometry scale in percent. 100 keeps catalog dimensions.","Vertical geometry scale in percent. 100 keeps catalog dimensions.","Component rotation in degrees around its origin. This changes layout sampling positions."};
    const QStringList crop{"Left crop edge as a fraction of source image width (0..1).","Top crop edge as a fraction of source image height (0..1).","Crop width as a fraction of the source image. Cropping is non-destructive.","Crop height as a fraction of the source image. Cropping is non-destructive."};
    for(auto* widget:root->findChildren<QWidget*>()) {
        const auto id=widget->objectName(); QString description=descriptions.value(id);
        if(id.startsWith("effectGeometry")) description=geometry.value(id.mid(14).toInt());
        if(id.startsWith("placement")) description=placement.value(id.mid(9).toInt());
        if(id.startsWith("imageCrop")) description=crop.value(id.mid(9).toInt());
        if(description.isEmpty() && qobject_cast<QAbstractSpinBox*>(widget)) {
            const auto name=widget->accessibleName();
            if(name=="Brush diameter") description="Drawing and mask stroke diameter in canvas pixels.";
            if(name=="Hardness %") description="Mask brush edge hardness. 0% gives a soft falloff; 100% gives a hard edge.";
            if(name=="Brush opacity %") description="Brush strength per stroke. Lower values build transparency gradually.";
            if(name=="Mask gray") description="Mask coverage: 0 hides, 255 reveals, intermediate values partially reveal the layer.";
        }
        if(!description.isEmpty()) {
            widget->setToolTip(description);
            if(auto* label=qobject_cast<QWidget*>(widget->property("parameterLabel").value<QObject*>())) label->setToolTip(description);
        }
    }
    for(auto* slider:root->findChildren<QSlider*>()) if(auto* field=qobject_cast<QWidget*>(slider->property("numericField").value<QObject*>())) {
        slider->setToolTip(field->toolTip()); slider->parentWidget()->setToolTip(field->toolTip());
    }
    for(auto* form:root->findChildren<QFormLayout*>()) for(int row=0;row<form->rowCount();++row) {
        auto* field=form->itemAt(row,QFormLayout::FieldRole); auto* label=form->itemAt(row,QFormLayout::LabelRole);
        if(field && field->widget() && label && label->widget()) label->widget()->setToolTip(field->widget()->toolTip());
    }
}

bool studioMotionEnabled() { return !QSettings().value("editor/reduceMotion",!QApplication::isEffectEnabled(Qt::UI_AnimateCombo)).toBool(); }

namespace {
class StudioBanner : public QWidget {
    QPixmap artwork_{":/branding/lighting-studio-banner.png"};
public:
    StudioBanner(QWidget* parent,int height):QWidget(parent){setFixedHeight(height);setMinimumWidth(240);setAccessibleName("Lighting Studio banner");setToolTip("Lighting Studio - conceptual branding artwork");}
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);painter.fillRect(rect(),QColor("#111214"));
        const auto size=artwork_.size().scaled(this->size(),Qt::KeepAspectRatio);
        const QRect target(QPoint((width()-size.width())/2,(height()-size.height())/2),size);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);painter.drawPixmap(target,artwork_);
    }
};
}
QWidget* studioBanner(QWidget* parent,int height){return new StudioBanner(parent,height);}
