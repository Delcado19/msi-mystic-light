#include "StudioWidgets.h"
#include "LightingDocument.h"
#include <QApplication>
#include <QAbstractSpinBox>
#include <QMap>
#include <QFormLayout>
#include <QColorDialog>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>
#include <algorithm>

void applyStudioStyle(QWidget* window)
{
    auto font=window->font(); font.setPointSizeF(std::max(10.0,font.pointSizeF())); window->setFont(font);
    window->setStyleSheet(R"(
        QWidget { color: #e8eaf0; background: #24262d; }
        QMainWindow, QDialog { background: #1b1d23; }
        QMenuBar, QToolBar, QStatusBar { background: #202229; spacing: 8px; }
        QMenuBar { padding: 4px; }
        QMenuBar::item:selected, QMenu::item:selected { background: #35436a; }
        QToolBar { border: 0; padding: 6px; }
        QDockWidget { font-weight: 600; }
        QDockWidget::title { padding: 12px; background: #202229; }
        QTabWidget::pane { border: 0; }
        QTabBar::tab { padding: 10px 18px; color: #b6bdcd; background: #202229; border-bottom: 2px solid transparent; }
        QTabBar::tab:selected { color: #f4f6ff; border-bottom-color: #8eaaff; background: #2c303b; }
        QScrollArea { border: 0; }
        QPushButton, QToolButton { background: #323640; border: 1px solid #454b59; border-radius: 6px; padding: 7px 10px; }
        QPushButton:hover, QToolButton:hover { background: #41495a; }
        QPushButton:pressed, QToolButton:pressed, QToolButton:checked, QPushButton:checked { background: #3c4c76; border-color: #8eaaff; }
        QPushButton:disabled, QToolButton:disabled { color: #8b92a1; background: #292c33; border-color: #343842; }
        QPushButton[primary="true"] { background: #425f9f; border-color: #829de4; color: white; font-weight: 600; }
        QComboBox, QLineEdit, QSpinBox, QDoubleSpinBox { border: 1px solid #454b59; border-radius: 5px; background: #1d2027; padding: 6px; min-height: 18px; }
        QComboBox { padding-right: 20px; }
        QAbstractSpinBox::up-button, QAbstractSpinBox::down-button { width: 0; }
        QTreeWidget, QListWidget { background: #202229; border: 0; outline: 0; }
        QTreeWidget::item, QListWidget::item { padding: 8px 4px; border-radius: 5px; }
        QTreeWidget::item:selected, QListWidget::item:selected { background: #344667; color: white; }
        QHeaderView::section { background: #24262d; color: #b6bdcd; border: 0; padding: 6px; }
        QSlider::groove:horizontal { background: #454c5e; height: 4px; border-radius: 2px; }
        QSlider::sub-page:horizontal { background: #91aaff; border-radius: 2px; }
        QSlider::handle:horizontal { background: #dce5ff; width: 12px; margin: -5px 0; border-radius: 6px; }
        QSlider { min-height: 24px; }
        QCheckBox { spacing: 8px; padding: 4px 0; }
        QCheckBox::indicator { width: 15px; height: 15px; border: 1px solid #717b91; border-radius: 3px; background: #1d2027; }
        QCheckBox::indicator:checked { background: #91aaff; border-color: #c1d0ff; }
        QLabel[muted="true"] { color: #b6bdcd; }
        QLabel[heading="true"] { font-size: 16px; font-weight: 600; padding: 6px 0; }
        QToolButton[disclosure="true"] { text-align: left; background: transparent; border: 0; border-top: 1px solid #3a3e49; border-radius: 0; padding: 12px 0; }
        QScrollBar:vertical { width: 10px; background: #202229; }
        QScrollBar::handle:vertical { background: #596174; min-height: 24px; border-radius: 4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QWidget:focus { border-color: #b7c9ff; }
        QToolTip { color: #edf1ff; background: #343d50; border: 1px solid #7788ad; padding: 6px; }
    )");
}

QWidget* sliderField(QDoubleSpinBox* value)
{
    auto* row=new QWidget; auto* layout=new QHBoxLayout(row); layout->setContentsMargins(0,0,0,0);
    auto* slider=new QSlider(Qt::Horizontal); slider->setRange(0,1000);
    slider->setAccessibleName(value->accessibleName().isEmpty() ? value->objectName() : value->accessibleName());
    slider->setProperty("numericField",QVariant::fromValue(static_cast<QObject*>(value)));
    value->setButtonSymbols(QAbstractSpinBox::NoButtons); value->setFixedWidth(72);
    layout->addWidget(slider,1); layout->addWidget(value);
    QObject::connect(slider,&QSlider::valueChanged,value,[value](int n) { value->setValue(value->minimum()+(value->maximum()-value->minimum())*n/1000.0); });
    QObject::connect(value,&QDoubleSpinBox::valueChanged,slider,[value,slider](double n) {
        const QSignalBlocker block(slider); slider->setValue(qRound(1000*(n-value->minimum())/(value->maximum()-value->minimum())));
    });
    return row;
}
void syncSliderFields(QWidget* root)
{
    // Undo/document refresh blocks numeric signals; synchronize handles without creating another edit.
    for(auto* slider:root->findChildren<QSlider*>()) if(auto* value=qobject_cast<QDoubleSpinBox*>(slider->property("numericField").value<QObject*>())) {
        const QSignalBlocker block(slider); slider->setEnabled(value->isEnabled());
        slider->setValue(qRound(1000*(value->value()-value->minimum())/(value->maximum()-value->minimum())));
    }
}
QWidget* disclosure(QVBoxLayout* layout,const QString& title,QWidget* content)
{
    auto* box=new QWidget; auto* stack=new QVBoxLayout(box); stack->setContentsMargins(0,0,0,0); stack->setSpacing(0);
    auto* toggle=new QToolButton; toggle->setText(title); toggle->setProperty("disclosure",true);
    toggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); toggle->setArrowType(Qt::RightArrow); toggle->setCheckable(true);
    toggle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed); stack->addWidget(toggle); stack->addWidget(content); content->hide();
    QObject::connect(toggle,&QToolButton::toggled,content,[toggle,content](bool open) { content->setVisible(open); toggle->setArrowType(open ? Qt::DownArrow : Qt::RightArrow); });
    layout->addWidget(box); return box;
}
std::optional<EffectKind> chooseEffect(QWidget* parent)
{
    QDialog dialog(parent); dialog.setObjectName("effectGallery"); dialog.setWindowTitle("Add an effect layer"); dialog.resize(720,530);
    auto* layout=new QVBoxLayout(&dialog); layout->setContentsMargins(24,20,24,20); layout->setSpacing(12);
    auto* title=new QLabel("Choose an effect"); title->setProperty("heading",true); layout->addWidget(title);
    auto* subtitle=new QLabel("Each effect becomes a layer with its own editable mask."); subtitle->setProperty("muted",true); layout->addWidget(subtitle);
    auto* search=new QLineEdit; search->setPlaceholderText("Search effects..."); search->setObjectName("effectSearch"); layout->addWidget(search);
    auto* list=new QListWidget; list->setObjectName("effectGalleryItems"); list->setViewMode(QListView::IconMode);
    list->setMovement(QListView::Static); list->setResizeMode(QListView::Adjust); list->setIconSize({112,70}); list->setGridSize({150,116});
    for(const auto& definition:effectDefinitions()) {
        if(definition.kind==EffectKind::Image) continue; // Images use Import, so choosing an effect never creates an empty image layer.
        EffectLayer layer; layer.effect=definition.kind; layer.color=QColor("#91aaff"); layer.secondaryColor=QColor("#ff91ba");
        // Gallery and canvas use the same source renderer, not approximate marketing thumbnails.
        QPixmap preview(112,70); preview.fill(QColor("#171b25")); QPainter painter(&preview);
        painter.drawImage(preview.rect(),renderEffect(layer,1)); painter.end();
        auto* item=new QListWidgetItem(QIcon(preview),definition.label,list); item->setData(Qt::UserRole,static_cast<int>(definition.kind));
        item->setToolTip("Add "+definition.label+" with a white mask"); item->setTextAlignment(Qt::AlignCenter);
    }
    layout->addWidget(list,1); auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("Add layer"); layout->addWidget(buttons);
    QObject::connect(search,&QLineEdit::textChanged,list,[list,buttons](const QString& text) {
        QListWidgetItem* first=nullptr;
        for(int i=0;i<list->count();++i) {
            auto* item=list->item(i); item->setHidden(!item->text().contains(text,Qt::CaseInsensitive));
            if(!item->isHidden() && !first) first=item;
        }
        list->setCurrentItem(first); buttons->button(QDialogButtonBox::Ok)->setEnabled(first!=nullptr);
    });
    QObject::connect(list,&QListWidget::itemDoubleClicked,&dialog,&QDialog::accept);
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    list->setCurrentRow(0); search->setFocus();
    if(dialog.exec()!=QDialog::Accepted || !list->currentItem() || list->currentItem()->isHidden()) return {};
    return static_cast<EffectKind>(list->currentItem()->data(Qt::UserRole).toInt());
}

QIcon studioToolIcon(const QString& name)
{
    QPixmap pixmap(24,24); pixmap.fill(Qt::transparent); QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing); painter.setPen(QPen(QColor("#e0e8fc"),1.7,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    if(name=="move") {
        painter.drawLine(12,3,12,21); painter.drawLine(3,12,21,12);
        for(int i=0;i<4;++i) { painter.save(); painter.translate(12,12); painter.rotate(90*i); painter.drawLine(0,-9,-3,-6); painter.drawLine(0,-9,3,-6); painter.restore(); }
    } else if(name=="brush") {
        painter.drawLine(9,15,18,4); painter.drawLine(12,17,21,6); painter.drawLine(18,4,21,6);
        painter.drawEllipse(QRectF(4,15,7,6)); painter.drawLine(4,18,2,22);
    } else {
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
    connect(picker_,&QPushButton::clicked,this,[this] {
        const auto selected=QColorDialog::getColor(color_,this,"Choose RGB color");
        if(selected.isValid() && selected!=color_) { setColor(selected); emit colorChanged(color_); }
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
ColorListEditor::ColorListEditor(int minimum,int maximum,QWidget* parent)
    : QWidget(parent),minimum_(minimum),maximum_(maximum)
{
    auto* layout=new QVBoxLayout(this); layout->setContentsMargins(0,0,0,0); layout->setSpacing(6);
    auto* scroll=new QScrollArea; scroll->setWidgetResizable(true); scroll->setMaximumHeight(156);
    rows_=new QWidget; new QVBoxLayout(rows_); rows_->layout()->setContentsMargins(0,0,4,0);
    scroll->setWidget(rows_); layout->addWidget(scroll);
    auto* actions=new QHBoxLayout; add_=new QPushButton("Add color"); remove_=new QPushButton("Remove last");
    actions->addWidget(add_); actions->addWidget(remove_); layout->addLayout(actions);
    add_->setToolTip("Add another color to this palette.");
    remove_->setToolTip("Remove the last color while retaining the minimum number required by this effect.");
    connect(add_,&QPushButton::clicked,this,[this] {
        if(colors_.size()>=maximum_) return; colors_.append(Qt::white); rebuild(); emit colorsChanged(colors_);
    });
    connect(remove_,&QPushButton::clicked,this,[this] {
        if(colors_.size()<=minimum_) return; colors_.removeLast(); rebuild(); emit colorsChanged(colors_);
    });
}
void ColorListEditor::setColors(const QVector<QColor>& colors)
{
    if(colors_==colors) return; colors_=colors; rebuild();
}
void ColorListEditor::rebuild()
{
    auto* layout=qobject_cast<QVBoxLayout*>(rows_->layout());
    while(auto* item=layout->takeAt(0)) { if(item->widget()) item->widget()->deleteLater(); delete item; }
    for(int i=0;i<colors_.size();++i) {
        auto* editor=new ColorEditor; editor->setColor(colors_[i]); layout->addWidget(editor);
        connect(editor,&ColorEditor::colorChanged,this,[this,i](QColor color) {
            if(i>=colors_.size()) return; colors_[i]=color; emit colorsChanged(colors_);
        });
    }
    rows_->setMinimumHeight(colors_.size()*36); add_->setEnabled(colors_.size()<maximum_); remove_->setEnabled(colors_.size()>minimum_);
}

void installParameterTooltips(QWidget* root)
{
    const QMap<QString,QString> descriptions{
        {"animationRate0","Rotation speed in degrees per second. Negative values reverse the rotation."},
        {"animationRate1","Pulse frequency in cycles per second (Hz). Zero disables pulsing."},
        {"animationRate2","Hue rotation in degrees per second. Gray and white stay achromatic."},
        {"driftX","Horizontal speed in canvas pixels per second. Effects wrap from one edge to the opposite edge."},
        {"driftY","Vertical speed in canvas pixels per second. Effects wrap from one edge to the opposite edge."},
        {"zoom","Periodic size variation as a percentage of the base size. Zero keeps size constant."},
        {"warp","Horizontal wave distortion amplitude in canvas pixels. Zero disables distortion."},
        {"breathe","Breathing frequency in cycles per second. Zero disables breathing."},
        {"trails","Trail duration in seconds. Zero disables trails. Seeking recreates the same result."},
        {"cycleRate","Color palette loops per second. Zero uses the selected effect color."},
        {"particleCount","Number of particles, from 1 to 400. More particles require more rendering time."},
        {"particleSeed","Random pattern seed. The same seed and time reproduce the same particle arrangement."},
        {"particleSpeed","Particle movement speed. Zero freezes movement within the effect."},
        {"particleSize","Particle radius in canvas pixels before effect rotation and scaling."},
        {"previewTime","Absolute preview time in seconds. Editing time pauses playback; it does not change the saved effect."},
        {"effectRepetitions","Number of repeated stripes or waves across the effect width."},
        {"channelRate","Channel playback multiplier. 1 is normal speed; 0 holds the offset frame."},
        {"channelOffset","Channel playback offset in seconds, added after multiplying the preview time by the clock rate."},
        {"nativeBrightness","Firmware brightness level, 0 (lowest) to 5 (highest). Requires output review before applying."},
        {"nativeSpeed","Firmware animation speed level, 0 to 2. Actual timing depends on the controller."},
        {"componentCount","Number of logical LEDs. Geometry is editable visually; physical wire order must contain each index exactly once."},
        {"layerTransparency","Layer transparency: 0% is fully visible, 100% is invisible. The grayscale mask additionally controls local coverage."}
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
            if(name=="Brush diameter") description="Mask brush diameter in canvas pixels. The outline on the canvas shows its size.";
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
